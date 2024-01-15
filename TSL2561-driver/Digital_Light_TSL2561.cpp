#include "Digital_Light_TSL2561.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
using namespace std;

void TSL2561_CalculateLux::closeRegister() {
    if (i2c_device >= 0) {
        close(i2c_device);
        i2c_device = -1;
    }
}

uint8_t TSL2561_CalculateLux::readRegister(int deviceAddress, int address, uint8_t *data) {
    if (ioctl(i2c_device, I2C_SLAVE, deviceAddress) < 0) {
        perror("Error setting I2C address");
        return -1;
    }
    if (write(i2c_device, &address, 32) != 32) {
        perror("Error writing to I2C device");
        return -1;
    }
    usleep(14000);
    if (read(i2c_device, &data, 32) != 32) {
        perror("Error reading from I2C device");
        return -1;
    }
    return 0;
}

uint8_t TSL2561_CalculateLux::writeRegister(int deviceAddress, int address, uint8_t val) {
    if (ioctl(i2c_device, I2C_SLAVE, deviceAddress) < 0) {
        perror("Error setting I2C address");
        return -1;
    }
    uint8_t buffer[2] = {static_cast<uint8_t>(address), val};
    if (write(i2c_device, buffer, 32) != 32) {
        perror("Error writing to I2C device");
        return -1;
    }

    return 0;
}

void TSL2561_CalculateLux::getLux(void) {
    readRegister(TSL2561_Address, TSL2561_Channal0L, &CH0_LOW);
    readRegister(TSL2561_Address, TSL2561_Channal0H, &CH0_HIGH);
    //read two bytes from registers 0x0E and 0x0F
    readRegister(TSL2561_Address, TSL2561_Channal1L, &CH1_LOW);
    readRegister(TSL2561_Address, TSL2561_Channal1H, &CH1_HIGH);

    ch0 = (CH0_HIGH << 8) | CH0_LOW;
    ch1 = (CH1_HIGH << 8) | CH1_LOW;
}
uint8_t TSL2561_CalculateLux::init() {
    uint8_t error = 0;
    i2c_device = open(I2C_DEVICE_PATH, O_RDWR);
    if (i2c_device < 0) {
        perror("Error opening I2C device file");
        return -1;
    }
    error = writeRegister(TSL2561_Address, TSL2561_Control, 0x03); // POWER UP
    if (error) {
        perror("Error write register");
        return -1;
    }

    error = writeRegister(TSL2561_Address, TSL2561_Timing, 0x00); //No High Gain (1x), integration time of 13ms
    if (error) {
        perror("Error write register");
        return -1;
    }

    error = writeRegister(TSL2561_Address, TSL2561_Interrupt, 0x00);
    if (error) {
        perror("Error write register");
        return -1;
    }

    error = writeRegister(TSL2561_Address, TSL2561_Control, 0x00); // POWER Down
    if (error) {
        perror("Error write register");
        return -1;
    }

    return 0;
}


uint16_t TSL2561_CalculateLux::readIRLuminosity() { // read Infrared channel value only, not convert to lux.
    writeRegister(TSL2561_Address, TSL2561_Control, 0x03); // POWER UP
    usleep(14000);
    getLux();

    writeRegister(TSL2561_Address, TSL2561_Control, 0x00); // POWER Down
    if (ch1 == 0) {
        return 0;
    }
    if ((ch0 / ch1) < 2 && ch0 > 4900) {
        return -1;  //ch0 out of range, but ch1 not. the lux is not valid in this situation.
    }
    return ch1;
}

uint16_t TSL2561_CalculateLux::readFSpecLuminosity() { //read Full Spectrum channel value only,  not convert to lux.
    writeRegister(TSL2561_Address, TSL2561_Control, 0x03); // POWER UP
    usleep(14000);
    getLux();

    writeRegister(TSL2561_Address, TSL2561_Control, 0x00); // POWER Down
    if (ch1 == 0) {
        return 0;
    }
    if ((ch0 / ch1) < 2 && ch0 > 4900) {
        return -1;  //ch0 out of range, but ch1 not. the lux is not valid in this situation.
    }
    return ch0;
}

TSL2561_CalculateLux::TSL2561_CalculateLux()
{
    this->i2c_device = -1;
}

signed long TSL2561_CalculateLux::readVisibleLux() {
    writeRegister(TSL2561_Address, TSL2561_Control, 0x03); // POWER UP
    usleep(14000);
    getLux();

    writeRegister(TSL2561_Address, TSL2561_Control, 0x00); // POWER Down
    if (ch1 == 0) {
        return 0;
    }
    if ((ch0 / ch1) < 2 && ch0 > 4900) {
        return -1;  //ch0 out of range, but ch1 not. the lux is not valid in this situation.
    }
    return calculateLux(0, 0, 0);  //T package, no gain, 13ms
}
unsigned long TSL2561_CalculateLux::calculateLux(unsigned int iGain, unsigned int tInt, int iType) {
    switch (tInt) {
        case 0:  // 13.7 msec
            chScale = CHSCALE_TINT0;
            break;
        case 1: // 101 msec
            chScale = CHSCALE_TINT1;
            break;
        default: // assume no scaling
            chScale = (1 << CH_SCALE);
            break;
    }
    if (!iGain) {
        chScale = chScale << 4;    // scale 1X to 16X
    }
    // scale the channel values
    channel0 = (ch0 * chScale) >> CH_SCALE;
    channel1 = (ch1 * chScale) >> CH_SCALE;

    ratio1 = 0;
    if (channel0 != 0) {
        ratio1 = (channel1 << (RATIO_SCALE + 1)) / channel0;
    }
    // round the ratio value
    unsigned long ratio = (ratio1 + 1) >> 1;

    switch (iType) {
        case 0: // T package
            if ((ratio >= 0) && (ratio <= K1T)) {
                b = B1T;
                m = M1T;
            } else if (ratio <= K2T) {
                b = B2T;
                m = M2T;
            } else if (ratio <= K3T) {
                b = B3T;
                m = M3T;
            } else if (ratio <= K4T) {
                b = B4T;
                m = M4T;
            } else if (ratio <= K5T) {
                b = B5T;
                m = M5T;
            } else if (ratio <= K6T) {
                b = B6T;
                m = M6T;
            } else if (ratio <= K7T) {
                b = B7T;
                m = M7T;
            } else if (ratio > K8T) {
                b = B8T;
                m = M8T;
            }
            break;
        case 1:// CS package
            if ((ratio >= 0) && (ratio <= K1C)) {
                b = B1C;
                m = M1C;
            } else if (ratio <= K2C) {
                b = B2C;
                m = M2C;
            } else if (ratio <= K3C) {
                b = B3C;
                m = M3C;
            } else if (ratio <= K4C) {
                b = B4C;
                m = M4C;
            } else if (ratio <= K5C) {
                b = B5C;
                m = M5C;
            } else if (ratio <= K6C) {
                b = B6C;
                m = M6C;
            } else if (ratio <= K7C) {
                b = B7C;
                m = M7C;
            }
    }
    temp = ((channel0 * b) - (channel1 * m));
    if (temp < 0) {
        temp = 0;
    }
    temp += (1 << (LUX_SCALE - 1));
    // strip off fractional portion
    lux = temp >> LUX_SCALE;
    return (lux);
}
TSL2561_CalculateLux TSL2561;


