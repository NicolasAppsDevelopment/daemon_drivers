#ifndef SENSORMEASURE_H
#define SENSORMEASURE_H

#include <string>
using namespace std;

class SensorMeasure
{
private:
    float temperature;
    float humidity;
    float pressure;
    float CO2;
    float O2;
    float luminosity;

public:
    SensorMeasure(float temp, float hum, float pres, float co2, float o2, float lum);
    string getTemperature();
    string getHumidity();
    string getPressure();
    string getCO2();
    string getO2();
    string getLuminosity();
};

#endif // SENSORMEASURE_H
