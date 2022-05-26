#include <fstream>
#include <iostream>

#include "geo.h"
#include "json_reader.h"
#include "map_renderer.h"

int main () {

    ReaderJSON reader;
    TransportCatalogue catalogue;

    reader.LoadJSON(std::cin);
    reader.TransferDataToCatalogue(catalogue);

    RenderSettings render_setting;
    reader.ReadRenderSettings(render_setting);

    auto all_routes = catalogue.GetAllBusesInfo();
    MapRenderer map_renderer(render_setting, all_routes);

    svg::Document document = map_renderer.CreateRoutesMap();

    std::string routes_map_str = map_renderer.ToString(document);
    json::Document json_doc = reader.StatReadToJSON(catalogue, routes_map_str);
    json::Print(json_doc, std::cout);

    return 0;
}
