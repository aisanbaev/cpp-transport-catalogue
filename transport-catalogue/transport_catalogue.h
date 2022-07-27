#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include <deque>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "serialization.h"

struct StopInfo {
    std::string stop_name;
    bool find_stop;
    std::set<std::string_view> buses;
};

struct BusInfo {
    std::string bus_name;
    bool find_bus;
    int num_stops;
    int num_unique_stops;
    int distance;
    double geo_distance;
};

class ReaderJSON;

class TransportCatalogue {
    friend class TransportRouter;
public:
    explicit TransportCatalogue(const ReaderJSON& reader);
    explicit TransportCatalogue(const CatalogueDeserializer& data_base);

    void AddStop(const std::string& stop_name, const geo::Coordinates& coordinates);
    void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stop_names, int num_stops);

    void SetDistance(const std::string_view stop_departure, const std::string_view stop_arrival, int distance);
    int GetDistance (const std::string_view stop_departure, const std::string_view stop_arrival) const;

    StopInfo GetStopInfo(const std::string& stop_name) const;
    BusInfo GetBusInfo(const std::string& bus_name) const;

    void SetRouteSettings(RouteSettings route_settings);

    uint32_t GetStopId(std::string_view stop_name) const;

    const std::unordered_map<std::string_view, const Bus*>& GetAllBuses() const;

    const std::deque<Stop>& GetStops() const;
    const std::deque<Bus>& GetBuses() const;

    RenderSettings GetRenderSettings() const;
    RouteSettings GetRoutingSettings() const;

private:
    uint32_t stop_id_ = 0;

    RouteSettings route_settings_;
    RenderSettings render_settings_;
    CatalogueSerializer catalogue_serializer_;

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<uint32_t, const Stop*> stop_id_to_stop_;

    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;

    struct StopPairHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*> stop_pair) const {
			return std::hash<const void*>{}(stop_pair.first) * (37) + std::hash<const void*>{}(stop_pair.second);
        }
	private:
		std::hash<const void*> hasher_;
	};

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> stop_ptr_pair_to_distance_;

    void TransferDataFromJSON(const ReaderJSON& reader);
    void TransferDataFromDatabase(const CatalogueDeserializer& data_base);
};


