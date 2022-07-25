#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <vector>
#include <string>
#include <string_view>

class ReaderJSON {
public:
    ReaderJSON(std::istream& is);

    json::Document StatReadToJSON(TransportCatalogue& catalogue, const TransportRouter& transport_router, const std::string& svg_doc);
    json::Document StatReadToJSON(TransportCatalogue& catalogue, const std::string& svg_doc);

    const json::Array& GetBaseRequests() const;
    const json::Dict& GetSerializationSettings() const;
    RenderSettings GetRenderSettings() const;
    RouteSettings GetRoutingSettings() const;

private:
    json::Document json_document_{nullptr};

    const json::Array& ReadStatRequests() const;
    const json::Dict& ReadRenderSettings() const;
    const json::Dict& ReadRoutingSettings() const;

    json::Dict StatReadBus(const json::Node& stat_query, const TransportCatalogue& catalogue);
    json::Dict StatReadStop(const json::Node& stat_query, const TransportCatalogue& catalogue);
    json::Dict StatReadSVG(const json::Node& stat_query, const std::string& svg_doc);
    json::Dict StatReadRoute(const json::Node& stat_query, const TransportRouter& transport_router);
    json::Dict StatReadRoute(const json::Node& stat_query,const TransportCatalogue& catalogue, const TransportRouter& transport_router);
};
