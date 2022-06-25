#include "transport_router.h"

TransportRouter::TransportRouter(const TransportCatalogue& catalogue)
    : catalogue_(catalogue)
    , transport_graph_(catalogue_.stop_id_) {

    CreateGraph();
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetTransportGraph() const {
    return transport_graph_;
}

const std::vector<RouteStat>& TransportRouter::GetStatRoutes() const {
    return routes_;
}

void TransportRouter::CreateGraph() {
    const double CONVERSION_TO_MINS = 0.06;

    for (const auto& [_, bus_ptr] : catalogue_.busname_to_bus_) {
        const Bus* bus = bus_ptr;
        int size = bus->stops.size();

        if (size != bus->number_stops) { // если маршрут некольцевой

            for (int i = 0; i < bus->number_stops - 1; ++i) {
                int compute_dist = 0;
                for (int j = i + 1; j < bus->number_stops; ++j) {
                    compute_dist += catalogue_.GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
                    double dist_time =  compute_dist / catalogue_.route_settings_.bus_velocity * CONVERSION_TO_MINS;
                    transport_graph_.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + catalogue_.route_settings_.bus_wait_time});
                    routes_.push_back({bus, bus->stops[i], j - i, dist_time});
                }
            }

            for (int i = bus->number_stops - 1; i < size - 1; ++i) {
                int compute_dist = 0;
                for (int j = i + 1; j < size; ++j) {
                    compute_dist += catalogue_.GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
                    double dist_time =  compute_dist / catalogue_.route_settings_.bus_velocity * CONVERSION_TO_MINS;
                    transport_graph_.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + catalogue_.route_settings_.bus_wait_time});
                    routes_.push_back({bus, bus->stops[i], j - i, dist_time});
                }
            }

        } else if (size == bus->number_stops){ // если маршрут кольцевой
            --size;

            for (int i = 0; i < size; ++i) {
                int compute_dist = 0;
                for (int j = i + 1; j <= size; ++j) {
                    compute_dist += catalogue_.GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
                    double dist_time =  compute_dist / catalogue_.route_settings_.bus_velocity * CONVERSION_TO_MINS;
                    transport_graph_.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + catalogue_.route_settings_.bus_wait_time});
                    routes_.push_back({bus, bus->stops[i], j - i, dist_time});
                }
            }
        }
    }
}
