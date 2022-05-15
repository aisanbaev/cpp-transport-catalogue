#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>

#include "geo.h"

struct StopInfo {
    std::string stop_name;
    bool find_stop;
    std::string buses;
};

struct BusInfo {
    std::string bus_name;
    int num_stops;
    size_t num_unique_stops;
    int distance;
    double geo_distance;
};

class TransportCatalogue {
public:
    void AddStop(const std::string& stop_name, Coordinates coordinates);

    void AddBus(const std::string& bus_name, std::vector<std::string_view> stop_names);

    void AddDistance(const std::string_view stop_name, std::unordered_map<std::string_view, int> stop_to_distance);

    StopInfo GetStopInfo(std::string stop_name);

    BusInfo GetBusInfo(std::string bus_name);

private:
    struct Stop {
        std::string name;
        Coordinates coordinates;
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
    };

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;

    struct StopPairHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*> stop_pair) const {
			return std::hash<const void*>{}(stop_pair.first) * (37) + std::hash<const void*>{}(stop_pair.second);
        }
	private:
		std::hash<const void*> hasher_;
	};

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> stop_ptr_pair_to_distance_;
};
