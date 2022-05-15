#include <iomanip>
#include <fstream>

#include "geo.h"
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main () {

    //ifstream input("tsB_case1_input.txt");

    istringstream input{
        "13 \n"
        "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino \n"
        "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino \n"
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye \n"
        "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka \n"
        "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino \n"
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam \n"
        "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam \n"
        "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya \n"
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya \n"
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye \n"
        "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye \n"
        "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757 \n"
        "Stop Prazhskaya: 55.611678, 37.603831 \n"
        "6 \n"
        "Bus 256 \n"
        "Bus 750 \n"
        "Bus 751 \n"
        "Stop Samara \n"
        "Stop Prazhskaya \n"
        "Stop Biryulyovo Zapadnoye \n"
    };

    Reader reader;

    string query_count;
    getline (input, query_count);

    for (int i = 0; i < stoi(query_count); ++i) {
        string s;
        getline (input, s);
        reader.Load(move(s));
    }

    TransportCatalogue catalogue;
    reader.TransferDataToCatalogue(catalogue);


    string output_count;
    getline (input, output_count);

    for (int i = 0; i < stoi(output_count); ++i) {
        string s;
        getline (input, s);

        size_t start = s.find_first_not_of(' ');
        const char first_symbol = s[start];

        if (first_symbol == 'S') {
            auto stop_info = catalogue.GetStopInfo(reader.ParseOutputQuery(s));
            PrintStopInfo(stop_info);
        } else if (first_symbol == 'B') {
            auto bus_info = catalogue.GetBusInfo(reader.ParseOutputQuery(s));
            PrintBusInfo(bus_info);
        }

    }

    return 0;
}
