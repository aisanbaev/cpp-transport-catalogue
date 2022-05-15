#include <iostream>
#include <unordered_set>
#include <iomanip>

#include "transport_catalogue.h"

using namespace std;

void TransportCatalogue::AddStop(const string& stop_name, Coordinates coordinates) {
    const Stop* stop = &stops_.emplace_back(Stop{stop_name, coordinates});

    stopname_to_stop_[stop->name] = stop;
    stop_to_buses_[stop->name];
}

void TransportCatalogue::AddDistance(const string_view stop_name, std::unordered_map<std::string_view, int> stop_to_distance) {
    const Stop* stop1 = stopname_to_stop_.at(stop_name);

    for (const auto [name, dist] : stop_to_distance) {
        const Stop* stop2 = stopname_to_stop_.at(name);
        stop_ptr_pair_to_distance_[{stop1, stop2}] = dist;
    }
}

void TransportCatalogue::AddBus(const string& bus_name, vector<string_view> stop_names) {
    std::vector<const Stop*> stops;

    for (auto s : stop_names) {
        stops.push_back(stopname_to_stop_.at(s));
    }

    const Bus* bus = &buses_.emplace_back(Bus{bus_name, stops});
    busname_to_bus_[bus->name] = bus;

    for (const Stop* stop : bus->stops) {
        stop_to_buses_[stop->name].insert(bus->name);
    }
}

StopInfo TransportCatalogue::GetStopInfo(string stop_name) {
    if (stop_to_buses_.count(stop_name) == 0) {
        return {stop_name, false, string{}};
    }

    set<string_view> print_set = stop_to_buses_[stop_name];
    if (print_set.empty()) {
        return {stop_name, true, string{}};
    }

    string result;
    for (string_view s : print_set) {
        result = result + " "s + string(s);
    }

    return {stop_name, true, result};
}

BusInfo TransportCatalogue::GetBusInfo (string bus_name) {
    if (busname_to_bus_.count(bus_name) == 0) {
        return BusInfo{bus_name,0,0,0,0};
    }

    const Bus* bus = busname_to_bus_.at(bus_name);
    int size = bus->stops.size();

    double geo_dist = 0;
    int dist = 0;
    unordered_set<string_view> names;
    names.insert(bus->stops[0]->name);

    for (int i = 1; i < size; ++i) {
        geo_dist += ComputeDistance({bus->stops[i - 1]->coordinates}, {bus->stops[i]->coordinates});

        dist += (stop_ptr_pair_to_distance_.count({bus->stops[i - 1], bus->stops[i]})) ?
                    stop_ptr_pair_to_distance_.at({bus->stops[i - 1], bus->stops[i]}) :
                    stop_ptr_pair_to_distance_.at({bus->stops[i], bus->stops[i - 1]});

        names.insert(bus->stops[i]->name);
    }

    return {bus_name, size, names.size(), dist, geo_dist};
}
