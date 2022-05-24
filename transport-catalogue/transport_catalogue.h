#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include <deque>

#include "domain.h"

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

class TransportCatalogue {
public:
    void AddStop(const std::string& stop_name, const geo::Coordinates& coordinates);

    void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stop_names, int num_stops);

    void SetDistance(const std::string_view stop_departure, const std::string_view stop_arrival, int distance);

    int GetDistance (const std::string_view stop_departure, const std::string_view stop_arrival) const;

    StopInfo GetStopInfo(const std::string& stop_name) const;

    BusInfo GetBusInfo(const std::string& bus_name) const;

    const std::unordered_map<std::string_view, const Bus*>& GetAllBusesInfo() const;

private:
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


