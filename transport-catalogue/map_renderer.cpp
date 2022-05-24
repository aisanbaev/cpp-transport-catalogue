#include "map_renderer.h"

#include <set>
#include <map>
#include <fstream>

using namespace std::literals;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Document CreateRoutesMap(const RenderSettings& render_setting, const std::unordered_map<std::string_view, const Bus*>& busname_to_bus) {
    std::set<std::string_view> buses;
    std::map<std::string_view, geo::Coordinates> stopname_to_coord;
    std::vector<geo::Coordinates> geo_coords;

    for (const auto& [bus_name, bus_ptr] : busname_to_bus) {
        buses.insert(bus_name);
        for (const auto& stop_ptr : bus_ptr->stops) {
            geo_coords.push_back(stop_ptr->coordinates);
            stopname_to_coord[stop_ptr->name] = stop_ptr->coordinates;
        }
    }

    // Создаём проектор сферических координат на карту
    const SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), render_setting.width, render_setting.height, render_setting.padding
    };

    svg::Document doc;

    const int palette_size= render_setting.color_palette.size();
    int palette_num = 0;

    //------------ Рисуем автобусные маршруты --------------//
    for (const auto& name : buses) {
        svg::Polyline bus_line;

        const Bus* bus = busname_to_bus.at(name);

        for (const Stop* stop : bus->stops) {
            const svg::Point screen_coord = proj(stop->coordinates); // Проецируем координаты
            bus_line.AddPoint(screen_coord);
        }

        if (palette_num == palette_size) {
            palette_num = 0;
        }

        bus_line.SetStrokeWidth(render_setting.line_width)
                .SetFillColor("none"s)
                .SetStrokeWidth(render_setting.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeColor(render_setting.color_palette[palette_num]);

        doc.Add(bus_line);

        if (!bus->stops.empty()) {
            ++palette_num;
        }
    }

    //------------- Наносим названия маршрутов -----------------//
    palette_num = 0;
    for (const auto& name : buses) {
        svg::Text route_name;

        const Bus* bus = busname_to_bus.at(name);
        if (bus->stops.empty()) {
            continue;
        }

        if (palette_num == palette_size) {
            palette_num = 0;
        }

        const svg::Point screen_coord_start = proj(bus->stops[0]->coordinates);

        route_name.SetData(bus->name)
                  .SetPosition(screen_coord_start)
                  .SetOffset(render_setting.bus_label_offset)
                  .SetFontFamily("Verdana"s)
                  .SetFontSize(render_setting.bus_label_font_size)
                  .SetFontWeight("bold"s);

        svg::Text route_name_base;
        route_name_base = route_name;
        route_name_base.SetStrokeColor(render_setting.underlayer_color)
                       .SetFillColor(render_setting.underlayer_color)
                       .SetStrokeWidth(render_setting.underlayer_width)
                       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text route_name_text;
        route_name_text = route_name;
        route_name_text.SetFillColor(render_setting.color_palette[palette_num]);

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

    //----------- Рисуем символы остановок---------//
    svg::Circle stop_circle;
    stop_circle.SetRadius(render_setting.stop_radius).SetFillColor("white"s);

    for (const auto& [_, coord] : stopname_to_coord) {
        stop_circle.SetCenter(proj(coord));
        doc.Add(stop_circle);
    }

    //----------- Наносим названия остановок---------//
    for (const auto& [name, coord] : stopname_to_coord) {
        svg::Text stop_name;

        stop_name.SetData(std::string(name))
                  .SetPosition(proj(coord))
                  .SetOffset(render_setting.stop_label_offset)
                  .SetFontFamily("Verdana"s)
                  .SetFontSize(render_setting.stop_label_font_size);

        svg::Text stop_name_base;
        stop_name_base = stop_name;
        stop_name_base.SetStrokeColor(render_setting.underlayer_color)
                       .SetFillColor(render_setting.underlayer_color)
                       .SetStrokeWidth(render_setting.underlayer_width)
                       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text stop_name_text;
        stop_name_text = stop_name;
        stop_name_text.SetFillColor("black"s);

        doc.Add(stop_name_base);
        doc.Add(stop_name_text);
    }

    return doc;
}
