#include <fstream>
#include <iostream>

#include "json_reader.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    bool make_base = true;

    if (mode == "make_base"sv) {
        ReaderJSON reader(std::cin);
        TransportCatalogue catalogue(reader, make_base);

    } else if (mode == "process_requests"sv) {
        ReaderJSON reader(std::cin);
        TransportCatalogue catalogue(reader, !make_base);
        TransportRouter router(catalogue);

        MapRenderer map_renderer(catalogue.GetRenderSettings(), catalogue.GetAllBuses());
        json::Document json_document = reader.StatReadToJSON(catalogue, router, std::move(map_renderer.GetMapAsString()));
        json::Print(json_document, std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}



