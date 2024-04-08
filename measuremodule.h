#ifndef MEASUREMODULE_H
#define MEASUREMODULE_H

#include <list>
#include <thread>
#include "drivererror.h"
#include "sensormeasure.h"
#include <mutex>

#include "STC31-driver/stc31.h"
#include "SHTC3-driver/shtc3.h"
#include "LightSensor-driver/grovelightsensor.h"
#include "Fibox-driver/FiboxDriver.h"
#include "MeasureConfig.h"
using namespace std;

#define NB_TEMPERATURE_SENSOR 2
#define NB_HUMIDITY_SENSOR 1
#define NB_PRESSURE_SENSOR 2
#define NB_CO2_SENSOR 1
#define NB_O2_SENSOR 1
#define NB_LUMINOSITY_SENSOR 1

// Number of samples to average
#define NB_OF_SAMPLE 10

class MeasureModule
{
    private:
        list<float> temperatureArray, humidityArray, pressureArray, co2Array, o2Array, luminosityArray;

        /**
         * @brief Reads data from the STC31 sensor (co2 and temperature) each seconds.
         * It stores the data in the corresponding arrays.
         */
        void stc31MeasureClock();

        /**
         * @brief Reads data from the SHTC3 sensor (temperature and humidity) each seconds.
         * It stores the data in the corresponding arrays.
         */
        void shtc3MeasureClock();

        /**
         * @brief Reads data from the BME680 sensor (pressure) each seconds.
         * It stores the data in the corresponding arrays.
         */
        void bme680MeasureClock();

        /**
         * @brief Reads data from the Fibox sensor (luminosity) each seconds.
         * It stores the data in the corresponding arrays.
         */
        void lightSensorMeasureClock();

        /**
         * @brief Reads data from the Fibox sensor (o2, temperature and pressure) each seconds.
         * It stores the data in the corresponding arrays.
         */
        void fiboxMeasureClock();

        /**
         * @brief Calibrates the STC31 sensor each 5 seconds.
         * It calculates the average of temperature, humidity, pressure and process it with the altitude.
         */
        void stc31CalibrationClock();

        /**
         * @brief Reset all the sensors.
         * It pauses the measure clocks and reset the data arrays.
         * It also initialise all the sensors.
         */
        void processReset();

        /**
         * @brief The list of errors that occurred in the driver.
         */
        list<DriverError> errorArray;

        /**
         * @brief Adds a temperature sample to the corresponding array.
         * It also removes the oldest sample if the array is full.
         * 
         * @param sample The temperature sample to add.
         */
        void addTemperatureSample(float);

        /**
         * @brief Adds a pressure sample to the corresponding array.
         * It also removes the oldest sample if the array is full.
         * 
         * @param sample The pressure sample to add.
         */
        void addPressureSample(float);

        /**
         * @brief Adds a humidity sample to the corresponding array.
         * It also removes the oldest sample if the array is full.
         * 
         * @param sample The humidity sample to add.
         */
        void addHumiditySample(float);

        /**
         * @brief Adds a co2 sample to the corresponding array.
         * It also removes the oldest sample if the array is full.
         * 
         * @param sample The co2 sample to add.
         */
        void addCo2Sample(float);

        /**
         * @brief Adds a o2 sample to the corresponding array.
         * It also removes the oldest sample if the array is full.
         * 
         * @param sample The o2 sample to add.
         */
        void addO2Sample(float);

        /**
         * @brief Adds a luminosity sample to the corresponding array.
         * It also removes the oldest sample if the array is full.
         * 
         * @param sample The luminosity sample to add.
         */
        void addLuminositySample(float);

        /**
         * @brief Calculates the pressure at sea level.
         * 
         * @param temperature The temperature.
         * @param pressure The pressure.
         * @param altitude The altitude.
         * @return The pressure at sea level.
         */
        float pressureAtSeaLevel(float temperature, float pressure, float altitude);

        /**
         * @brief Calculates the average of the given array.
         * It also remove the maximum and minimum values before process the average.
         *
         * @param array The array to process.
         * @return The average of the array.
         */
        float getAverage(list<float> array);

        /**
         * @brief True if the measure module is stopped (mainly due to a sesnor error or a reset).
         * If true, all the measure and calibration clocks are paused.
         */
        bool stopped;

        /**
         * @brief True if the measure module is initialising.
         * It will be at true during the execution of the processReset().
         */
        bool initialising;

        /**
         * @brief The mutex used to protect the STC31 snsor.
         * It prevents the STC31 sensor to be used by multiple threads at the same time (calibration and measure clocks).
         */
        mutex stc31DriverMutex;

        STC31Driver stc31Driver;
        SHTC3Driver shtc3Driver;
        GroveLightSensorDriver lightSensorDriver;
        FiboxDriver fiboxDriver;

        /**
         * @brief The configuration object used to recalculate and correct measurements.
         */
        MeasureConfig* config;

        /**
         * @brief The oxygen calculation object used to calculate the oxygen level from the raw Fibox values.
         */
        OxygenCalculation* oxyCalculator;

    public:
        /**
         * @brief Constructs a new MeasureModule object.
         * It initialises all the sensors and starts the measure and calibration clocks.
         */
        MeasureModule();

        /**
         * @brief Launch the reset function in a new thread.
         */
        void reset();

        /**
         * @brief Retrieves all the physical values from the sensors.
         * It calculates the average of the values.
         *
         * @return The SensorMeasure object containing all the average physical values.
         */
        SensorMeasure* get();

        /**
        * @brief Sets the configuration of the measurement module.
        *
        * @param altitude The altitude of the measurement.
        * @param F1 The F1 constant.
        * @param M The M constant.
        * @param DPHI1 The DPHI1 constant.
        * @param DPHI2 The DPHI2 constant.
        * @param DKSV1 The DKSV1 constant.
        * @param DKSV2 The DKSV2 constant.
        * @param pressure The pressure of the measurement.
        * @param cal0 The cal0 constant.
        * @param cal2nd The cal2nd constant.
        * @param t0 The t0 constant.
        * @param t2nd The t2nd constant.
        * @param o2Cal2nd The o2Cal2nd constant.
        * @param calibIsHumid The calibIsHumid constant (true if the calibration data has been calculated in a humid environment).
        * @param enableTempFibox The enableTempFibox constant (true to use the temperature sensor of the Fibox device).
        * @param humidMode The humidMode constant (true if measurements are realised in humid environment).
        */
        void setConfig(int altitude, double F1, double M, double DPHI1, double DPHI2, double DKSV1, double DKSV2, double pressure, double cal0, double cal2nd, double t0, double t2nd, double o2Cal2nd, bool calibIsHumid, bool enableTempFibox, bool humidMode);
        
        /**
         * @brief Retrieves the list of errors that occurred in the driver.
         *
         * @return The list of errors that occurred in the driver as a string.
         */
        list<DriverError> getErrors();

        /**
         * @brief Returns if the measure module is initialising.
         *
         * @return True if the measure module is initialising.
         */
        bool isInitialising() const;
};

#endif // MEASUREMODULE_H
