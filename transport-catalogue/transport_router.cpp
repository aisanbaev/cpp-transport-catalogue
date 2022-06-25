#include "transport_router.h"

TransportRouter::TransportRouter(const TransportCatalogue& catalogue)
    : catalogue_(catalogue)
    , transport_graph_(catalogue_.stop_id_)
    , router_(CreateGraph()) {
}

const std::optional<std::vector<RouteStat>> TransportRouter::GetRouteInfo(graph::VertexId stop_from_id, graph::VertexId stop_to_id) const {
    auto route_info = router_.BuildRoute(stop_from_id, stop_to_id);
    std::vector<RouteStat> result;

    if (route_info) {

        for (int num_edge : (*route_info).edges) {
            result.push_back(routes_[num_edge]);
        }

        return result;

    } else {
        return std::nullopt;
    }
}

void TransportRouter::AddEdges(const Bus* bus, int from_left, int to_right) {
    const double CONVERSION_TO_MINS = 0.06;

    for (int i = from_left; i < to_right - 1; ++i) {
        int compute_dist = 0;

        for (int j = i + 1; j < to_right; ++j) {
            compute_dist += catalogue_.GetDistance(bus->stops[j - 1]->name, bus->stops[j]->name);
            double dist_time =  compute_dist / catalogue_.route_settings_.bus_velocity * CONVERSION_TO_MINS;
            transport_graph_.AddEdge({bus->stops[i]->id, bus->stops[j]->id, dist_time + catalogue_.route_settings_.bus_wait_time});
            routes_.push_back({bus, bus->stops[i], j - i, dist_time});
        }

    }
}

const graph::DirectedWeightedGraph<double>& TransportRouter::CreateGraph() {

    for (const auto& [_, bus_ptr] : catalogue_.busname_to_bus_) {
        const Bus* bus = bus_ptr;
        int size = bus->stops.size();

        AddEdges(bus, 0, bus->number_stops);            // добавляем прямые маршруты

        if (size != bus->number_stops) {                // если маршрут некольцевой
            AddEdges(bus, bus->number_stops - 1, size); // добавляем обратные маршруты
        }
    }
    return transport_graph_;
}
