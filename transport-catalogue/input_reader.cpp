#include "input_reader.h"

using namespace std;

void Reader::Load(const string& str) {
    size_t start = str.find_first_not_of(' ');
    const char first_symbol = str[start];

    if (first_symbol == 'S') {
        stop_queries_.push_back(move(str));
    } else if (first_symbol == 'B') {
        bus_queries_.push_back(move(str));
    }
}

void Reader::InputRead(std::istream& is, TransportCatalogue& catalogue) {
    string query_count;
    getline (is, query_count);

    for (int i = 0; i < stoi(query_count); ++i) {
        string s;
        getline (is, s);
        Load(move(s));
    }

    TransferDataToCatalogue(catalogue);
}

void Reader::TransferDataToCatalogue(TransportCatalogue& catalogue) {

    for (const string& s : stop_queries_) {
        const auto& [stop_name, geo] = ParseStopQueryToNameAndGeo(s);
        catalogue.AddStop(stop_name, geo);
    }

    for (const string& s : bus_queries_) {
        const auto& [bus_name, stop_names] = ParseBusQuery(s);
        catalogue.AddBus(bus_name, stop_names);
    }

    for (const string& s : stop_queries_) {
        const auto [stop_depart, stop_to_distance] = ParseStopQueryToDistances(s);
        for (const auto [stop_arrival, distance] : stop_to_distance) {
            catalogue.SetDistance(stop_depart, stop_arrival, distance);
        }
    }
}

std::string_view ClearWord (std::string_view s) {
    if (s.empty()) {
        return s;
    }

    const auto left = s.find_first_not_of(' ');
    const auto right = s.find_last_not_of(' ');

    return s.substr(left, right - left + 1);
}

std::vector<std::string_view> Reader::Split(std::string_view str, char separator) {
    if (str.empty()) {
        return {};
    }

    std::vector<std::string_view> result;
    const int64_t pos_end = str.npos;
    int64_t space = 0;

    while (space != pos_end && !str.empty()) {
        space = str.find(separator, 0);
        if (space == 0) {
            str.remove_prefix(space + 1);
            continue;
        }
        result.push_back(ClearWord(str.substr(0, space)));
        str.remove_prefix(space + 1);
    }

    return result;
}

string ParseOutputQuery(const std::string& str) {
    string_view name = str;
    name.remove_prefix(4);
    name = ClearWord(name);

    return string(name);
}

tuple<string, Coordinates> Reader::ParseStopQueryToNameAndGeo (const std::string& str) {
    size_t colon = str.find(':');

    string stop_name = str.substr(5, colon - 5);
    string_view sv = str;
    sv.remove_prefix(colon + 1);

    double lat = stod(string(sv.substr(0, sv.find(','))));
    sv.remove_prefix(sv.find(',') + 1);
    double lng = stod(string(sv.substr(0, sv.find(','))));

    return {stop_name, {lat, lng}};
}

std::tuple<std::string_view, std::unordered_map<std::string_view, int>> Reader::ParseStopQueryToDistances (const std::string& str) {
    size_t colon = str.find(':');

    string_view sv = str;
    string_view stop_name = sv.substr(5, colon - 5);
    sv.remove_prefix(colon + 1);

    vector<string_view> s = Split(sv, ',');

    unordered_map<string_view, int> stop_to_distance;

    for (int i = 2; i < int(s.size()); ++i) {
        size_t div = s[i].find("to"s);
        string_view dist = s[i].substr(0, s[i].find('m'));
        string_view name = s[i].substr(div + 2, s[i].size() - div);
        stop_to_distance[ClearWord(name)] = stoi(string(dist));
    }

    return {stop_name, stop_to_distance};
}

tuple<std::string, std::vector<std::string_view>> Reader::ParseBusQuery(const std::string& str) {
    const size_t colon = str.find(':');

    string bus_name = str.substr(4, colon - 4);
    string_view sv = str;
    sv.remove_prefix(colon + 1);

    vector<string_view> stop_names;

    if (sv.find('-') != std::string::npos) {
        stop_names = Split(sv, '-');
        stop_names.reserve(2 * stop_names.size());
        stop_names.insert(stop_names.end(), stop_names.rbegin() + 1, stop_names.rend());
    } else {
        stop_names = Split(sv, '>');
    }

    return {bus_name, stop_names};
}
