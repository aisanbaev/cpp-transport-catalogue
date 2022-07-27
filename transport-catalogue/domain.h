#pragma once

#include "geo.h"
#include "svg.h"
#include "graph.h"

#include <vector>
#include <string>

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    uint32_t id;
    std::vector<Stop*> nearby_stops;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    int number_stops;
};

struct RenderSettings {
    double width;
    double height;

    double padding;

    double line_width;
    double stop_radius;

    int bus_label_font_size;
    svg::Point bus_label_offset;

    int stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};

struct RouteSettings {
    int bus_wait_time;
    double bus_velocity;
};

struct RouteStat {
    const Bus* bus;
    const Stop* stop_from;
    int stop_count;
    double travel_time;
};
