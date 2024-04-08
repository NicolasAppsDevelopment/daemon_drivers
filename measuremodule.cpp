#include "measuremodule.h"
#include "STC31-driver/stc31.h"
#include "BME680-driver/common.h"
#include "SHTC3-driver/shtc3.h"
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <cfloat>

using namespace std;

void MeasureModule::bme680MeasureClock()
{
    while (true) {
        if (!stopped) {
            try {
                int16_t error = 0;

                float pressure;

                error = BME68XCommon::bme680_get_measure(&pressure);
                if (error) {
                    if (error != 2) { // DO NOT THROW ERROR IF IT'S A NO NEW DATA ERROR
                        throw DriverError("Impossible de récupérer les données de mesure du capteur BME680. La fonction [bme680_get_measure] a retourné le code d'erreur : " + to_string(error));
                    }
                } else {
                    addPressureSample(pressure);
                }
            } catch (const DriverError& e) {
                errorArray.push_front(e);
                this->stopped = true;
            } catch (...) {
                errorArray.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur BME680."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::lightSensorMeasureClock()
{
    while (true) {
        if (!stopped) {
            try {
                int16_t error = 0;

                int16_t lum;
                float luminosity;

                error = lightSensorDriver.getLuminosity(&lum);
                if (error) {
                    throw DriverError("Impossible de récupérer les données de mesure du capteur de lumière.");
                }

                luminosity = ((float)lum / 716.0) * 100.0;

                addLuminositySample(luminosity);

            } catch (const DriverError& e) {
                errorArray.push_front(e);
                this->stopped = true;
            } catch (...) {
                errorArray.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur de lumière."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::fiboxMeasureClock()
{
    while (true) {
        if (!stopped) {
            try {
                FiboxAnswer* data = fiboxDriver.getMeasure();

                if (data->isTemperatureEnabled) {
                    addTemperatureSample((float)data->temperature);
                }
                addPressureSample((float)data->pressure);

                float avgTemperature = 0.0f;
                try
                {
                    avgTemperature = getAverage(temperatureArray);
                }
                catch (const DriverError&)
                {
                    avgTemperature = (float)data->temperature;
                }
                oxyCalculator->setTemperature(avgTemperature);

                float avgPressure = 0.0f;
                try
                {
					avgPressure = getAverage(pressureArray);
				}
                catch (const DriverError&)
                {
					avgPressure = (float)data->pressure;
				}
                oxyCalculator->setPressure(avgPressure);

                oxyCalculator->setPhaseAngle((float)data->phase);
                
                float o2 = (float)oxyCalculator->getOxygenValue();
                if (isnanf(o2) || isinff(o2)) {
					throw DriverError("La valeur d'oxygène calculé n'était pas un nombre. Vérifier vos valeurs de calibration.");
                }
                addO2Sample(o2);
            } catch (const DriverError& e) {
                errorArray.push_front(e);
                this->stopped = true;
            } catch (...) {
                errorArray.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur Fibox."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::shtc3MeasureClock()
{
    while (true) {
        if (!stopped) {
            try {
                int8_t error = 0;

                int32_t temp, humid;

                float humidity;
                float temperature;

                //this->SHTC3_driver_mutex.lock();
                error = shtc3Driver.shtc1_measure_blocking_read(&temp, &humid);
                //this->SHTC3_driver_mutex.unlock();

                if (error) {
                    throw DriverError("Impossible de récupérer les données de mesure du capteur SHTC3. La fonction [shtc1_measure_blocking_read] a retourné le code d'erreur : " + to_string(error));
                }

                humidity = (float)humid / 1000.0f;
                temperature = (float)temp / 1000.0f;

                addHumiditySample(humidity);
                addTemperatureSample(temperature);
            } catch (const DriverError& e) {
                errorArray.push_front(e);
                this->stopped = true;
            } catch (...) {
                errorArray.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur SHTC3."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::stc31MeasureClock()
{
    while (true) {
        if (!stopped) {
            try {
                int16_t error = 0;

                UShort gas_ticks;
                int16_t temperature_ticks;

                float gas;
                float temperature;

                this->stc31DriverMutex.lock();
                error = stc31Driver.stc3x_measure_gas_concentration(&gas_ticks, &temperature_ticks);
                this->stc31DriverMutex.unlock();

                if (error) {
                    throw DriverError("Impossible de récupérer les données de mesure du capteur STC31. La fonction [stc3x_measure_gas_concentration] a retourné le code d'erreur : " + to_string(error));
                }

                gas = 100.0f * ((float)gas_ticks - 16384.0f) / 32768.0f;
                temperature = (float)temperature_ticks / 200.0f;

                addCo2Sample(gas);
                addTemperatureSample(temperature);
            } catch (const DriverError& e) {
                errorArray.push_front(e);
                this->stopped = true;
            } catch (...) {
                errorArray.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de mesure du capteur STC31."));
                this->stopped = true;
            }
        }

        sleep(1);
    }
}

void MeasureModule::stc31CalibrationClock()
{
    while (true) {
        if (!stopped) {
            try {
                float temperature = __FLT_MIN__;
                try {
                    temperature = getAverage(temperatureArray);
                } catch (const DriverError& e) {}

                float humidity = __FLT_MIN__;
                try {
                    humidity = getAverage(humidityArray);
                } catch (const DriverError& e) {}

                float pressure = __FLT_MIN__;
                try {
                    pressure = getAverage(pressureArray);

                    if (temperature != __FLT_MIN__ && pressure != __FLT_MIN__) {
                        // convert to pressure at altitude to pressure at sea level
                        pressure = pressureAtSeaLevel(temperature, pressure, this->config->altitude);
                    }
                } catch (const DriverError& e) {}

                if (temperature != __FLT_MIN__ && pressure != __FLT_MIN__ && humidity != __FLT_MIN__) {
                    int16_t error = 0;

                    UShort hum = humidity * 65535 / 100;

                    this->stc31DriverMutex.lock();
                    error = stc31Driver.stc3x_set_relative_humidity(hum);
                    this->stc31DriverMutex.unlock();

                    if (error) {
                        throw DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_relative_humidity] a retourné le code d'erreur : " + to_string(error));
                    }

                    UShort pres = pressure / 100; // Pa to mbar (hPa)

                    this->stc31DriverMutex.lock();
                    error = stc31Driver.stc3x_set_pressure(pres);
                    this->stc31DriverMutex.unlock();

                    if (error) {
                        throw DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_pressure] a retourné le code d'erreur : " + to_string(error));
                    }

                    int16_t temp = temperature * 200;

                    this->stc31DriverMutex.lock();
                    error = stc31Driver.stc3x_set_temperature(temp);
                    this->stc31DriverMutex.unlock();

                    if (error) {
                        throw DriverError("Impossible de calibrer le capteur STC31. La fonction [stc3x_set_temperature] a retourné le code d'erreur : " + to_string(error));
                    }
                }
            } catch (const DriverError& e) {
                errorArray.push_front(e);
                this->stopped = true;
            } catch (...) {
                errorArray.push_front(DriverError("Une errreur inconnue est survenu dans la boucle de calibration du capteur STC31."));
                this->stopped = true;
            }
        }

        sleep(5);
    }
}

void MeasureModule::addTemperatureSample(float temperature)
{
    temperatureArray.push_front(temperature);
    if (temperatureArray.size() > NB_OF_SAMPLE * NB_TEMPERATURE_SENSOR) {
        temperatureArray.pop_back();
    }
}

void MeasureModule::addPressureSample(float pressure)
{
    pressureArray.push_front(pressure);
    if (pressureArray.size() > NB_OF_SAMPLE * NB_PRESSURE_SENSOR) {
        pressureArray.pop_back();
    }
}

void MeasureModule::addHumiditySample(float humidity)
{
    humidityArray.push_front(humidity);
    if (humidityArray.size() > NB_OF_SAMPLE * NB_HUMIDITY_SENSOR) {
        humidityArray.pop_back();
    }
}

void MeasureModule::addCo2Sample(float co2)
{
    co2Array.push_front(co2);
    if (co2Array.size() > NB_OF_SAMPLE * NB_CO2_SENSOR) {
        co2Array.pop_back();
    }
}

void MeasureModule::addO2Sample(float o2)
{
    o2Array.push_front(o2);
    if (o2Array.size() > NB_OF_SAMPLE * NB_O2_SENSOR) {
        o2Array.pop_back();
    }
}

void MeasureModule::addLuminositySample(float luminosity)
{
    luminosityArray.push_front(luminosity);
    if (luminosityArray.size() > NB_OF_SAMPLE * NB_LUMINOSITY_SENSOR) {
        luminosityArray.pop_back();
    }
}

float MeasureModule::pressureAtSeaLevel(float temperature, float pressure, float altitude)
{
    // Constants
    float gravitational_acceleration = 9.80665f;  // Standard gravitational acceleration (m/s^2)

    // Calculate temperature at the given altitude
    float temperature_at_altitude = 273.15f + temperature;

    // Calculate pressure at sea level using the barometric formula
    return pressure * exp((gravitational_acceleration * altitude) / (287.058f * temperature_at_altitude));
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

list<DriverError> MeasureModule::getErrors()
{
    return errorArray;
}

bool MeasureModule::isInitialising() const
{
    return this->initialising;
}

MeasureModule::MeasureModule()
{
    this->stc31Driver = STC31Driver();
    this->shtc3Driver = SHTC3Driver();
    this->lightSensorDriver = GroveLightSensorDriver();
    this->fiboxDriver = FiboxDriver();

    // init default config
    this->config = new MeasureConfig();
    oxyCalculator = new OxygenCalculation(config);

    reset();

    thread t(&MeasureModule::stc31MeasureClock, this);
    t.detach();

    thread t2(&MeasureModule::shtc3MeasureClock, this);
    t2.detach();

    thread t3(&MeasureModule::bme680MeasureClock, this);
    t3.detach();

    thread t4(&MeasureModule::lightSensorMeasureClock, this);
    t4.detach();

    thread t5(&MeasureModule::fiboxMeasureClock, this);
    t5.detach();

    thread t6(&MeasureModule::stc31CalibrationClock, this);
    t6.detach();
}

void MeasureModule::reset()
{
    thread t(&MeasureModule::processReset, this);
    t.detach();
}

void MeasureModule::processReset()
{
    this->stopped = true;
    this->initialising = true;
    this->errorArray.clear();
    this->temperatureArray.clear();
    this->humidityArray.clear();
    this->luminosityArray.clear();
    this->co2Array.clear();
    this->o2Array.clear();
    this->pressureArray.clear();

    int16_t error = 0;

    /* STC31 init */
    stc31Driver.sensirion_i2c_hal_free();
    error = stc31Driver.sensirion_i2c_hal_init();
    if (error) {
        errorArray.push_front(DriverError("Impossible d'initialiser la communication avec le capteur STC31. La fonction [sensirion_i2c_hal_init] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    UShort self_test_output;
    this->stc31DriverMutex.lock();
    error = stc31Driver.stc3x_self_test(&self_test_output);
    this->stc31DriverMutex.unlock();
    if (error) {
        errorArray.push_front(DriverError("L'auto-test du capteur STC31 a échoué. La fonction [stc3x_self_test] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    this->stc31DriverMutex.lock();
    error = stc31Driver.stc3x_set_binary_gas(0x0001);
    this->stc31DriverMutex.unlock();
    if (error) {
        errorArray.push_front(DriverError("La défénition du mode de relève du co2 a échoué. La fonction [stc3x_set_binary_gas] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    /* SHTC3 init */
    shtc3Driver.sensirion_i2c_hal_free();
    error = shtc3Driver.sensirion_i2c_hal_init();
    if (error) {
        errorArray.push_front(DriverError("Impossible d'initialiser la communication avec le capteur SHTC3. La fonction [sensirion_i2c_hal_init] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    int timeout = 0;
    while (shtc3Driver.shtc1_probe() != STATUS_OK && timeout <= 30) {
        printf("SHTC3 sensor probing failed: %d\n", shtc3Driver.shtc1_probe());
        timeout++;
        sleep(1);
    }
    if (timeout > 30) {
		errorArray.push_front(DriverError("Impossible de communiquer avec le capteur SHTC3. La fonction [shtc1_probe] n'a pas retourné de réponse positive dans le temps imparti."));
		this->initialising = false;
		return;
	}

    /* BME680 init */
    BME68XCommon::i2c_hal_free();
    error = BME68XCommon::i2c_hal_init();
    if (error) {
        errorArray.push_front(DriverError("Impossible d'initialiser la communication avec le capteur BME680. La fonction [i2c_hal_init] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    error = BME68XCommon::bme680_self_test();
    if (error) {
        errorArray.push_front(DriverError("Erreur ignorée. L'auto-test du capteur BME680 a échoué. La fonction [bme680_self_test] a retourné le code d'erreur : " + to_string(error)));
        // IGNORE ERROR: SELF TEST CAN CAUSE ERROR BUT VALUES ARE OK (JUST FOR PRESSURE)
        /*this->initialising = false;
        return;*/
    }

    /* Grove Light Sensor v1.2 ADC init */
    lightSensorDriver.sensirion_i2c_hal_free();
    error = lightSensorDriver.sensirion_i2c_hal_init();
    if (error) {
        errorArray.push_front(DriverError("Impossible d'initialiser la communication avec le capteur de lumière. La fonction [sensirion_i2c_hal_init] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    error = lightSensorDriver.initAddress();
    if (error) {
        errorArray.push_front(DriverError("Impossible d'initialiser la communication avec le capteur de lumière. La fonction [initAddress] a retourné le code d'erreur : " + to_string(error)));
        this->initialising = false;
        return;
    }

    /* Fibox init */
    try
    {
        fiboxDriver.initFiboxCommunication();
    }
    catch (const DriverError e)
    {
        errorArray.push_front(e);
        this->initialising = false;
        return;
    }

    this->stopped = false;
    this->initialising = false;
}

SensorMeasure* MeasureModule::get()
{
    if (stopped || initialising) {
        return nullptr;
    }

    float temperature = __FLT_MIN__;
    try {
        temperature = getAverage(temperatureArray);
    } catch (const DriverError& e) {
        String err_msg = e.message + " Série concernée : température.";
        errorArray.push_front(DriverError(err_msg));
    }

    float humidity = __FLT_MIN__;
    try {
        humidity = getAverage(humidityArray);
    } catch (const DriverError& e) {
        String err_msg = e.message + " Série concernée : humidité.";
        errorArray.push_front(DriverError(err_msg));
    }

    float pressure = __FLT_MIN__;
    try {
        pressure = getAverage(pressureArray);

        // convert to pressure at altitude to pressure at sea level
        pressure = pressureAtSeaLevel(temperature, pressure, this->config->altitude);
    } catch (const DriverError& e) {
        String err_msg = e.message + " Série concernée : pression.";
        errorArray.push_front(DriverError(err_msg));
    }

    float co2 = __FLT_MIN__;
    try {
        co2 = getAverage(co2Array);
    } catch (const DriverError& e) {
        String err_msg = e.message + " Série concernée : CO2.";
        errorArray.push_front(DriverError(err_msg));
    }

    float o2 = __FLT_MIN__;
    try {
        o2 = getAverage(o2Array);
    } catch (const DriverError& e) {
        String err_msg = e.message + " Série concernée : o2.";
        errorArray.push_front(DriverError(err_msg));
    }

    float luminosity = __FLT_MIN__;
    try {
        luminosity = getAverage(luminosityArray);
    } catch (const DriverError& e) {
        String err_msg = e.message + " Série concernée : luminosité.";
        errorArray.push_front(DriverError(err_msg));
    }

    return new SensorMeasure(temperature, humidity, pressure, co2, o2, luminosity);
}

void MeasureModule::setConfig(int altitude, double F1, double M, double DPHI1, double DPHI2, double DKSV1, double DKSV2, double pressure, double cal0, double cal2nd, double t0, double t2nd, double o2Cal2nd, bool calibIsHumid, bool humidMode, bool enableTempFibox)
{
    this->config->set(altitude, F1, M, DPHI1, DPHI2, DKSV1, DKSV2, pressure, cal0, cal2nd, t0, t2nd, o2Cal2nd, calibIsHumid, enableTempFibox, humidMode);
    this->fiboxDriver.setEnableTempFibox(config->enableTempFibox);

    // clear parameters dependant data
    co2Array.clear();
    o2Array.clear();
}
