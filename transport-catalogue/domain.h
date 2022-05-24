#pragma once

#include "geo.h"

#include <vector>
#include <string>

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    int number_stops;
};
