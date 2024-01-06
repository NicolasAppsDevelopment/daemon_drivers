/**
 * \file
 *
 * \brief Sensirion SHT driver interface
 *
 * This module provides access to the SHT functionality over a generic I2C
 * interface. It supports measurements without clock stretching only.
 */

#ifndef SHTC1_H
#define SHTC1_H

#include "Sensirion-driver-base/sensirion_driver.h"

#define STATUS_OK 0
#define STATUS_ERR_BAD_DATA (-1)
#define STATUS_CRC_FAIL (-2)
#define STATUS_UNKNOWN_DEVICE (-3)
#define SHTC1_MEASUREMENT_DURATION_USEC 14400

/* all measurement commands return T (CRC) RH (CRC) */
#if USE_SENSIRION_CLOCK_STRETCHING
#define SHTC1_CMD_MEASURE_HPM 0x7CA2
#define SHTC1_CMD_MEASURE_LPM 0x6458
#else /* USE_SENSIRION_CLOCK_STRETCHING */
#define SHTC1_CMD_MEASURE_HPM 0x7866
#define SHTC1_CMD_MEASURE_LPM 0x609C
#endif /* USE_SENSIRION_CLOCK_STRETCHING */

#define SHTC1_CMD_DURATION_USEC 1000

#define SHTC3_CMD_SLEEP 0xB098
#define SHTC3_CMD_WAKEUP 0x3517
#define SHTC1_ADDRESS 0x71

class SHTC3Driver : public SensirionDriver {
public:
    SHTC3Driver();

    uint16_t shtc1_cmd_measure;

    /**
     * Detects if a sensor is connected by reading out the ID register.
     * If the sensor does not answer or if the answer is not the expected value,
     * the test fails.
     *
     * @return 0 if a sensor was detected
     */
    int16_t shtc1_probe(void);

    /**
     * Starts a measurement and then reads out the results. This function blocks
     * while the measurement is in progress. The duration of the measurement depends
     * on the sensor in use, please consult the datasheet.
     * Temperature is returned in [degree Celsius], multiplied by 1000,
     * and relative humidity in [percent relative humidity], multiplied by 1000.
     *
     * @param temperature   the address for the result of the temperature
     * measurement
     * @param humidity      the address for the result of the relative humidity
     * measurement
     * @return              0 if the command was successful, else an error code.
     */
    int16_t shtc1_measure_blocking_read(int32_t* temperature, int32_t* humidity);

    /**
     * Starts a measurement in high precision mode. Use shtc1_read() to read out the
     * values, once the measurement is done. The duration of the measurement depends
     * on the sensor in use, please consult the datasheet.
     *
     * @return     0 if the command was successful, else an error code.
     */
    int16_t shtc1_measure(void);

    /**
     * Reads out the results of a measurement that was previously started by
     * shtc1_measure(). If the measurement is still in progress, this function
     * returns an error.
     * Temperature is returned in [degree Celsius], multiplied by 1000,
     * and relative humidity in [percent relative humidity], multiplied by 1000.
     *
     * @param temperature   the address for the result of the temperature
     * measurement
     * @param humidity      the address for the result of the relative humidity
     * measurement
     * @return              0 if the command was successful, else an error code.
     */
    int16_t shtc1_read(int32_t* temperature, int32_t* humidity);

    /**
     * Send the sensor to sleep, if supported.
     *
     * Note: DESPITE THE NAME, THIS COMMAND IS ONLY AVAILABLE FOR THE SHTC3
     *
     * Usage:
     * ```
     * int16_t ret;
     * int32_t temperature, humidity;
     * ret = shtc1_wake_up();
     * if (ret) {
     *     // error waking up
     * }
     * ret = shtc1_measure_blocking_read(&temperature, &humidity);
     * if (ret) {
     *     // error measuring
     * }
     * ret = shtc1_sleep();
     * if (ret) {
     *     // error sending sensor to sleep
     * }
     * ```
     *
     * @return  0 if the command was successful, else an error code.
     */
    int16_t shtc1_sleep(void);

    /**
     * Wake the sensor from sleep
     *
     * Note: DESPITE THE NAME, THIS COMMAND IS ONLY AVAILABLE FOR THE SHTC3
     *
     * Usage:
     * ```
     * int16_t ret;
     * int32_t temperature, humidity;
     * ret = shtc1_wake_up();
     * if (ret) {
     *     // error waking up
     * }
     * ret = shtc1_measure_blocking_read(&temperature, &humidity);
     * if (ret) {
     *     // error measuring
     * }
     * ret = shtc1_sleep();
     * if (ret) {
     *     // error sending sensor to sleep
     * }
     * ```
     *
     * @return  0 if the command was successful, else an error code.
     */
    int16_t shtc1_wake_up(void);

    /**
     * Enable or disable the SHT's low power mode
     *
     * @param enable_low_power_mode 1 to enable low power mode, 0 to disable
     */
    void shtc1_enable_low_power_mode(uint8_t enable_low_power_mode);

    /**
     * Read out the serial number
     *
     * @param serial    the address for the result of the serial number
     * @return          0 if the command was successful, else an error code.
     */
    int16_t shtc1_read_serial(uint32_t* serial);

    /**
     * Return the driver version
     *
     * @return Driver version string
     */
    const char* shtc1_get_driver_version(void);

    /**
     * Returns the configured SHT address.
     *
     * @return The configured i2c address
     */
    uint8_t shtc1_get_configured_address(void);
};

#endif /* SHTC1_H */
