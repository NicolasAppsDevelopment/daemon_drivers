#include "sensormeasure.h"

SensorMeasure::SensorMeasure(float temp, float hum, float pres, float co2, float o2, float lum)
{
    this->temperature = temp;
    this->humidity = hum;
    this->pressure = pres;
    this->co2 = co2;
    this->o2 = o2;
    this->luminosity = lum;
    this->complete = (temp != __FLT_MIN__ && hum != __FLT_MIN__ && pres != __FLT_MIN__ && co2 != __FLT_MIN__ && o2 != __FLT_MIN__ && lum != __FLT_MIN__);
}

String SensorMeasure::getTemperature()
{
    if (temperature == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(temperature);
    }
}

String SensorMeasure::getHumidity()
{
    if (humidity == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(humidity);
    }
}

String SensorMeasure::getPressure()
{
    if (pressure == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(pressure);
    }
}

String SensorMeasure::getCo2()
{
    if (co2 == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(co2);
    }
}

String SensorMeasure::getO2()
{
    if (o2 == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(o2);
    }
}

String SensorMeasure::getLuminosity()
{
    if (luminosity == __FLT_MIN__) {
        return "null";
    } else {
        return to_string(luminosity);
    }
}

bool SensorMeasure::isComplete()
{
    return this->complete;
}
