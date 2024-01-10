#ifndef MEASUREMODULE_H
#define MEASUREMODULE_H

#include <list>
#include <thread>
#include <drivererror.h>
#include <sensormeasure.h>
#include <mutex>

#include "STC31-driver/stc31.h"
#include "SHTC3-driver/shtc3.h"
using namespace std;

#define NB_TEMPERATURE_SENSOR 2
#define NB_HUMIDITY_SENSOR 1
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
        void SHTC3_measure_clock();
        void BME680_measure_clock();
        void STC31_calibration_clock();
        void reset_function();

        list<DriverError> error_array;
        void addTemperatureSample(float);
        void addPressureSample(float);
        void addHumiditySample(float);
        void addCO2Sample(float);
        void addO2Sample(float);
        void addLuminositySample(float);

        float pressureAtSeaLevel(float temperature, float pressure, float altitude);

        float getAverage(list<float> array);

        int altitude;
        bool stopped;
        bool initialising;
        mutex STC31_driver_mutex;

        STC31Driver STC31_driver;
        SHTC3Driver SHTC3_driver;

    public:
        MeasureModule();
        void reset();
        SensorMeasure* get();
        void setAltitude(int altitude);
        string get_errors();
        bool isInitialising();
};

#endif // MEASUREMODULE_H
