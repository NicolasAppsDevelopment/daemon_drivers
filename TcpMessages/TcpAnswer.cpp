#include "TcpAnswer.h"

TcpAnswer::TcpAnswer(String id)
{
	this->id = id;
	this->success = true;
	this->data = "";
	this->errorCode = 0;
}

String TcpAnswer::toString()
{
	String str = "{\"id\":\"" + this->id + "\", \"success\":" + (this->success ? "true" : "false") + ",";
	if (this->success)
	{
		str += "\"data\":" + (this->data == "" ? "[]" : this->data);
	}
	else
	{
		str += "\"error\": {\"code\":" + to_string(this->errorCode) + ",\"message\":\"" + this->data + "\"}";
	}

	return str + "}\0";
}

void TcpAnswer::setMeasurementsData(SensorMeasure* data) {
	this->data = "{\"CO2\": " + data->getCo2() + ", \"temperature\": " + data->getTemperature() + ", \"humidity\": " + data->getHumidity() + ", \"pressure\": " + data->getPressure() + ", \"O2\": " + data->getO2() + ", \"luminosity\": " + data->getLuminosity() + "}";
}

void TcpAnswer::setMeasurementErrorsData(list<DriverError> data) {
	this->data = "[";

	if (data.size() > 0) {
		for (const DriverError& error : data) {
			this->data += "{\"date\": \"" + error.occuredDate + "\", \"message\": \"" + error.message + "\"},";
		}

		// remove last comma
		this->data = this->data.substr(0, this->data.length() - 1);
	}

	this->data += "]";
}

void TcpAnswer::setError(String error, int code)
{
	this->errorCode = code;
	this->data = error;
	this->success = false;
}
