#include "transport_catalogue.h"

#include <unordered_set>

using namespace std;

void TransportCatalogue::AddStop(const string& stop_name, const Coordinates& coordinates) {
    const Stop* stop = &stops_.emplace_back(Stop{stop_name, coordinates});

    stopname_to_stop_[stop->name] = stop;
    stop_to_buses_[stop->name];
}

void TransportCatalogue::AddBus(const string& bus_name, const vector<string_view>& stop_names) {
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

void TransportCatalogue::SetDistance(const string_view stop_departure, const string_view stop_arrival, int distance) {
    const Stop* ptr_stop_depart = stopname_to_stop_.at(stop_departure);
    const Stop* ptr_stop_arrival = stopname_to_stop_.at(stop_arrival);

    stop_ptr_pair_to_distance_[{ptr_stop_depart, ptr_stop_arrival}] = distance;
}

int TransportCatalogue::GetDistance (const std::string_view stop_departure, const std::string_view stop_arrival) const {
    const Stop* ptr_stop_depart = stopname_to_stop_.at(stop_departure);
    const Stop* ptr_stop_arrival = stopname_to_stop_.at(stop_arrival);

    return (stop_ptr_pair_to_distance_.count({ptr_stop_depart, ptr_stop_arrival})) ?
            stop_ptr_pair_to_distance_.at({ptr_stop_depart, ptr_stop_arrival}) :
            stop_ptr_pair_to_distance_.at({ptr_stop_arrival, ptr_stop_depart});
}

StopInfo TransportCatalogue::GetStopInfo(const string& stop_name) const {
    if (stop_to_buses_.count(stop_name) == 0) {
        return {stop_name, false, string{}};
    }

    set<string_view> print_set = stop_to_buses_.at(stop_name);
    if (print_set.empty()) {
        return {stop_name, true, string{}};
    }

    string result;
    for (string_view s : print_set) {
        result = result + " "s + string(s);
    }

    return {stop_name, true, result};
}

BusInfo TransportCatalogue::GetBusInfo (const string& bus_name) const {
    if (busname_to_bus_.count(bus_name) == 0) {
        return BusInfo{bus_name, 0, 0, 0, 0};
    }

    const Bus* bus = busname_to_bus_.at(bus_name);
    int size = bus->stops.size();

    double geo_dist = 0;
    int dist = 0;
    unordered_set<string_view> names;
    names.insert(bus->stops[0]->name);

    for (int i = 1; i < size; ++i) {
        geo_dist += ComputeDistance({bus->stops[i - 1]->coordinates}, {bus->stops[i]->coordinates});
        dist += GetDistance(bus->stops[i - 1]->name, bus->stops[i]->name);
        names.insert(bus->stops[i]->name);
    }

    return {bus_name, size, names.size(), dist, geo_dist};
}
