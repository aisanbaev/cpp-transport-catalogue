#include "transport_catalogue.h"

#include <iostream>
#include <unordered_set>

using namespace std;

void TransportCatalogue::AddStop(const string& stop_name, const geo::Coordinates& coordinates) {
    const Stop* stop = &stops_.emplace_back(Stop{stop_name, coordinates, stop_id_});

    stopname_to_stop_[stop->name] = stop;
    stop_to_buses_[stop->name];

    ++stop_id_;
}

void TransportCatalogue::AddBus(const string& bus_name, const vector<string_view>& stop_names, int num_stops) {
    std::vector<const Stop*> stops;

    for (auto s : stop_names) {
        stops.push_back(stopname_to_stop_.at(s));
    }

    const Bus* bus = &buses_.emplace_back(Bus{bus_name, stops, num_stops});
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
        return {stop_name, false, {}};
    }

    set<string_view> print_set = stop_to_buses_.at(stop_name);
    if (print_set.empty()) {
        return {stop_name, true, {}};
    }

    return {stop_name, true, print_set};
}

BusInfo TransportCatalogue::GetBusInfo (const string& bus_name) const {
    if (busname_to_bus_.count(bus_name) == 0) {
        return BusInfo{bus_name, false, 0, 0, 0, 0};
    }

    const Bus* bus = busname_to_bus_.at(bus_name);
    int size = bus->stops.size();

    double geo_dist = 0;
    int dist = 0;
    unordered_set<string_view> names;
    names.insert(bus->stops[0]->name);

    for (int i = 1; i < size; ++i) {
        geo_dist += geo::ComputeDistance({bus->stops[i - 1]->coordinates}, {bus->stops[i]->coordinates});
        dist += GetDistance(bus->stops[i - 1]->name, bus->stops[i]->name);
        names.insert(bus->stops[i]->name);
    }

    return {bus_name, true, size, static_cast<int>(names.size()), dist, geo_dist};
}

void TransportCatalogue::SetRouteSettings(RouteSettings route_settings) {
    route_settings_ = route_settings;
}

const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetAllBusesInfo() const {
    return busname_to_bus_;
}

graph::VertexId TransportCatalogue::GetStopId(std::string_view stop_name) const {
    const Stop* stop_from = stopname_to_stop_.at(stop_name);
    return stop_from->id;
}
