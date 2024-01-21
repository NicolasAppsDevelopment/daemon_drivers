#include "grovelightsensor.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

GroveLightSensorDriver::GroveLightSensorDriver() : SensirionDriver()
{}

int16_t GroveLightSensorDriver::getLuminosity(int16_t* luminosity)
{
    // Specify the register to read by command address
    uint8_t cmd_buffer[1];
    cmd_buffer[0] = A2_OUTPUT_VOLTAGE_CMD;

    // Write the register address to initiate the read
    if (write(i2c_device, cmd_buffer, sizeof(cmd_buffer[0])) != sizeof(cmd_buffer[0])) {
        perror("Unable to set register address for light sensor");
        close(i2c_device);
        return -1;
    }

    // Read 2 bytes (word) from the specified register
    uint16_t buffer[1];
    if (read(i2c_device, buffer, sizeof(buffer[0])) != sizeof(buffer[0])) {
        perror("Unable to read from I2C device for light sensor");
        close(i2c_device);
        return -2;
    }
    *luminosity = buffer[0];

    return 0;
}

int16_t GroveLightSensorDriver::init_address()
{
    // Set the I2C slave address
    if (ioctl(i2c_device, I2C_SLAVE, GROVE_BASE_HAT_I2C_ADDRESS) < 0) {
        perror("Unable to set I2C slave address for light sensor");
        return -1;
    }
    return 0;
}

