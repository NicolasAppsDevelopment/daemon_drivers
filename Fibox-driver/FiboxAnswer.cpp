#include "FiboxAnswer.h"

FiboxAnswer::FiboxAnswer(double temperature, double pressure, double phase, std::map<UInt, String> errors)
{
	this->temperature = temperature;
	this->pressure = pressure;
	this->phase = phase;
	this->errors = errors;
	this->isTemperatureEnabled = false;
}
