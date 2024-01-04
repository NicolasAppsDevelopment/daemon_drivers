#include "measuremodule.h"
#include "STC31-driver/sensirion_common.h"
#include "STC31-driver/sensirion_i2c_hal.h"
#include "STC31-driver/stc3x_i2c.h"
#include "BME680-driver/common.h"
#include <unistd.h>
#include <math.h>

using namespace std;

void MeasureModule::BME680_measure_clock()
{
    while (true) {
        if (!stopped) {
            try {
                int16_t error = 0;

                float temperature;
                float humidity;
                float pressure;

                error = bme680_get_measure(&temperature, &pressure, &humidity);
                if (error) {
                    if (error != 2) { // DO NOT THROW ERROR IF IT'S A NO NEW DATA ERROR
                        throw DriverError("Impossible de récupérer les données de mesure du capteurs BME680. La fonction [bme680_get_measure] a retourné le code d'erreur : " + to_string(error));
                    }
                } else {
                    addHumiditySample(humidity);
                    addPressureSample(pressure);
                    addTemperatureSample(temperature);
                }
            } catch (const DriverError& e) {
                error_array.push_front(e);
                this->stopped = true;
            } catch (...) {
                error_array.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur BME680."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::STC31_measure_clock()
{
    while (true) {
        if (!stopped) {
            try {
                int16_t error = 0;

                uint16_t gas_ticks;
                uint16_t temperature_ticks;

                float gas;
                float temperature;

                error = stc3x_measure_gas_concentration(&gas_ticks, &temperature_ticks);
                if (error) {
                    throw DriverError("Impossible de récupérer les données de mesure du capteurs STC31. La fonction [stc3x_measure_gas_concentration] a retourné le code d'erreur : " + to_string(error));
                }

                gas = 100 * ((float)gas_ticks - 16384.0) / 32768.0;
                temperature = (float)temperature_ticks / 200.0;

                addCO2Sample(gas);
                addTemperatureSample(temperature);
            } catch (const DriverError& e) {
                error_array.push_front(e);
                this->stopped = true;
            } catch (...) {
                error_array.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur STC31."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::STC31_calibration_clock()
{
    while (true) {
        if (!stopped && !this->STC31_calibrating) {
            try {
                float temperature = __FLT_MIN__;
                try {
                    temperature = getAverage(temperature_array);
                } catch (const DriverError& e) {
                    perror("WARN: Impossible de calibrer le capteur STC31. Série pas assez remplie concernée : température.");
                }

                float humidity = __FLT_MIN__;
                try {
                    humidity = getAverage(humidity_array);
                } catch (const DriverError& e) {
                    perror("WARN: Impossible de calibrer le capteur STC31. Série pas assez remplie concernée : humidité.");
                }

                float pressure = __FLT_MIN__;
                try {
                    pressure = getAverage(pressure_array);

                    if (temperature != __FLT_MIN__ && pressure != __FLT_MIN__) {
                        // convert to pressure at altitude to pressure at sea level
                        pressure = pressureAtSeaLevel(temperature, pressure, this->altitude);
                    }
                } catch (const DriverError& e) {
                    perror("WARN: Impossible de calibrer le capteur STC31. Série pas assez remplie concernée : pression.");
                }

                if (temperature != __FLT_MIN__ && pressure != __FLT_MIN__ && humidity != __FLT_MIN__) {
                    int16_t error = 0;
                    this->STC31_calibrating = true;

                    uint16_t hum = humidity * 65535 / 100;
                    error = stc3x_set_relative_humidity(hum);
                    if (error) {
                        throw DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_relative_humidity] a retourné le code d'erreur : " + to_string(error));
                    }

                    uint16_t pres = pressure;
                    error = stc3x_set_pressure(pres);
                    if (error) {
                        throw DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_pressure] a retourné le code d'erreur : " + to_string(error));
                    }

                }
            } catch (const DriverError& e) {
                error_array.push_front(e);
                this->stopped = true;
            } catch (...) {
                error_array.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de calibration du capteur STC31."));
                this->stopped = true;
            }
        }

        this->STC31_calibrating = false;
        sleep(5);
    }
}

void MeasureModule::addTemperatureSample(float temperature)
{
    temperature_array.push_front(temperature);
    if (temperature_array.size() > NB_OF_SAMPLE * NB_TEMPERATURE_SENSOR) {
        temperature_array.pop_back();
    }
}

void MeasureModule::addPressureSample(float pressure)
{
    pressure_array.push_front(pressure);
    if (pressure_array.size() > NB_OF_SAMPLE * NB_PRESSURE_SENSOR) {
        pressure_array.pop_back();
    }
}

void MeasureModule::addHumiditySample(float humidity)
{
    humidity_array.push_front(humidity);
    if (humidity_array.size() > NB_OF_SAMPLE * NB_HUMIDITY_SENSOR) {
        humidity_array.pop_back();
    }
}

void MeasureModule::addCO2Sample(float co2)
{
    CO2_array.push_front(co2);
    if (CO2_array.size() > NB_OF_SAMPLE * NB_CO2_SENSOR) {
        CO2_array.pop_back();
    }
}

void MeasureModule::addO2Sample(float o2)
{
    O2_array.push_front(o2);
    if (O2_array.size() > NB_OF_SAMPLE * NB_O2_SENSOR) {
        O2_array.pop_back();
    }
}

void MeasureModule::addLuminositySample(float luminosity)
{
    luminosity_array.push_front(luminosity);
    if (luminosity_array.size() > NB_OF_SAMPLE * NB_LUMINOSITY_SENSOR) {
        luminosity_array.pop_back();
    }
}

float MeasureModule::pressureAtSeaLevel(float temperature, float pressure, float altitude)
{
    // Constants
    float gravitational_acceleration = 9.80665;  // Standard gravitational acceleration (m/s^2)

    // Calculate temperature at the given altitude
    float temperature_at_altitude = 273.15 + temperature;

    // Calculate pressure at sea level using the barometric formula
    return pressure * exp((gravitational_acceleration * altitude) / (287.058 * temperature_at_altitude));
}



float MeasureModule::getAverage(list<float> array)
{
    // remove higher & lower value, then process the average
    if (array.size() < NB_OF_SAMPLE) {
        throw DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise de cette série.");
    }

    float avg = 0;
    float max = __FLT_MIN__;
    float min = __FLT_MAX__;
    for (auto const& val : array) {
        if (val > max) {
            max = val;
        }
        if (val < min) {
            min = val;
        }
        avg += val;
    }
    avg -= max;
    avg -= min;
    avg /= array.size() - 2;

    return avg;
}

string MeasureModule::get_errors()
{
    string res = "{\"success\": false, \"errors\": [";
    for (auto const& e : error_array) {
        res += "{\"occuredDate\": \"" + e.occuredDate + "\", \"message\": \"" + e.message + "\"},";
    }

    res.pop_back();
    res += "]}\0";
    return res;
}



MeasureModule::MeasureModule()
{
    this->stopped = true;
    reset();

    thread t(&MeasureModule::STC31_measure_clock, this);
    t.detach();

    thread t2(&MeasureModule::BME680_measure_clock, this);
    t2.detach();

    thread t3(&MeasureModule::STC31_calibration_clock, this);
    t3.detach();
}

void MeasureModule::reset()
{
    this->altitude = 0;
    this->STC31_calibrating = false;
    this->error_array.clear();
    this->temperature_array.clear();
    this->humidity_array.clear();
    this->luminosity_array.clear();
    this->CO2_array.clear();
    this->O2_array.clear();
    this->pressure_array.clear();

    int16_t error = 0;

    /* STC31 init */
    sensirion_i2c_hal_free();
    error = sensirion_i2c_hal_init();
    if (error) {
        error_array.push_front(DriverError("Impossible d'initialiser la communication avec le capteur STC31. La fonction [sensirion_i2c_hal_init] a retourné le code d'erreur : " + to_string(error)));
        return;
    }

    uint16_t self_test_output;
    error = stc3x_self_test(&self_test_output);
    if (error) {
        error_array.push_front(DriverError("L'auto-test du capteur STC31 a échoué. La fonction [stc3x_self_test] a retourné le code d'erreur : " + to_string(error)));
        return;
    }

    error = stc3x_set_binary_gas(0x0001);
    if (error) {
        error_array.push_front(DriverError("La défénition du mode de relève du CO2 a échoué. La fonction [stc3x_set_binary_gas] a retourné le code d'erreur : " + to_string(error)));
        return;
    }

    /* BME680 init */
    i2c_hal_free();
    error = i2c_hal_init();
    if (error) {
        error_array.push_front(DriverError("Impossible d'initialiser la communication avec le capteur BME680. La fonction [i2c_hal_init] a retourné le code d'erreur : " + to_string(error)));
        return;
    }

    error = bme680_self_test();
    if (error) {
        error_array.push_front(DriverError("L'auto-test du capteur BME680 a échoué. La fonction [bme680_self_test] a retourné le code d'erreur : " + to_string(error)));
        return;
    }

    this->stopped = false;
}

SensorMeasure* MeasureModule::get()
{
    if (stopped) {
        return nullptr;
    }

    float temperature = __FLT_MIN__;
    try {
        temperature = getAverage(temperature_array);
    } catch (const DriverError& e) {
        string err_msg = e.message + " Série concernée : température.";
        error_array.push_front(DriverError(err_msg));
    }

    float humidity = __FLT_MIN__;
    try {
        humidity = getAverage(humidity_array);
    } catch (const DriverError& e) {
        string err_msg = e.message + " Série concernée : humidité.";
        error_array.push_front(DriverError(err_msg));
    }

    float pressure = __FLT_MIN__;
    try {
        pressure = getAverage(pressure_array);

        // convert to pressure at altitude to pressure at sea level
        pressure = pressureAtSeaLevel(temperature, pressure, this->altitude);
    } catch (const DriverError& e) {
        string err_msg = e.message + " Série concernée : pression.";
        error_array.push_front(DriverError(err_msg));
    }

    float CO2 = __FLT_MIN__;
    try {
        CO2 = getAverage(CO2_array);
    } catch (const DriverError& e) {
        string err_msg = e.message + " Série concernée : CO2.";
        error_array.push_front(DriverError(err_msg));
    }

    float O2 = __FLT_MIN__;
    try {
        O2 = getAverage(O2_array);
    } catch (const DriverError& e) {
        string err_msg = e.message + " Série concernée : O2.";
        error_array.push_front(DriverError(err_msg));
    }

    float luminosity = __FLT_MIN__;
    try {
        luminosity = getAverage(luminosity_array);
    } catch (const DriverError& e) {
        string err_msg = e.message + " Série concernée : luminosité.";
        error_array.push_front(DriverError(err_msg));
    }

    return new SensorMeasure(temperature, humidity, pressure, CO2, O2, luminosity);
}

void MeasureModule::setAltitude(int altitude)
{
    this->altitude = altitude;
}
