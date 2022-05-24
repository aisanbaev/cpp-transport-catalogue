#include <fstream>
#include <sstream>
#include <iostream>

#include "geo.h"
#include "json_reader.h"
#include "map_renderer.h"

int main () {

    //std::ifstream input("json.txt");

    TransportCatalogue catalogue;
    ReaderJSON reader;

    reader.LoadJSON(std::cin);
    reader.TransferDataToCatalogue(catalogue);

    RenderSettings render_setting;
    reader.ReadRenderSettings(render_setting);

    auto all_routes = catalogue.GetAllBusesInfo();
    svg::Document document = CreateRoutesMap(render_setting, all_routes);

    std::ostringstream routes_map_stream;
    document.Render(routes_map_stream);
    std::string routes_map_str = routes_map_stream.str();

    json::Document json_doc = reader.StatReadToJSON(catalogue, routes_map_str);
    json::Print(json_doc, std::cout);

    return 0;
}
