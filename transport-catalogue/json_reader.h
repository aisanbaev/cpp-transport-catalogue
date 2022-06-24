#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <tuple>

class ReaderJSON {
public:
    void LoadJSON (std::istream& is);

    void TransferDataToCatalogue(TransportCatalogue& catalogue);
    void ReadRenderSettings(RenderSettings& render_set);

    json::Document StatReadToJSON(TransportCatalogue& catalogue, const graph::Router<double>& router, const std::string& svg_doc);

    RouteSettings ReadRoutingSettings() const;
private:
    json::Document json_document_{nullptr};

    const json::Array& GetBaseQueries() const;
    const json::Array& GetStatQueries() const;
    const json::Dict& GetRenderSettings() const;
    const json::Dict& GetRoutingSettings() const;

    json::Dict StatReadBus(const json::Node& stat_query, const TransportCatalogue& catalogue);
    json::Dict StatReadStop(const json::Node& stat_query, const TransportCatalogue& catalogue);
    json::Dict StatReadSVG(const json::Node& stat_query, const std::string& svg_doc);
    json::Dict StatReadRoute(const json::Node& stat_query, const TransportCatalogue& catalogue, const graph::Router<double>& router);
};
