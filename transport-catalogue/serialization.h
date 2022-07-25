#pragma once

#include "domain.h"

#include <string>
#include <vector>
#include <transport_catalogue.pb.h>

class CatalogueSerializer {
public:
    void SerializeStop(const std::string& name, uint32_t id, double lat, double lng);
    void SerializationBus(const std::string& name, const std::vector<uint32_t>& id_stops, uint32_t num_stops);
    void SerializeDistances(uint32_t stop_depart_id, uint32_t stop_arrival_id, uint32_t distance);
    void SerializeRenderSettings(RenderSettings render_set);
    void SerializeRouteSettings(RouteSettings route_set);

    void SaveTo(std::ostream& output) const;

private:
    catalogue_serialize::StopList stop_list_;
    catalogue_serialize::BusList bus_list_;
    catalogue_serialize::RenderSettings render_settings_;
    catalogue_serialize::RoutingSettings routing_settings_;

    catalogue_serialize::RenderSettings_Color SerializeColor(const svg::Color& color);
};

class CatalogueDeserializer {
public:
    explicit CatalogueDeserializer(const std::string& file_name);

    const catalogue_serialize::StopList& GetStopList() const;
    const catalogue_serialize::BusList& GetBusList() const;
    RenderSettings GetRenderSettings();
    RouteSettings GetRoutingSettings();

private:
    catalogue_serialize::TransportCatalogue db;
    RenderSettings render_settings_;
    RouteSettings routing_settings_;

    void DeserializeRenderSetting();
    void DeserializeRoutingSettings();
    svg::Color DeserializeColor(const catalogue_serialize::RenderSettings_Color& color);
};


