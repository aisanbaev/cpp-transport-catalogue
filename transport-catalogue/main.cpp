#include <iomanip>
#include <fstream>

#include "geo.h"
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main () {
    Reader reader;
    TransportCatalogue catalogue;

    reader.InputRead(cin, catalogue);

    StatRead(cin, catalogue);

    return 0;
}
