#include "serialization.h"

#include <fstream>

void CatalogueSerializer::SerializeStop(const std::string& name, uint32_t id, double lat, double lng) {
    catalogue_serialize::Stop stop;
    stop.set_name(name);
    stop.set_id(id);
    stop.set_lat(lat);
    stop.set_lng(lng);

    *stop_list_.add_stops() = stop;
}

void CatalogueSerializer::SerializationBus(const std::string& name, const std::vector<uint32_t>& id_stops, uint32_t num_stops) {
    catalogue_serialize::Bus bus;
    bus.set_name(name);
    *bus.mutable_id_stops() = {id_stops.begin(), id_stops.end()};
    bus.set_num_stops(num_stops);

    *bus_list_.add_buses() = bus;
}

void CatalogueSerializer::SerializeDistances(uint32_t stop_depart_id, uint32_t stop_arrival_id, uint32_t distance) {
    catalogue_serialize::Stop& stop = *stop_list_.mutable_stops(stop_depart_id);
    stop.mutable_stop_arrival_to_dist()->insert({stop_arrival_id, distance});
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

catalogue_serialize::RenderSettings_Color CatalogueSerializer::SerializeColor(const svg::Color& color) {
    catalogue_serialize::RenderSettings_Color result;

    if (std::holds_alternative<std::string>(color)) {
        result.set_id(1);
        result.set_color_name(std::get<std::string>(color));

    } else if (std::holds_alternative<svg::Rgb>(color)) {
        result.set_id(2);
        result.set_r(std::get<svg::Rgb>(color).red);
        result.set_g(std::get<svg::Rgb>(color).green);
        result.set_b(std::get<svg::Rgb>(color).blue);

    } else if (std::holds_alternative<svg::Rgba>(color)) {
        result.set_id(3);
        result.set_r(std::get<svg::Rgba>(color).red);
        result.set_g(std::get<svg::Rgba>(color).green);
        result.set_b(std::get<svg::Rgba>(color).blue);
        result.set_a(std::get<svg::Rgba>(color).opacity);
    }

    return result;
}

void CatalogueSerializer::SaveTo(std::ostream& output) const {
    catalogue_serialize::TransportCatalogue db;
    *db.mutable_stop_list() = std::move(stop_list_);
    *db.mutable_bus_list() = std::move(bus_list_);
    *db.mutable_render_settings() = std::move(render_settings_);
    *db.mutable_route_settings() = std::move(routing_settings_);
    db.SerializeToOstream(&output);
}

CatalogueDeserializer::CatalogueDeserializer(const std::string& file_name) {
    std::ifstream input(file_name, std::ios::binary);
    db.ParseFromIstream(&input);
    DeserializeRenderSetting();
    DeserializeRoutingSettings();
}

const catalogue_serialize::StopList& CatalogueDeserializer::GetStopList() const {
    return db.stop_list();
}

const catalogue_serialize::BusList& CatalogueDeserializer::GetBusList() const {
    return db.bus_list();
}

RenderSettings CatalogueDeserializer::GetRenderSettings() {
    return render_settings_;
}

RouteSettings CatalogueDeserializer::GetRoutingSettings() {
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

    for (const catalogue_serialize::RenderSettings_Color& color : db_rs.color_palette()) {
        render_settings_.color_palette.push_back(std::move(DeserializeColor(color)));
    }
}

svg::Color CatalogueDeserializer::DeserializeColor(const catalogue_serialize::RenderSettings_Color& color) {
    svg::Color result;

    if (color.id() == 1) {
        result = color.color_name();

    } else if (color.id() == 2) {
        result = svg::Rgb{
            static_cast<uint8_t>(color.r()),
            static_cast<uint8_t>(color.g()),
            static_cast<uint8_t>(color.b())
        };

    } else if (color.id() == 3) {
        result = svg::Rgba{
            static_cast<uint8_t>(color.r()),
            static_cast<uint8_t>(color.g()),
            static_cast<uint8_t>(color.b()),
            color.a()
        };
    }

    return result;
}


