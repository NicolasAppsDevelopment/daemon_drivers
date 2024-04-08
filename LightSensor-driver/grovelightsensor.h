#ifndef GROVELIGHTSENSORDRIVER_H
#define GROVELIGHTSENSORDRIVER_H

#include "../Sensirion-driver-base/sensirion_config.h"
#include "../Sensirion-driver-base/sensirion_driver.h"
#include "../types.h"

#define GROVE_BASE_HAT_I2C_ADDRESS 0x04
#define A2_PIN 2
#define GET_OUTPUT_VOLTAGE_CMD 0x30
#define A2_OUTPUT_VOLTAGE_CMD (GET_OUTPUT_VOLTAGE_CMD + A2_PIN)

/**
 * GroveLightSensorDriver - Grove Light Sensor driver class
 * It implements the SensirionDriver interface to access init/free I2C device functions
 * @see https://github.com/Seeed-Studio/grove.py/blob/master/grove/grove_light_sensor_v1_2.py
 */
class GroveLightSensorDriver : public SensirionDriver
{
public:
    GroveLightSensorDriver();

    /**
     * Get the luminosity value from the light sensor
     *
     * @param luminosity Pointer to the luminosity value
     * @return           0 on success, error code otherwise
     */
    int16_t getLuminosity(int16_t *luminosity);

    /**
     * Initialize the I2C address of the light sensor
     *
     * @return 0 on success, error code otherwise
     */
    int16_t initAddress();
};

#endif // GROVELIGHTSENSORDRIVER_H
