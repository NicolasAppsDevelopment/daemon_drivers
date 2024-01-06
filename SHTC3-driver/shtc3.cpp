/**
 * \file
 *
 * \brief Sensirion SHTC1 (and compatible) driver implementation
 *
 * This module provides access to the SHTC1 functionality over a generic I2C
 * interface. It supports measurements without clock stretching only.
 *
 * SHTC1 compatible sensors: SHTW1, SHTW2, SHTC3
 */

#include "shtc3.h"
#include "Sensirion-driver-base/sensirion_common.h"
#include <iostream>

using namespace std;

int16_t SHTC3Driver::shtc1_sleep(void) {
    return sensirion_i2c_write_cmd(SHTC1_ADDRESS, SHTC3_CMD_SLEEP);
}

int16_t SHTC3Driver::shtc1_wake_up(void) {
    return sensirion_i2c_write_cmd(SHTC1_ADDRESS, SHTC3_CMD_WAKEUP);
}

int16_t SHTC3Driver::shtc1_measure_blocking_read(int32_t* temperature, int32_t* humidity) {
    int16_t ret;

    ret = shtc1_measure();
    if (ret) {
        printf("shtc1_measure return error: %d", ret);
        return ret;
    }
#if !defined(USE_SENSIRION_CLOCK_STRETCHING) || !USE_SENSIRION_CLOCK_STRETCHING
    sensirion_i2c_hal_sleep_usec(SHTC1_MEASUREMENT_DURATION_USEC);
#endif /* USE_SENSIRION_CLOCK_STRETCHING */
    return shtc1_read(temperature, humidity);
}

int16_t SHTC3Driver::shtc1_measure(void) {
    return sensirion_i2c_write_cmd(SHTC1_ADDRESS, this->shtc1_cmd_measure);
}

int16_t SHTC3Driver::shtc1_read(int32_t* temperature, int32_t* humidity) {
    uint16_t words[2];
    printf("sensirion_i2c_read_words num_words: %d", SENSIRION_NUM_WORDS(words));
    int16_t ret = sensirion_i2c_read_words(SHTC1_ADDRESS, words,
                                           SENSIRION_NUM_WORDS(words));
    /**
     * formulas for conversion of the sensor signals, optimized for fixed point
     * algebra:
     * Temperature = 175 * S_T / 2^16 - 45
     * Relative Humidity = 100 * S_RH / 2^16
     */
    *temperature = ((21875 * (int32_t)words[0]) >> 13) - 45000;
    *humidity = ((12500 * (int32_t)words[1]) >> 13);

    return ret;
}

int16_t SHTC3Driver::shtc1_probe(void) {
    uint32_t serial;
    int16_t ret;

    ret = shtc1_wake_up(); /* Try to wake up the sensor */
    if (ret) {
        return ret;
    }
    return shtc1_read_serial(&serial);
}

void SHTC3Driver::shtc1_enable_low_power_mode(uint8_t enable_low_power_mode) {
    shtc1_cmd_measure =
        enable_low_power_mode ? SHTC1_CMD_MEASURE_LPM : SHTC1_CMD_MEASURE_HPM;
}

int16_t SHTC3Driver::shtc1_read_serial(uint32_t* serial) {
    int16_t ret;
    const uint16_t tx_words[] = {0x007B};
    uint16_t serial_words[SENSIRION_NUM_WORDS(*serial)];

    ret = sensirion_i2c_write_cmd_with_args(SHTC1_ADDRESS, 0xC595, tx_words,
                                            SENSIRION_NUM_WORDS(tx_words));
    if (ret)
        printf("err sensirion_i2c_write_cmd_with_args");
        return ret;

    sensirion_i2c_hal_sleep_usec(SHTC1_CMD_DURATION_USEC);

    ret = sensirion_i2c_delayed_read_cmd(
        SHTC1_ADDRESS, 0xC7F7, SHTC1_CMD_DURATION_USEC, &serial_words[0], 1);
    if (ret)
        printf("err sensirion_i2c_delayed_read_cmd");
        return ret;

    ret = sensirion_i2c_delayed_read_cmd(
        SHTC1_ADDRESS, 0xC7F7, SHTC1_CMD_DURATION_USEC, &serial_words[1], 1);
    if (ret)
        printf("err sensirion_i2c_delayed_read_cmd 2");
        return ret;

    *serial = ((uint32_t)serial_words[0] << 16) | serial_words[1];
    return ret;
}

uint8_t SHTC3Driver::shtc1_get_configured_address(void) {
    return SHTC1_ADDRESS;
}

SHTC3Driver::SHTC3Driver() : SensirionDriver(), shtc1_cmd_measure(SHTC1_CMD_MEASURE_HPM)
{}
