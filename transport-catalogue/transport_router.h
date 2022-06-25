#pragma once

#include "transport_catalogue.h"

class TransportRouter {
public:
    TransportRouter(const TransportCatalogue& catalogue);

    const std::optional<std::vector<RouteStat>> GetRouteInfo(graph::VertexId stop_from_id, graph::VertexId stop_to_id) const;

private:
    const TransportCatalogue& catalogue_;
    std::vector<RouteStat> routes_;

    graph::DirectedWeightedGraph<double> transport_graph_;
    graph::Router<double> router_;

    const graph::DirectedWeightedGraph<double>& CreateGraph();
    void AddEdges(const Bus* bus, int from_left, int to_right);
};
