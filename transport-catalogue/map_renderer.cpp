#include "map_renderer.h"

#include <sstream>

using namespace std::literals;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void MapRenderer::DrawBusRoutes(svg::Document& doc, const SphereProjector& proj) {

    int palette_num = 0;

    for (const auto& name : buses_) {
        svg::Polyline bus_line;

        if (palette_num == palette_size_) {
            palette_num = 0;
        }

        const Bus* bus = busname_to_bus_.at(name);
        for (const Stop* stop : bus->stops) {
            const svg::Point screen_coord = proj(stop->coordinates); // Проецируем координаты
            bus_line.AddPoint(screen_coord);
        }

        bus_line.SetStrokeWidth(render_setting_.line_width)
            .SetFillColor("none"s)
            .SetStrokeWidth(render_setting_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeColor(render_setting_.color_palette[palette_num]);

        if (!bus->stops.empty()) {
            ++palette_num;
        }

        doc.Add(bus_line);
    }
}

void MapRenderer::DrawRouteNames(svg::Document& doc, const SphereProjector& proj) {
    int palette_num = 0;

    for (const auto& name : buses_) {

        if (palette_num == palette_size_) {
            palette_num = 0;
        }

        const Bus* bus = busname_to_bus_.at(name);
        if (bus->stops.empty()) {
            continue;
        }

        svg::Text route_name;
        const svg::Point screen_coord_start = proj(bus->stops[0]->coordinates);

        route_name.SetData(bus->name)
                  .SetPosition(screen_coord_start)
                  .SetOffset(render_setting_.bus_label_offset)
                  .SetFontFamily("Verdana"s)
                  .SetFontSize(render_setting_.bus_label_font_size)
                  .SetFontWeight("bold"s);

        svg::Text route_name_base;
        route_name_base = route_name;
        route_name_base.SetStrokeColor(render_setting_.underlayer_color)
                       .SetFillColor(render_setting_.underlayer_color)
                       .SetStrokeWidth(render_setting_.underlayer_width)
                       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text route_name_text;
        route_name_text = route_name;
        route_name_text.SetFillColor(render_setting_.color_palette[palette_num]);

        doc.Add(route_name_base);
        doc.Add(route_name_text);

        if (int(bus->stops.size()) != bus->number_stops && bus->stops[0]->name != bus->stops[bus->number_stops - 1]->name) {
            const svg::Point screen_coord_finish = proj(bus->stops[bus->number_stops - 1]->coordinates);
            route_name_base.SetPosition(screen_coord_finish);
            route_name_text.SetPosition(screen_coord_finish);
            doc.Add(route_name_base);
            doc.Add(route_name_text);
        }

        ++palette_num;
    }
}

void MapRenderer::DrawStops(svg::Document& doc, const SphereProjector& proj) {
    svg::Circle stop_circle;
    stop_circle.SetRadius(render_setting_.stop_radius).SetFillColor("white"s);

    for (const auto& [_, coord] : stopname_to_coord_) {
        stop_circle.SetCenter(proj(coord));
        doc.Add(stop_circle);
    }
}

void MapRenderer::DrawStopNames(svg::Document& doc, const SphereProjector& proj) {
    for (const auto& [name, coord] : stopname_to_coord_) {
        svg::Text stop_name;

        stop_name.SetData(std::string(name))
                  .SetPosition(proj(coord))
                  .SetOffset(render_setting_.stop_label_offset)
                  .SetFontFamily("Verdana"s)
                  .SetFontSize(render_setting_.stop_label_font_size);

        svg::Text stop_name_base;
        stop_name_base = stop_name;
        stop_name_base.SetStrokeColor(render_setting_.underlayer_color)
                       .SetFillColor(render_setting_.underlayer_color)
                       .SetStrokeWidth(render_setting_.underlayer_width)
                       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text stop_name_text;
        stop_name_text = stop_name;
        stop_name_text.SetFillColor("black"s);

        doc.Add(stop_name_base);
        doc.Add(stop_name_text);
    }
}

svg::Document MapRenderer::CreateRoutesMap() {
    svg::Document doc;

    for (const auto& [bus_name, bus_ptr] : busname_to_bus_) {
        buses_.insert(bus_name);
        for (const auto& stop_ptr : bus_ptr->stops) {
            geo_coords_.push_back(stop_ptr->coordinates);
            stopname_to_coord_[stop_ptr->name] = stop_ptr->coordinates;
        }
    }

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{
        geo_coords_.begin(), geo_coords_.end(), render_setting_.width, render_setting_.height, render_setting_.padding
    };

    DrawBusRoutes(doc, proj);
    DrawRouteNames(doc, proj);
    DrawStops(doc, proj);
    DrawStopNames(doc, proj);

    return doc;
}

std::string MapRenderer::ToString(const svg::Document& doc) {
    std::ostringstream map_stream;
    doc.Render(map_stream);
    return map_stream.str();
}
