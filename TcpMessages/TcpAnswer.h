#pragma once

#include "../types.h"
#include "../sensormeasure.h"
#include "../drivererror.h"
#include <list>
using namespace std;


/**
 * @brief TCP answer builder/helper class.
 */
class TcpAnswer
{
public:
	int errorCode;
	String id;
	String data;

	/**
	 * The success state of the query previously asked
	 */
	bool success;

	/**
	 * Constructor building and empty tcp answer request with a specific id
	 * Use the same id as the request to link the answer to the request
	 */
	TcpAnswer(String id);

	/**
	 * Returns the JSON string representation of the TcpAnswer
	 * Use this into to write data over the client socket
	 */
	String toString();

	void setMeasurementsData(SensorMeasure* data);
	void setMeasurementErrorsData(list<DriverError> data);
	void setError(String error, int code = -1);
};
