#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <tuple>

class ReaderJSON {
public:
    void LoadJSON (std::istream& is);

    void TransferDataToCatalogue(TransportCatalogue& catalogue);
    void ReadRenderSettings(RenderSettings& render_set);

    json::Document StatReadToJSON(TransportCatalogue& catalogue, const std::string& svg_doc);

private:
    json::Document json_document_{nullptr};

    const json::Array& GetBaseQueries() const;
    const json::Array& GetStatQueries() const;
    const json::Dict& GetRenderSettings() const;
};
