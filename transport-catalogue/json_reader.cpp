#include "json_reader.h"

using namespace std;
using namespace json;

const Array& ReaderJSON::GetBaseQueries() const {
    return json_document_.GetRoot().AsMap().at("base_requests"s).AsArray();
}

const Array& ReaderJSON::GetStatQueries() const {
    return json_document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
}

const Dict& ReaderJSON::GetRenderSettings() const {
    return json_document_.GetRoot().AsMap().at("render_settings"s).AsMap();
}

void ReaderJSON::LoadJSON (std::istream& is) {
    json_document_ = Load(is);
}

void ReaderJSON::ReadRenderSettings(RenderSettings& render_set) {
    render_set.width = GetRenderSettings().at("width"s).AsDouble();
    render_set.height = GetRenderSettings().at("height"s).AsDouble();

    render_set.padding = GetRenderSettings().at("padding"s).AsDouble();

    render_set.line_width = GetRenderSettings().at("line_width"s).AsDouble();
    render_set.stop_radius = GetRenderSettings().at("stop_radius"s).AsDouble();

    render_set.bus_label_font_size = GetRenderSettings().at("bus_label_font_size"s).AsInt();
    render_set.bus_label_offset.x = GetRenderSettings().at("bus_label_offset"s).AsArray().at(0).AsDouble();
    render_set.bus_label_offset.y = GetRenderSettings().at("bus_label_offset"s).AsArray().at(1).AsDouble();

    render_set.stop_label_font_size = GetRenderSettings().at("stop_label_font_size"s).AsInt();
    render_set.stop_label_offset.x = GetRenderSettings().at("stop_label_offset"s).AsArray().at(0).AsDouble();
    render_set.stop_label_offset.y = GetRenderSettings().at("stop_label_offset"s).AsArray().at(1).AsDouble();

    if (GetRenderSettings().at("underlayer_color"s).IsString()) {
        render_set.underlayer_color = GetRenderSettings().at("underlayer_color"s).AsString();
    } else {
        const int rgb_size = GetRenderSettings().at("underlayer_color"s).AsArray().size();
        const auto rgb_array = GetRenderSettings().at("underlayer_color"s).AsArray();

        if (rgb_size == 3) {
            svg::Rgb rgb_color;
            rgb_color.red = rgb_array.at(0).AsInt();
            rgb_color.green = rgb_array.at(1).AsInt();
            rgb_color.blue = rgb_array.at(2).AsInt();
            render_set.underlayer_color = rgb_color;
        } else if (rgb_size == 4) {
            svg::Rgba rgba_color;
            rgba_color.red = rgb_array.at(0).AsInt();
            rgba_color.green = rgb_array.at(1).AsInt();
            rgba_color.blue = rgb_array.at(2).AsInt();
            rgba_color.opacity = rgb_array.at(3).AsDouble();
            render_set.underlayer_color = rgba_color;
        }
    }

    render_set.underlayer_width = GetRenderSettings().at("underlayer_width"s).AsDouble();

    for (const auto& color : GetRenderSettings().at("color_palette"s).AsArray()) {
        if (color.IsString()) {
            render_set.color_palette.push_back(color.AsString());
        } else {
        const int rgb_size = color.AsArray().size();
        const auto rgb_array = color.AsArray();

        if (rgb_size == 3) {
            svg::Rgb rgb_color;
            rgb_color.red = rgb_array.at(0).AsInt();
            rgb_color.green = rgb_array.at(1).AsInt();
            rgb_color.blue = rgb_array.at(2).AsInt();
            render_set.color_palette.push_back(rgb_color);
        } else if (rgb_size == 4) {
            svg::Rgba rgba_color;
            rgba_color.red = rgb_array.at(0).AsInt();
            rgba_color.green = rgb_array.at(1).AsInt();
            rgba_color.blue = rgb_array.at(2).AsInt();
            rgba_color.opacity = rgb_array.at(3).AsDouble();
            render_set.color_palette.push_back(rgba_color);
        }
        }
    }

}

void ReaderJSON::TransferDataToCatalogue(TransportCatalogue& catalogue) {

    for (const auto& stop_query : GetBaseQueries()) {
        if (stop_query.AsMap().at("type"s) != "Stop"s) {
            continue;
        }
        string stop_name = stop_query.AsMap().at("name"s).AsString();
        double lat = stop_query.AsMap().at("latitude"s).AsDouble();
        double lng = stop_query.AsMap().at("longitude"s).AsDouble();
        catalogue.AddStop(stop_name, {lat,lng});
    }

    for (const auto& bus_query : GetBaseQueries()) {
        if (bus_query.AsMap().at("type"s) != "Bus"s) {
            continue;
        }
        string bus_name = bus_query.AsMap().at("name"s).AsString();
        const Array& stops = bus_query.AsMap().at("stops"s).AsArray();
        vector<string_view> stop_names;

        for (const auto& node : stops) {
            stop_names.push_back(node.AsString());
        }
        int num_stops = stop_names.size();

        if (!bus_query.AsMap().at("is_roundtrip"s).AsBool()) {
            stop_names.reserve(2 * stop_names.size());
            stop_names.insert(stop_names.end(), stop_names.rbegin() + 1, stop_names.rend());
        }

        catalogue.AddBus(bus_name, stop_names, num_stops);
    }

    for (const auto& stop_query : GetBaseQueries()) {
        if (stop_query.AsMap().at("type"s) != "Stop"s) {
            continue;
        }

        string stop_depart = stop_query.AsMap().at("name"s).AsString();
        const Dict& name_to_dist = stop_query.AsMap().at("road_distances"s).AsMap();

        for (const auto& [stop_arrival, distance] : name_to_dist) {
            catalogue.SetDistance(stop_depart, stop_arrival, distance.AsInt());
        }
    }
}

json::Document ReaderJSON::StatReadToJSON(TransportCatalogue& catalogue, const string& svg_doc) {
    Array result;
    for (const auto& stat_query : GetStatQueries()) {
        if (stat_query.AsMap().at("type"s) == "Stop"s) {
            string stop_name = stat_query.AsMap().at("name"s).AsString();
            auto stop_info = catalogue.GetStopInfo(stop_name);

            if (!stop_info.find_stop) {
                result.push_back(Dict{
                    {"request_id"s, stat_query.AsMap().at("id"s).AsInt()}
                  , {"error_message"s, "not found"s}
                });
                continue;
            }

            Array buses;
            for (string_view s : stop_info.buses) {
                buses.push_back(Node{string(s)});
            }

            Node dict_node{Dict{
                  {"buses"s, {buses}}
                , {"request_id"s, stat_query.AsMap().at("id"s).AsInt()}
            }};
            result.push_back(dict_node);
        }

        if (stat_query.AsMap().at("type"s) == "Bus"s) {
            string bus_name = stat_query.AsMap().at("name"s).AsString();
            auto bus_info = catalogue.GetBusInfo(bus_name);

            if (!bus_info.find_bus) {
                result.push_back(Dict{
                      {"request_id"s, stat_query.AsMap().at("id"s).AsInt()}
                    , {"error_message"s, "not found"s}
                });
                continue;
            }

            Node dict_node{Dict{
                  {"curvature"s, bus_info.distance / bus_info.geo_distance}
                , {"request_id"s, stat_query.AsMap().at("id"s).AsInt()}
                , {"route_length"s, bus_info.distance}
                , {"stop_count", bus_info.num_stops}
                , {"unique_stop_count", bus_info.num_unique_stops}
            }};
            result.push_back(dict_node);
        }

        if (stat_query.AsMap().at("type"s) == "Map"s) {
            Node dict_node{Dict{
                  {"map"s, svg_doc}
                , {"request_id"s, stat_query.AsMap().at("id"s).AsInt()}
            }};
            result.push_back(dict_node);
        }
    }
    return Document(result);
}

