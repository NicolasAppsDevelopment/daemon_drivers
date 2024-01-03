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
        try {
            int16_t error = 0;

            float temperature;
            float humidity;
            float pressure;

            error = bme680_get_measure(&temperature, &pressure, &humidity);
            if (error) {
                error_array.push_front(DriverError("Impossible de récupérer les données de mesure du capteurs BME680. La fonction [bme680_get_measure] a retourné le code d'erreur : " + to_string(error)));
                return;
            }

            addHumiditySample(humidity);
            addPressureSample(pressure);
            addTemperatureSample(temperature);

            sleep(1);
        } catch (...) {
            error_array.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur BME680."));
            return;
        }
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

void MeasureModule::calibrate_STC31_sensor(int pressure, int humidity)
{
    int16_t error = 0;

    uint16_t hum = humidity * 65535 / 100;
    error = stc3x_set_relative_humidity(hum);
    if (error) {
        error_array.push_front(DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_relative_humidity] a retourné le code d'erreur : " + to_string(error)));
        return;
    }

    uint16_t pres = pressure;
    error = stc3x_set_pressure(pres);
    if (error) {
        error_array.push_front(DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_pressure] a retourné le code d'erreur : " + to_string(error)));
        return;
    }
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

void MeasureModule::STC31_measure_clock()
{
    while (true) {
        try {
            int16_t error = 0;

            uint16_t gas_ticks;
            uint16_t temperature_ticks;

            float gas;
            float temperature;

            error = stc3x_measure_gas_concentration(&gas_ticks, &temperature_ticks);
            if (error) {
                error_array.push_front(DriverError("Impossible de récupérer les données de mesure du capteurs STC31. La fonction [stc3x_measure_gas_concentration] a retourné le code d'erreur : " + to_string(error)));
                return;
            }

            gas = 100 * ((float)gas_ticks - 16384.0) / 32768.0;
            temperature = (float)temperature_ticks / 200.0;

            addCO2Sample(gas);
            addTemperatureSample(temperature);

            sleep(1);
        } catch (...) {
            error_array.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur STC31."));
            return;
        }
    }
}

MeasureModule::MeasureModule()
{
    reset();
}

void MeasureModule::reset()
{
    this->error_array.clear();
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

    thread t(&MeasureModule::STC31_measure_clock, this);
    t.detach();

    thread t2(&MeasureModule::BME680_measure_clock, this);
    t2.detach();
}

SensorMeasure* MeasureModule::get(int altitude)
{
    // remove higher & lower value, then process the average for temperature
    if (temperature_array.size() < NB_OF_SAMPLE) {
        error_array.push_front(DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise de la température."));
        return nullptr;
    }

    float temperature = 0;
    float temperature_max = __FLT_MIN__;
    float temperature_min = __FLT_MAX__;
    for (auto const& t : temperature_array) {
        if (t > temperature_max) {
            temperature_max = t;
        }
        if (t < temperature_min) {
            temperature_min = t;
        }
        temperature += t;
    }
    temperature -= temperature_max;
    temperature -= temperature_min;
    temperature /= temperature_array.size() - 2;

    // remove higher & lower value, then process the average for humidity
    if (humidity_array.size() < NB_OF_SAMPLE) {
        error_array.push_front(DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise de l'humidité."));
        return nullptr;
    }

    float humidity = 0;
    float humidity_max = __FLT_MIN__;
    float humidity_min = __FLT_MAX__;
    for (auto const& h : humidity_array) {
        if (h > humidity_max) {
            humidity_max = h;
        }
        if (h < humidity_min) {
            humidity_min = h;
        }
        humidity += h;
    }
    humidity -= humidity_max;
    humidity -= humidity_min;
    humidity /= humidity_array.size() - 2;

    // remove higher & lower value, then process the average for pressure
    if (pressure_array.size() < NB_OF_SAMPLE) {
        error_array.push_front(DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise de la pression."));
        return nullptr;
    }

    float pressure = 0;
    float pressure_max = __FLT_MIN__;
    float pressure_min = __FLT_MAX__;
    for (auto const& p : pressure_array) {
        if (p > pressure_max) {
            pressure_max = p;
        }
        if (p < pressure_min) {
            pressure_min = p;
        }
        pressure += p;
    }
    pressure -= pressure_max;
    pressure -= pressure_min;
    pressure /= pressure_array.size() - 2;

    // convert to pressure at altitude to pressure at sea level
    pressure = exp((-9.80665 * 0.0289644 * altitude) / (8.31432 * (273.15 + temperature))) / pressure;

    // remove higher & lower value, then process the average for CO2
    if (CO2_array.size() < NB_OF_SAMPLE) {
        error_array.push_front(DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise du CO2."));
        return nullptr;
    }

    float CO2 = 0;
    float CO2_max = __FLT_MIN__;
    float CO2_min = __FLT_MAX__;
    for (auto const& co2 : CO2_array) {
        if (co2 > CO2_max) {
            CO2_max = co2;
        }
        if (co2 < CO2_min) {
            CO2_min = co2;
        }
        CO2 += co2;
    }
    CO2 -= CO2_max;
    CO2 -= CO2_min;
    CO2 /= CO2_array.size() - 2;

    // remove higher & lower value, then process the average for O2
    if (O2_array.size() < NB_OF_SAMPLE) {
        error_array.push_front(DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise du O2."));
        return nullptr;
    }

    float O2 = 0;
    float O2_max = __FLT_MIN__;
    float O2_min = __FLT_MAX__;
    for (auto const& o2 : O2_array) {
        if (o2 > O2_max) {
            O2_max = o2;
        }
        if (o2 < O2_min) {
            O2_min = o2;
        }
        O2 += o2;
    }
    O2 -= O2_max;
    O2 -= O2_min;
    O2 /= O2_array.size() - 2;

    // remove higher & lower value, then process the average for luminosity
    if (luminosity_array.size() < NB_OF_SAMPLE) {
        error_array.push_front(DriverError("Le module de relève n'a pas encore assez de données pour réaliser une moyenne précise de la luminosité."));
        return nullptr;
    }

    float luminosity = 0;
    float luminosity_max = __FLT_MIN__;
    float luminosity_min = __FLT_MAX__;
    for (auto const& l : luminosity_array) {
        if (l > luminosity_max) {
            luminosity_max = l;
        }
        if (l < luminosity_min) {
            luminosity_min = l;
        }
        luminosity += l;
    }
    luminosity -= luminosity_max;
    luminosity -= luminosity_min;
    luminosity /= luminosity_array.size() - 2;

    return new SensorMeasure(temperature, humidity, pressure, CO2, O2, luminosity);
}
