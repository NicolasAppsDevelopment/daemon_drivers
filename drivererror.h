#ifndef DRIVERERROR_H
#define DRIVERERROR_H

#include <string>
using namespace std;

class DriverError
{
public:
    DriverError(string msg);
    string message;
    string occuredDate;
};

#endif // DRIVERERROR_H
