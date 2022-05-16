#pragma once

#include "transport_catalogue.h"

void PrintStopInfo(StopInfo stop);

void PrintBusInfo(BusInfo bus);

void StatRead(std::istream& is, TransportCatalogue& catalogue);
