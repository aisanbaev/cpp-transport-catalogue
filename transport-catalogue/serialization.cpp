#include "serialization.h"
#include "transport_catalogue.h"

#include <fstream>

void CatalogueSerializer::SerializeTransportCatalogue(const TransportCatalogue& catalogue) {

    for (const Stop& stop : catalogue.GetStops()) {
        SerializeStop(stop);
        SerializeDistances(catalogue, stop);
    }

    for (const Bus& bus : catalogue.GetBuses()) {
        SerializeBus(bus);
    }
}

void CatalogueSerializer::SerializeStop(const Stop& stop) {
    catalogue_serialize::Stop serial_stop;
    serial_stop.set_name(stop.name);
    serial_stop.set_id(stop.id);
    serial_stop.set_lat(stop.coordinates.lat);
    serial_stop.set_lng(stop.coordinates.lng);
    *base_.add_stops() = std::move(serial_stop);
}

void CatalogueSerializer::SerializeBus(const Bus& bus) {
    catalogue_serialize::Bus serial_bus;
    serial_bus.set_name(bus.name);
    serial_bus.set_num_stops(bus.number_stops);

    for (const Stop* stop: bus.stops) {
        serial_bus.add_id_stops(stop->id);
    }

    *base_.add_buses() = std::move(serial_bus);
}

void CatalogueSerializer::SerializeDistances(const TransportCatalogue& catalogue, const Stop& stop_depart) {
    catalogue_serialize::Stop& serial_stop = *base_.mutable_stops(stop_depart.id);

    for (Stop* stop_arrival : stop_depart.nearby_stops) {
        int distance = catalogue.GetDistance(stop_depart.name, stop_arrival->name);
        catalogue_serialize::Distance dist_to_stop;
        dist_to_stop.set_stop_id(stop_arrival->id);
        dist_to_stop.set_distance(distance);
        *serial_stop.add_distances() = std::move(dist_to_stop);
    }
}

void CatalogueSerializer::SerializeRouteSettings(RouteSettings route_set) {
    routing_settings_.set_wait_time(route_set.bus_wait_time);
    routing_settings_.set_bus_speed(route_set.bus_velocity);
}

void CatalogueSerializer::SerializeRenderSettings(RenderSettings render_set) {
    render_settings_.set_width(render_set.width);
    render_settings_.set_height(render_set.height);
    render_settings_.set_padding(render_set.padding);
    render_settings_.set_line_width(render_set.line_width);
    render_settings_.set_stop_radius(render_set.stop_radius);

    render_settings_.set_bus_label_font_size(render_set.bus_label_font_size);
    render_settings_.set_bus_label_offset_x(render_set.bus_label_offset.x);
    render_settings_.set_bus_label_offset_y(render_set.bus_label_offset.y);

    render_settings_.set_stop_label_font_size(render_set.stop_label_font_size);
    render_settings_.set_stop_label_offset_x(render_set.stop_label_offset.x);
    render_settings_.set_stop_label_offset_y(render_set.stop_label_offset.y);

    *render_settings_.mutable_underlayer_color() = std::move(SerializeColor(render_set.underlayer_color));
    render_settings_.set_underlayer_width(render_set.underlayer_width);

    for (const svg::Color& color : render_set.color_palette) {
        *render_settings_.add_color_palette() = std::move(SerializeColor(color));
    }
}

catalogue_serialize::Color CatalogueSerializer::SerializeColor(const svg::Color& svg_color) {
    catalogue_serialize::Color color;

    if (std::holds_alternative<std::string>(svg_color)) {
        color.set_name(std::get<std::string>(svg_color));

    } else if (std::holds_alternative<svg::Rgb>(svg_color)) {
        color.mutable_rgb()->set_red(std::get<svg::Rgb>(svg_color).red);
        color.mutable_rgb()->set_green(std::get<svg::Rgb>(svg_color).green);
        color.mutable_rgb()->set_blue(std::get<svg::Rgb>(svg_color).blue);

    } else if (std::holds_alternative<svg::Rgba>(svg_color)) {
        color.mutable_rgba()->set_red(std::get<svg::Rgba>(svg_color).red);
        color.mutable_rgba()->set_green(std::get<svg::Rgba>(svg_color).green);
        color.mutable_rgba()->set_blue(std::get<svg::Rgba>(svg_color).blue);
        color.mutable_rgba()->set_opacity(std::get<svg::Rgba>(svg_color).opacity);
    }

    return color;
}

void CatalogueSerializer::SaveTo(const std::string& file_name) const {
    catalogue_serialize::TransportCatalogue db;
    *db.mutable_base() = std::move(base_);
    *db.mutable_render_settings() = std::move(render_settings_);
    *db.mutable_route_settings() = std::move(routing_settings_);

    std::ofstream out(file_name, std::ios::binary);
    db.SerializeToOstream(&out);
}

CatalogueDeserializer::CatalogueDeserializer(const std::string& file_name) {
    std::ifstream input(file_name, std::ios::binary);
    db.ParseFromIstream(&input);
    DeserializeRenderSetting();
    DeserializeRoutingSettings();
}

const catalogue_serialize::TransportBase& CatalogueDeserializer::GetTransportBase() const {
    return db.base();
}

RenderSettings CatalogueDeserializer::GetRenderSettings() const {
    return render_settings_;
}

RouteSettings CatalogueDeserializer::GetRoutingSettings() const {
    return routing_settings_;
}

void CatalogueDeserializer::DeserializeRoutingSettings() {
    routing_settings_.bus_wait_time = db.route_settings().wait_time();
    routing_settings_.bus_velocity = db.route_settings().bus_speed();
}

void CatalogueDeserializer::DeserializeRenderSetting() {
    const catalogue_serialize::RenderSettings& db_rs = db.render_settings();

    render_settings_.width = db_rs.width();
    render_settings_.height = db_rs.height();
    render_settings_.padding = db_rs.padding();
    render_settings_.line_width = db_rs.line_width();
    render_settings_.stop_radius = db_rs.stop_radius();

    render_settings_.bus_label_font_size = db_rs.bus_label_font_size();
    render_settings_.bus_label_offset = {db_rs.bus_label_offset_x(), db_rs.bus_label_offset_y()};
    render_settings_.stop_label_font_size = db_rs.stop_label_font_size();
    render_settings_.stop_label_offset = {db_rs.stop_label_offset_x(), db_rs.stop_label_offset_y()};

    render_settings_.underlayer_color = std::move(DeserializeColor(db_rs.underlayer_color()));
    render_settings_.underlayer_width = db_rs.underlayer_width();

    for (const catalogue_serialize::Color& color : db_rs.color_palette()) {
        render_settings_.color_palette.push_back(std::move(DeserializeColor(color)));
    }
}

svg::Color CatalogueDeserializer::DeserializeColor(const catalogue_serialize::Color& color) {
    svg::Color result;

    if (color.has_name()) {
        result = color.name();

    } else if (color.has_rgb()) {
        result = svg::Rgb{
            static_cast<uint8_t>(color.rgb().red()),
            static_cast<uint8_t>(color.rgb().green()),
            static_cast<uint8_t>(color.rgb().blue())
        };

    } else if (color.has_rgba()) {
        result = svg::Rgba{
            static_cast<uint8_t>(color.rgba().red()),
            static_cast<uint8_t>(color.rgba().green()),
            static_cast<uint8_t>(color.rgba().blue()),
            color.rgba().opacity()
        };
    }

    return result;
}


