#include "drivererror.h"
#include <ctime>
using namespace std;

DriverError::DriverError(String msg) : message(msg)
{
    time_t raw_time;
    struct tm* timeinfo = new tm;
    char buffer[80];

    time(&raw_time);
    timeinfo = localtime(&raw_time);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    String s(buffer);

    // FORMAT : YYYY-MM-DD hh:mm:ss
    this->occuredDate = s;
}
