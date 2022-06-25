#include "json_reader.h"
#include "json_builder.h"
#include "graph.h"

using namespace std;
using namespace json;

ReaderJSON::ReaderJSON (std::istream& is) {
    json_document_ = Load(is);
}

const Array& ReaderJSON::GetBaseQueries() const {
    return json_document_.GetRoot().AsDict().at("base_requests"s).AsArray();
}

const Array& ReaderJSON::GetStatQueries() const {
    return json_document_.GetRoot().AsDict().at("stat_requests"s).AsArray();
}

const Dict& ReaderJSON::GetRenderSettings() const {
    return json_document_.GetRoot().AsDict().at("render_settings"s).AsDict();
}

const Dict& ReaderJSON::GetRoutingSettings() const {
    return json_document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
}

RouteSettings ReaderJSON::ReadRoutingSettings() const {
    return {GetRoutingSettings().at("bus_wait_time"s).AsInt(),
            GetRoutingSettings().at("bus_velocity"s).AsDouble()};
}

void ReaderJSON::ReadRenderSettings(RenderSettings& render_set) {
    const Dict& setting_map = GetRenderSettings();

    render_set.width = setting_map.at("width"s).AsDouble();
    render_set.height = setting_map.at("height"s).AsDouble();

    render_set.padding = setting_map.at("padding"s).AsDouble();

    render_set.line_width = setting_map.at("line_width"s).AsDouble();
    render_set.stop_radius = setting_map.at("stop_radius"s).AsDouble();

    render_set.bus_label_font_size = setting_map.at("bus_label_font_size"s).AsInt();
    render_set.bus_label_offset.x = setting_map.at("bus_label_offset"s).AsArray().at(0).AsDouble();
    render_set.bus_label_offset.y = setting_map.at("bus_label_offset"s).AsArray().at(1).AsDouble();

    render_set.stop_label_font_size = setting_map.at("stop_label_font_size"s).AsInt();
    render_set.stop_label_offset.x = setting_map.at("stop_label_offset"s).AsArray().at(0).AsDouble();
    render_set.stop_label_offset.y = setting_map.at("stop_label_offset"s).AsArray().at(1).AsDouble();

    if (setting_map.at("underlayer_color"s).IsString()) {
        render_set.underlayer_color = setting_map.at("underlayer_color"s).AsString();

    } else {
        const int rgb_size = setting_map.at("underlayer_color"s).AsArray().size();
        const auto rgb_array = setting_map.at("underlayer_color"s).AsArray();

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

    render_set.underlayer_width = setting_map.at("underlayer_width"s).AsDouble();

    for (const auto& color : setting_map.at("color_palette"s).AsArray()) {
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
        if (stop_query.AsDict().at("type"s) != "Stop"s) {
            continue;
        }
        string stop_name = stop_query.AsDict().at("name"s).AsString();
        double lat = stop_query.AsDict().at("latitude"s).AsDouble();
        double lng = stop_query.AsDict().at("longitude"s).AsDouble();
        catalogue.AddStop(stop_name, {lat,lng});
    }

    for (const auto& bus_query : GetBaseQueries()) {
        if (bus_query.AsDict().at("type"s) != "Bus"s) {
            continue;
        }
        string bus_name = bus_query.AsDict().at("name"s).AsString();
        const Array& stops = bus_query.AsDict().at("stops"s).AsArray();
        vector<string_view> stop_names;

        for (const auto& node : stops) {
            stop_names.push_back(node.AsString());
        }
        int num_stops = stop_names.size();

        if (!bus_query.AsDict().at("is_roundtrip"s).AsBool()) {
            stop_names.reserve(2 * stop_names.size());
            stop_names.insert(stop_names.end(), stop_names.rbegin() + 1, stop_names.rend());
        }

        catalogue.AddBus(bus_name, stop_names, num_stops);
    }

    for (const auto& stop_query : GetBaseQueries()) {
        if (stop_query.AsDict().at("type"s) != "Stop"s) {
            continue;
        }

        string stop_depart = stop_query.AsDict().at("name"s).AsString();
        const Dict& name_to_dist = stop_query.AsDict().at("road_distances"s).AsDict();

        for (const auto& [stop_arrival, distance] : name_to_dist) {
            catalogue.SetDistance(stop_depart, stop_arrival, distance.AsInt());
        }
    }

    catalogue.SetRouteSettings(ReadRoutingSettings());
}

json::Dict ReaderJSON::StatReadBus(const json::Node& stat_query, const TransportCatalogue& catalogue) {
    string stop_name = stat_query.AsDict().at("name"s).AsString();
    auto stop_info = catalogue.GetStopInfo(stop_name);

    if (!stop_info.find_stop) {
        return json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build().AsDict();
    }

    Array buses;
    for (string_view s : stop_info.buses) {
        buses.push_back(Node{string(s)});
    }

    return json::Builder{}
                    .StartDict()
                        .Key("buses"s).Value(buses)
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                    .EndDict()
                    .Build().AsDict();
}

json::Dict ReaderJSON::StatReadStop(const json::Node& stat_query, const TransportCatalogue& catalogue) {
    string bus_name = stat_query.AsDict().at("name"s).AsString();
    auto bus_info = catalogue.GetBusInfo(bus_name);

    if (!bus_info.find_bus) {
        return json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build().AsDict();
    }

    return json::Builder{}
                    .StartDict()
                        .Key("curvature"s).Value(bus_info.distance / bus_info.geo_distance)
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                        .Key("route_length"s).Value(bus_info.distance)
                        .Key("stop_count").Value(bus_info.num_stops)
                        .Key("unique_stop_count").Value(bus_info.num_unique_stops)
                    .EndDict()
                    .Build().AsDict();
}

json::Dict ReaderJSON::StatReadSVG(const json::Node& stat_query, const string& svg_doc) {
    return json::Builder{}
                    .StartDict()
                        .Key("map"s).Value(svg_doc)
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                    .EndDict()
                    .Build().AsDict();
}

json::Dict ReaderJSON::StatReadRoute(const json::Node& stat_query, const TransportCatalogue& catalogue, const graph::Router<double>& router, const vector<RouteStat>& routes) {
    const RouteSettings route_settings = ReadRoutingSettings();

    graph::VertexId stop_from_id = catalogue.GetStopId(stat_query.AsDict().at("from"s).AsString());
    graph::VertexId stop_to_id = catalogue.GetStopId(stat_query.AsDict().at("to"s).AsString());
    auto route_info = router.BuildRoute(stop_from_id, stop_to_id);

    Array items;

    if (route_info) {

        for (int num_edge : (*route_info).edges) {
            Dict wait = json::Builder{}
                             .StartDict()
                                 .Key("type"s).Value("Wait"s)
                                 .Key("stop_name"s).Value(routes[num_edge].stop_from->name)
                                 .Key("time"s).Value(route_settings.bus_wait_time)
                             .EndDict()
                             .Build().AsDict();

            Dict trip = json::Builder{}
                             .StartDict()
                                 .Key("type"s).Value("Bus"s)
                                 .Key("bus"s).Value(routes[num_edge].bus->name)
                                 .Key("span_count"s).Value(routes[num_edge].stop_count)
                                 .Key("time"s).Value(routes[num_edge].travel_time)
                             .EndDict()
                             .Build().AsDict();

            items.push_back(wait);
            items.push_back(trip);
        }

        return json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                        .Key("total_time"s).Value((*route_info).weight)
                        .Key("items"s).Value(items)
                    .EndDict()
                    .Build().AsDict();

    } else {

        return json::Builder{}
                    .StartDict()
                        .Key("request_id"s).Value(stat_query.AsDict().at("id"s).AsInt())
                        .Key("error_message"s).Value("not found"s)
                    .EndDict()
                    .Build().AsDict();

    }

}

json::Document ReaderJSON::StatReadToJSON(TransportCatalogue& catalogue, const TransportRouter& transport_router, const std::string& svg_doc) {
    graph::Router<double> router(transport_router.GetTransportGraph());
    const vector<RouteStat>& stat_routes = transport_router.GetStatRoutes();

    Array result;
    for (const auto& stat_query : GetStatQueries()) {

        if (stat_query.AsDict().at("type"s) == "Stop"s) {
            result.push_back(StatReadBus(stat_query, catalogue));
        }

        if (stat_query.AsDict().at("type"s) == "Bus"s) {
            result.push_back(StatReadStop(stat_query, catalogue));
        }

        if (stat_query.AsDict().at("type"s) == "Map"s) {
            result.push_back(StatReadSVG(stat_query, svg_doc));
        }

        if (stat_query.AsDict().at("type"s) == "Route"s) {
            result.push_back(StatReadRoute(stat_query, catalogue, router, stat_routes));
        }
    }
    return Document(result);
}

