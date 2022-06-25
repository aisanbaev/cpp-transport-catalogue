#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <tuple>

class ReaderJSON {
public:
    ReaderJSON(std::istream& is);

    void TransferDataToCatalogue(TransportCatalogue& catalogue);
    void ReadRenderSettings(RenderSettings& render_set);
    RouteSettings ReadRoutingSettings() const;

    json::Document StatReadToJSON(TransportCatalogue& catalogue, const TransportRouter& transport_router, const std::string& svg_doc);

private:
    json::Document json_document_{nullptr};

    const json::Array& GetBaseQueries() const;
    const json::Array& GetStatQueries() const;
    const json::Dict& GetRenderSettings() const;
    const json::Dict& GetRoutingSettings() const;

    json::Dict StatReadBus(const json::Node& stat_query, const TransportCatalogue& catalogue);
    json::Dict StatReadStop(const json::Node& stat_query, const TransportCatalogue& catalogue);
    json::Dict StatReadSVG(const json::Node& stat_query, const std::string& svg_doc);
    json::Dict StatReadRoute(const json::Node& stat_query, const TransportCatalogue& catalogue, const TransportRouter& transport_router);
};
