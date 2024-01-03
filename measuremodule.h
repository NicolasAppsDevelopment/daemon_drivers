#ifndef MEASUREMODULE_H
#define MEASUREMODULE_H

#include <list>
#include <thread>
#include <drivererror.h>
#include <sensormeasure.h>
using namespace std;

#define NB_TEMPERATURE_SENSOR 3
#define NB_HUMIDITY_SENSOR 2
#define NB_PRESSURE_SENSOR 1
#define NB_CO2_SENSOR 1
#define NB_O2_SENSOR 1
#define NB_LUMINOSITY_SENSOR 1

#define NB_OF_SAMPLE 10

class MeasureModule
{
    private:
        list<float> temperature_array, humidity_array, pressure_array, CO2_array, O2_array, luminosity_array;
        void STC31_measure_clock();
        void BME680_measure_clock();

        list<DriverError> error_array;
        void addTemperatureSample(float);
        void addPressureSample(float);
        void addHumiditySample(float);
        void addCO2Sample(float);
        void addO2Sample(float);
        void addLuminositySample(float);

        float pressureAtSeaLevel(float temperature, float pressure, float altitude);

    public:
        MeasureModule();
        void reset();
        SensorMeasure* get(int altitude);
        void calibrate_STC31_sensor(int pressure, int humidity);
        string get_errors();
};

#endif // MEASUREMODULE_H
