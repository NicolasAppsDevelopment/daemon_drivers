#include "sensormeasure.h"

SensorMeasure::SensorMeasure(float temp, float hum, float pres, float co2, float o2, float lum)
{
    this->temperature = temp;
    this->humidity = hum;
    this->pressure = pres;
    this->CO2 = co2;
    this->O2 = o2;
    this->luminosity = lum;
}

string SensorMeasure::getTemperature()
{
    if (temperature == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(temperature);
    }
}

string SensorMeasure::getHumidity()
{
    if (humidity == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(humidity);
    }
}

string SensorMeasure::getPressure()
{
    if (pressure == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(pressure);
    }
}

string SensorMeasure::getCO2()
{
    if (CO2 == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(CO2);
    }
}

string SensorMeasure::getO2()
{
    if (O2 == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(O2);
    }
}

string SensorMeasure::getLuminosity()
{
    if (luminosity == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(luminosity);
    }
}
