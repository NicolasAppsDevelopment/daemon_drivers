#include "drivererror.h"
#include <ctime>
#include <string>
using namespace std;


DriverError::DriverError(string msg) : message(msg)
{
    time_t raw_time;
    struct tm* timeinfo;
    char buffer[80];

    time(&raw_time);
    timeinfo = localtime(&raw_time);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    string s(buffer);

    // FORMAT : YYYY-MM-DD hh:mm:ss
    this->occuredDate = s;
}
