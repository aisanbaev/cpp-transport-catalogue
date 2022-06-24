#include "transport_catalogue.h"
#include <iostream>

#include <unordered_set>

using namespace std;

graph::DirectedWeightedGraph<double> TransportCatalogue::CreateGraph(const RouteSettings& route_settings) {
    graph::DirectedWeightedGraph<double> result(stop_id_ + 1);

    for (const auto& [_, bus_ptr] : busname_to_bus_) {
        const Bus* bus = bus_ptr;
        int size = bus->stops.size();

        if (size != bus->number_stops) { // если маршрут некольцевой

            for (int i = 0; i < bus->number_stops - 1; ++i) {
                int compute_dist = 0;
                for (int j = i + 1; j < bus->number_stops; ++j) {
                    compute_dist += GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
                    double dist_time =  compute_dist / route_settings.bus_velocity * 0.06;
                    result.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + route_settings.bus_wait_time});
                    routes_.push_back({bus, bus->stops[i], j - i, dist_time});
                }
            }

            for (int i = bus->number_stops - 1; i < size - 1; ++i) {
                int compute_dist = 0;
                for (int j = i + 1; j < size; ++j) {
                    compute_dist += GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
                    double dist_time =  compute_dist / route_settings.bus_velocity * 0.06;
                    result.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + route_settings.bus_wait_time});
                    routes_.push_back({bus, bus->stops[i], j - i, dist_time});
                }
            }

        } else if (size == bus->number_stops){ // если маршрут кольцевой
            --size;

            for (int i = 0; i < size; ++i) {
                int compute_dist = 0;
                for (int j = i + 1; j <= size; ++j) {
                    compute_dist += GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
                    double dist_time =  compute_dist / route_settings.bus_velocity * 0.06;
                    result.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + route_settings.bus_wait_time});
                    routes_.push_back({bus, bus->stops[i], j - i, dist_time});
                }
            }
        }
    }

    return result;
}

void TransportCatalogue::AddStop(const string& stop_name, const geo::Coordinates& coordinates) {
    ++stop_id_;
    const Stop* stop = &stops_.emplace_back(Stop{stop_name, coordinates, stop_id_});

    stopname_to_stop_[stop->name] = stop;
    stop_to_buses_[stop->name];
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

const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetAllBusesInfo() const {
    return busname_to_bus_;
}

const std::vector<RouteStat>& TransportCatalogue::GetStatRoutes() const {
    return routes_;
}

graph::VertexId TransportCatalogue::GetStopId(std::string_view stop_name) const {
    const Stop* stop_from = stopname_to_stop_.at(stop_name);
    return stop_from->id;
}
