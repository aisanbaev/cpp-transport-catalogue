#include "transport_catalogue.h"
#include "json_reader.h"

#include <fstream>
#include <unordered_set>
#include <transport_catalogue.pb.h>

using namespace std;

TransportCatalogue::TransportCatalogue(const ReaderJSON& reader, bool is_make_base) {
    if (is_make_base) {
        TransferDataFromJSON(reader);
    } else {
        TransferDataFromDatabase(reader);
    }
}

void TransportCatalogue::AddStop(const string& stop_name, const geo::Coordinates& coordinates) {
    const Stop* stop = &stops_.emplace_back(Stop{stop_name, coordinates, stop_id_});

    stopname_to_stop_[stop->name] = stop;
    stop_to_buses_[stop->name];
    stop_id_to_stop_[stop_id_] = stop;

    ++stop_id_;
}

void TransportCatalogue::AddBus(const string& bus_name, const vector<string_view>& stop_names, int num_stops) {
    std::vector<const Stop*> stops;

    for (auto s : stop_names) {
        stops.push_back(stopname_to_stop_.at(s));
    }

    const Bus* bus = &buses_.emplace_back(Bus{bus_name, stops, num_stops});
    busname_to_bus_[bus->name] = bus;

    for (const Stop* stop : bus->stops) {
        stop_to_buses_[stop->name].insert(bus->name);
    }
}

void TransportCatalogue::SetDistance(const string_view stop_departure, const string_view stop_arrival, int distance) {
    const Stop* ptr_stop_depart = stopname_to_stop_.at(stop_departure);
    const Stop* ptr_stop_arrival = stopname_to_stop_.at(stop_arrival);

    stop_ptr_pair_to_distance_[{ptr_stop_depart, ptr_stop_arrival}] = distance;
}

int TransportCatalogue::GetDistance (const std::string_view stop_departure, const std::string_view stop_arrival) const {
    const Stop* ptr_stop_depart = stopname_to_stop_.at(stop_departure);
    const Stop* ptr_stop_arrival = stopname_to_stop_.at(stop_arrival);

    return (stop_ptr_pair_to_distance_.count({ptr_stop_depart, ptr_stop_arrival})) ?
            stop_ptr_pair_to_distance_.at({ptr_stop_depart, ptr_stop_arrival}) :
            stop_ptr_pair_to_distance_.at({ptr_stop_arrival, ptr_stop_depart});
}

StopInfo TransportCatalogue::GetStopInfo(const string& stop_name) const {
    if (stop_to_buses_.count(stop_name) == 0) {
        return {stop_name, false, {}};
    }

    set<string_view> print_set = stop_to_buses_.at(stop_name);
    if (print_set.empty()) {
        return {stop_name, true, {}};
    }

    return {stop_name, true, print_set};
}

BusInfo TransportCatalogue::GetBusInfo (const string& bus_name) const {
    if (busname_to_bus_.count(bus_name) == 0) {
        return BusInfo{bus_name, false, 0, 0, 0, 0};
    }

    const Bus* bus = busname_to_bus_.at(bus_name);
    int size = bus->stops.size();

    double geo_dist = 0;
    int dist = 0;
    unordered_set<string_view> names;
    names.insert(bus->stops[0]->name);

    for (int i = 1; i < size; ++i) {
        geo_dist += geo::ComputeDistance({bus->stops[i - 1]->coordinates}, {bus->stops[i]->coordinates});
        dist += GetDistance(bus->stops[i - 1]->name, bus->stops[i]->name);
        names.insert(bus->stops[i]->name);
    }

    return {bus_name, true, size, static_cast<int>(names.size()), dist, geo_dist};
}

void TransportCatalogue::SetRouteSettings(RouteSettings route_settings) {
    route_settings_ = route_settings;
}

const std::unordered_map<std::string_view, const Bus*>& TransportCatalogue::GetAllBuses() const {
    return busname_to_bus_;
}

RenderSettings TransportCatalogue::GetRenderSettings() const {
    return render_settings_;
}

RouteSettings TransportCatalogue::GetRoutingSettings() const {
    return route_settings_;
}

graph::VertexId TransportCatalogue::GetStopId(std::string_view stop_name) const {
    const Stop* stop_from = stopname_to_stop_.at(stop_name);
    return stop_from->id;
}

void TransportCatalogue::TransferDataFromJSON(const ReaderJSON& reader) {

    for (const auto& stop_query : reader.GetBaseRequests()) {
        if (stop_query.AsDict().at("type"s) != "Stop"s) {
            continue;
        }
        string stop_name = stop_query.AsDict().at("name"s).AsString();
        double lat = stop_query.AsDict().at("latitude"s).AsDouble();
        double lng = stop_query.AsDict().at("longitude"s).AsDouble();
        AddStop(stop_name, {lat,lng});
        catalogue_serializer_.SerializeStop(stop_name, stop_id_, lat, lng);
    }

    for (const auto& bus_query : reader.GetBaseRequests()) {
        if (bus_query.AsDict().at("type"s) != "Bus"s) {
            continue;
        }
        string bus_name = bus_query.AsDict().at("name"s).AsString();
        const json::Array& stops = bus_query.AsDict().at("stops"s).AsArray();
        vector<string_view> stop_names;

        for (const auto& node : stops) {
            stop_names.push_back(node.AsString());
        }

        int num_stops = stop_names.size();

        if (!bus_query.AsDict().at("is_roundtrip"s).AsBool()) {
            stop_names.reserve(2 * stop_names.size());
            stop_names.insert(stop_names.end(), stop_names.rbegin() + 1, stop_names.rend());
        }

        AddBus(bus_name, stop_names, num_stops);

        vector<uint32_t> id_stops;
        for (const auto& stop_name : stop_names) {
            id_stops.push_back(stopname_to_stop_.at(stop_name)->id);
        }
        catalogue_serializer_.SerializationBus(bus_name, id_stops, num_stops);
    }

    for (const auto& stop_query : reader.GetBaseRequests()) {
        if (stop_query.AsDict().at("type"s) != "Stop"s) {
            continue;
        }

        string stop_depart = stop_query.AsDict().at("name"s).AsString();
        const json::Dict& name_to_dist = stop_query.AsDict().at("road_distances"s).AsDict();

        for (const auto& [stop_arrival, distance] : name_to_dist) {
            SetDistance(stop_depart, stop_arrival, distance.AsInt());
            catalogue_serializer_.SerializeDistances(stopname_to_stop_.at(stop_depart)->id, stopname_to_stop_.at(stop_arrival)->id, distance.AsInt());
        }
    }

    catalogue_serializer_.SerializeRenderSettings(std::move(reader.GetRenderSettings()));
    catalogue_serializer_.SerializeRouteSettings(std::move(reader.GetRoutingSettings()));

    const string& file_name = reader.GetSerializationSettings().at("file"s).AsString();
    std::ofstream out(file_name, std::ios::binary);
    catalogue_serializer_.SaveTo(out);
}

void TransportCatalogue::TransferDataFromDatabase(const ReaderJSON& reader) {
    const string& file_name = reader.GetSerializationSettings().at("file"s).AsString();
    CatalogueDeserializer data_base(file_name);

    for (const catalogue_serialize::Stop& stop : data_base.GetStopList().stops()) {
        string stop_name = stop.name();
        double lat = stop.lat();
        double lng = stop.lng();
        AddStop(stop_name, {lat,lng});
    }

    for (const catalogue_serialize::Bus& bus : data_base.GetBusList().buses()) {

        string bus_name = bus.name();
        int num_stops = bus.num_stops();

        vector<string_view> stop_names;
        for (const uint32_t stop_id : bus.id_stops()) {
            stop_names.push_back(stop_id_to_stop_.at(stop_id)->name);
        }

        AddBus(bus_name, stop_names, num_stops);
    }

    for (const catalogue_serialize::Stop& stop : data_base.GetStopList().stops()) {
        string stop_depart = stop.name();

        for (const auto& [stop_arrival_id, distance] : stop.stop_arrival_to_dist()) {
            string stop_arrival = stop_id_to_stop_.at(stop_arrival_id)->name;
            SetDistance(stop_depart, stop_arrival, distance);
        }
    }

    render_settings_ = std::move(data_base.GetRenderSettings());
    route_settings_ = std::move(data_base.GetRoutingSettings());
}

