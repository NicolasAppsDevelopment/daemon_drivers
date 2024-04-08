#ifndef DRIVERERROR_H
#define DRIVERERROR_H

#include "types.h"
using namespace std;

/**
 * @brief The DriverError class represents an error that occurred in the driver.
 */
class DriverError
{
public:
    /**
    * @brief Constructs a new DriverError object with the given message.
    * 
    * @param msg The error message associated with the DriverError.
    */
    DriverError(String msg);
    
    String message;
    String occuredDate;
};

#endif // DRIVERERROR_H
