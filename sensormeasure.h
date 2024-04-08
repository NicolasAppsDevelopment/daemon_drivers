#ifndef SENSORMEASURE_H
#define SENSORMEASURE_H

#include "types.h"
using namespace std;

/**
 * @brief The SensorMeasure class represents a measure of the sensors.
 */
class SensorMeasure
{
private:
    float temperature;
    float humidity;
    float pressure;
    float co2;
    float o2;
    float luminosity;
    bool complete;

public:
    SensorMeasure(float temp, float hum, float pres, float co2, float o2, float lum);

    /**
     * @brief Gets the temperature of the measure.
     * 
     * @return The temperature of the measure as string.
     */
    String getTemperature();

    /**
     * @brief Gets the humidity of the measure.
     * 
     * @return The humidity of the measure as string.
     */
    String getHumidity();

    /**
     * @brief Gets the pressure of the measure.
     * 
     * @return The pressure of the measure as string.
     */
    String getPressure();

    /**
     * @brief Gets the co2 of the measure.
     * 
     * @return The co2 of the measure as string.
     */
    String getCo2();

    /**
     * @brief Gets the o2 of the measure.
     * 
     * @return The o2 of the measure as string.
     */
    String getO2();

    /**
     * @brief Gets the luminosity of the measure.
     * 
     * @return The luminosity of the measure as string.
     */
    String getLuminosity();

    /**
	 * @brief Gets if the measure is complete (no null values).
	 * 
	 * @return the result.
	 */
    bool isComplete();
};

#endif // SENSORMEASURE_H
