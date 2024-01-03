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
