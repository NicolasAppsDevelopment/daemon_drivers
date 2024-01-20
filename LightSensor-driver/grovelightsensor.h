#ifndef GROVELIGHTSENSORDRIVER_H
#define GROVELIGHTSENSORDRIVER_H

#include "Sensirion-driver-base/sensirion_config.h"
#include "Sensirion-driver-base/sensirion_driver.h"

#define GROVE_BASE_HAT_I2C_ADDRESS 0x04
#define A2_PIN 2
#define GET_OUTPUT_VOLTAGE_CMD 0x30
#define A2_OUTPUT_VOLTAGE_CMD (GET_OUTPUT_VOLTAGE_CMD + A2_PIN)

class GroveLightSensorDriver : public SensirionDriver
{
public:
    GroveLightSensorDriver();
    int16_t getLuminosity(int16_t *luminosity);
};

#endif // GROVELIGHTSENSORDRIVER_H
