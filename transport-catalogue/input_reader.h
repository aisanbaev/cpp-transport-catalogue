#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <tuple>

class Reader {
public:
    void InputRead(std::istream& is, TransportCatalogue& catalogue);

private:
    std::vector<std::string> stop_queries_;
    std::vector<std::string> bus_queries_;

    void Load(const std::string& str);
    void TransferDataToCatalogue(TransportCatalogue& catalogue);

    std::vector<std::string_view> Split(std::string_view str, char separator);

    std::tuple<std::string, Coordinates> ParseStopQueryToNameAndGeo (const std::string& str);
    std::tuple<std::string_view, std::unordered_map<std::string_view, int>> ParseStopQueryToDistances (const std::string& str);

    std::tuple<std::string, std::vector<std::string_view>> ParseBusQuery(const std::string& str);
};

std::string ParseOutputQuery(const std::string& str);
std::string_view ClearWord (std::string_view s);
