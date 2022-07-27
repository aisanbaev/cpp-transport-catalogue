#pragma once

#include "domain.h"

#include <string>
#include <vector>
#include <transport_catalogue.pb.h>

class TransportCatalogue;

class CatalogueSerializer {
public:
    void SerializeTransportCatalogue(const TransportCatalogue& catalogue);
    void SerializeRenderSettings(RenderSettings render_set);
    void SerializeRouteSettings(RouteSettings route_set);

    void SaveTo(const std::string& file_name) const;

private:
    catalogue_serialize::TransportBase base_;
    catalogue_serialize::RenderSettings render_settings_;
    catalogue_serialize::RoutingSettings routing_settings_;

    void SerializeStop(const Stop& stop);
    void SerializeBus(const Bus& bus);
    void SerializeDistances(const TransportCatalogue& catalogue, const Stop& stop_depart);
    catalogue_serialize::Color SerializeColor(const svg::Color& svg_color);
};

class CatalogueDeserializer {
public:
    explicit CatalogueDeserializer(const std::string& file_name);

    const catalogue_serialize::TransportBase& GetTransportBase() const;

    RenderSettings GetRenderSettings() const;
    RouteSettings GetRoutingSettings() const;

private:
    catalogue_serialize::TransportCatalogue db;
    RenderSettings render_settings_;
    RouteSettings routing_settings_;

    void DeserializeRenderSetting();
    void DeserializeRoutingSettings();
    svg::Color DeserializeColor(const catalogue_serialize::Color& color);
};


