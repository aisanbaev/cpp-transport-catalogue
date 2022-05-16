#include "stat_reader.h"
#include "input_reader.h"

#include <iostream>
#include <iomanip>

using namespace std;

void PrintStopInfo(StopInfo stop) {
    if (!stop.find_stop) {
        cout << "Stop "s << stop.stop_name << ": not found" << endl;
    } else if (stop.buses.empty()) {
        cout << "Stop "s << stop.stop_name << ": no buses" << endl;
    } else {
        cout << "Stop "s << stop.stop_name << ": buses"s << stop.buses << endl;
    }

}

void PrintBusInfo(BusInfo bus) {
    if (bus.num_stops == 0) {
        cout << "Bus "s << bus.bus_name << ": not found"s << endl;
        return;
    }
    cout << "Bus "s << bus.bus_name << ": "s <<
            bus.num_stops << " stops on route, "s <<
            bus.num_unique_stops << " unique stops, "s <<
            bus.distance << setprecision(6) <<
            " route length, "s << bus.distance / bus.geo_distance <<
            " curvature"s << endl;
}

void StatRead(std::istream& is, TransportCatalogue& catalogue) {
    string output_count;
    getline (is, output_count);

    for (int i = 0; i < stoi(output_count); ++i) {
        string s;
        getline (is, s);

        size_t start = s.find_first_not_of(' ');
        const char first_symbol = s[start];

        if (first_symbol == 'S') {
            auto stop_info = catalogue.GetStopInfo(ParseOutputQuery(s));
            PrintStopInfo(stop_info);
        } else if (first_symbol == 'B') {
            auto bus_info = catalogue.GetBusInfo(ParseOutputQuery(s));
            PrintBusInfo(bus_info);
        }

    }

}
