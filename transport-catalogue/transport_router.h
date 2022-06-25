#pragma once

#include "transport_catalogue.h"

class TransportRouter {
public:
    TransportRouter(const TransportCatalogue& catalogue);

    const graph::DirectedWeightedGraph<double>& GetTransportGraph() const;
    const std::vector<RouteStat>& GetStatRoutes() const;

private:
    const TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<double> transport_graph_;

    std::vector<RouteStat> routes_;

    void CreateGraph();
};
