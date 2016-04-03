#include <time.h>

using namespace std;

#include "Util.h"

// Obtenemos la fecha con el siguiente formato -> YYYY-MM-DD.HH:mm:ss
string Util::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

