#ifndef SENSORMEASURE_H
#define SENSORMEASURE_H


class SensorMeasure
{
public:
    SensorMeasure(float temp, float hum, float pres, float co2, float o2, float lum);
    float temperature;
    float humidity;
    float pressure;
    float CO2;
    float O2;
    float luminosity;
};

#endif // SENSORMEASURE_H
