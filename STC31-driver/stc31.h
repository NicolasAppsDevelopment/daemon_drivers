#ifndef STC31_H
#define STC31_H

#include "../Sensirion-driver-base/sensirion_config.h"
#include "../Sensirion-driver-base/sensirion_driver.h"
#include "../types.h"

#define STC3X_I2C_ADDRESS 0x29

/**
* STC31Driver - STC31 driver class
* @see https://github.com/Sensirion/raspberry-pi-i2c-stc3x 
*/
class STC31Driver : public SensirionDriver {
public:
    STC31Driver();

    /**
     * stc3x_set_binary_gas() - The STC3x measures the concentration of binary gas
    mixtures. It is important to note that the STC3x is not selective for gases, and
    it assumes that the binary gas is set correctly. The sensor can only give a
    correct concentration value when only the gases set with this command are
    present. When the system is reset, or wakes up from sleep mode, the sensor goes
    back to default mode, in which no binary gas is selected. This means that the
    binary gas must be reconfigured. When no binary gas is selected (default mode)
    the concentration measurement will return undefined results. This allows to
    detect unexpected sensor interruption (e.g. due to temporary power loss) and
    consequently reset the binary gas to the appropriate mixture.
     *
     * @param binary_gas See section 3.3.2 in the corresponding datasheet for a list
    of available binary gases. STC31: - 0x0000: CO₂ in N₂ for range in 0 to 100 vol%
    - 0x0001: CO₂ in air for range in 0 to 100 vol% - 0x0002: CO₂ in N₂ for range in
    0 to 25 vol% - 0x0003: CO₂ in air for range in 0 to 25 vol%
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_set_binary_gas(UShort binary_gas);

    /**
     * stc3x_set_relative_humidity() - As mentioned in section 5.1 of the datasheet,
    the measurement principle of the concentration measurement is dependent on the
    humidity of the gas. With the set relative humidity command, the sensor uses
    internal algorithms to compensate the concentration results. When no value is
    written to the sensor after a soft reset, wake-up or power-up, a relative
    humidity of 0% is assumed. The value written to the sensor is used until a new
    value is written to the sensor
     *
     * @param relative_humidity_ticks Convert %RH to value by: RH * (2^16 - 1) / 100
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_set_relative_humidity(UShort relative_humidity_ticks);

    /**
     * stc3x_set_temperature() - The concentration measurement requires a
    compensation of temperature. Per default, the sensor uses the internal
    temperature sensor to compensate the concentration results. However, when using
    the SHTxx, it is recommended to also use its temperature value, because it is
    more accurate. When no value is written to the sensor after a soft reset,
    wake-up or power-up, the internal temperature signal is used. The value written
    to the sensor is used until a new value is written to the sensor.
     *
     * @param temperature_ticks Convert °C to value by: T * 200
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_set_temperature(UShort temperature_ticks);

    /**
     * stc3x_set_pressure() - A pressure value can be written into the sensor, for
    density compensation of the gas concentration measurement. It is recommended to
    set the pressure level, if it differs significantly from 1013mbar. Pressure
    compensation is valid from 600mbar to 1200mbar. When no value is written to the
    sensor after a soft reset, wake-up or power-up, a pressure of 1013mbar is
    assumed. The value written is used until a new value is written to the sensor.
     *
     * @param absolue_pressure Ambient pressure in mbar (milli-bars)
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_set_pressure(UShort absolue_pressure);

    /**
     * stc3x_measure_gas_concentration() - The measurement of gas concentration is
    done in one measurement in a single shot, and takes less than 66ms. When
    measurement data is available, it can be read out by sending an I2C read header
    and reading out the data from the sensor. If no measurement data is available
    yet, the sensor will respond with a NACK on the I2C read header. In case the
    ‘Set temperature command’ has been used prior to the measurement command, the
    temperature value given out by the STC3x will be that one of the ‘Set
    temperature command’. When the ‘Set temperature command’ has not been used, the
    internal temperature value can be read out. During product development it is
    recommended to compare the internal temperature value of the STC3x and the
    temperature value of the SHTxx, to check whether both sensors are properly
    thermally coupled. The values must be within 0.7°C.
     *
     * @note The Gas concentration is a 16-bit unsigned integer. The temperature and
    byte 7 and 8 don’t need to be read out. The read sequence can be aborted after
    any byte by a NACK and a STOP condition. The measurement command should not be
    triggered more often than once a second.
     *
     * @param gas_ticks Gas concentration. Convert to val % by 100 * (value - 2^14)
    / 2^15
     *
     * @param temperature_ticks Temperature. Convert to °C by value / 200
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_measure_gas_concentration(UShort* gas_ticks,
                                            int16_t *temperature_ticks);

    /**
     * stc3x_forced_recalibration() - Forced recalibration (FRC) is used to improve
     * the sensor output with a known reference value. See the Field Calibration
     * Guide for more details. If no argument is given, the sensor will assume a
     * default value of 0 vol%. This command will trigger a concentration
     * measurement as described in 3.3.6 of the datasheet and therefore it will take
     * the same measurement time.
     *
     * @param reference_concentration Reference concentration
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_forced_recalibration(UShort reference_concentration);

    /**
     * stc3x_enable_automatic_self_calibration() - Enable the automatic
    self-calibration (ASC). The sensor can run in automatic self-calibration mode.
    This mode will enhance the accuracy for applications where the target gas is not
    present for the majority of the time. See the Field Calibration Guide for more
    details. This feature can be enabled or disabled by using the commands as shown
    below. The automatic self-calibration is optimized for a gas concentration
    measurement interval of 1s. Substantially different measurement intervals may
    decrease the self-calibration performance. The default state is disabled.
    Automatic self-calibration in combination with sleep mode requires a specific
    sequence of steps. See section 3.3.9 in the datasheet for more detailed
    instructions
     *
     * @note The sensor will apply automatic self-calibration
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_enable_automatic_self_calibration(void);

    /**
     * stc3x_disable_automatic_self_calibration() - Disable the automatic
    self-calibration (ASC). The sensor can run in automatic self-calibration mode.
    This mode will enhance the accuracy for applications where the target gas is not
    present for the majority of the time. See the Field Calibration Guide for more
    details. This feature can be enabled or disabled by using the commands as shown
    below. The default state is disabled.
     *
     * @note The sensor will not apply automatic self-calibration. This is the
    default state of the sensor.
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_disable_automatic_self_calibration(void);

    /**
     * stc3x_prepare_read_state() - The sensor will prepare its current state to be
     * read out.
     *
     * @note See section 3.3.9 of the datasheet for detailed instructions.
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_prepare_read_state(void);

    /**
     * stc3x_set_sensor_state() - Write the sensor state as read out earlier.
     *
     * @param state Current sensor state
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_set_sensor_state(const Byte* state, Byte state_size);

    /**
     * stc3x_get_sensor_state() - Read out the sensor state.
     *
     * @param state Current sensor state
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_get_sensor_state(Byte* state, Byte state_size);

    /**
     * stc3x_apply_state() - The sensor will apply the written state data.
     *
     * @note See section 3.3.9 of the datasheet for detailed instructions.
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_apply_state(void);

    /**
     * stc3x_self_test() - The sensor will run an on-chip self-test. A successful
    self-test will return zero. The 16-bit result of a sensor self-test is a
    combination of possible error states, encoded as bits (starting with lsb): Bits
    | Error State ------------------------------- 0-1: | Memory error 2  : | VDD out
    of range 3-8: | Measurement value error ------------------------------- In case
    of a successful self-test the sensor returns 0x0000 with correct CRC.
     *
     * @param self_test_output Self test result. Error code or 0x0000 on success.
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_self_test(UShort* self_test_output);

    /**
     * stc3x_enter_sleep_mode() - Put sensor into sleep mode.
    In sleep mode the sensor uses the minimum amount of current. The mode can only
    be entered from idle mode, i.e. when the sensor is not measuring. This mode is
    particularly useful for battery operated devices. To minimize the current in
    this mode, the complexity of the sleep mode circuit has been reduced as much as
    possible, which is mainly reflected by the way the sensor exits the sleep mode.
    The sleep command can be sent after the result have been read out and the sensor
    is in idle mode. The sensor exits the sleep mode and enters the idle mode when
    it receives the valid I2C address and a write bit (‘0’). Note that the I2C
    address is not acknowledged. It is possible to poll the sensor to see whether
    the sensor has received the address and has woken up. This takes maximum 12ms.
     *
     * @note Only available in idle mode
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_enter_sleep_mode(void);

    /**
     * stc3x_prepare_product_identifier() - Prepare for reading the product
     * identifier and sensor serial number.
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_prepare_product_identifier(void);

    /**
     * stc3x_read_product_identifier() - Read the product identifier and sensor
    serial number.
     *
     * @note Make sure to call 'prepare product identifier' immediately before.
     *
     * @param product_number 32-bit unique product and revision number. The number
    is listed below: STC31: 0x08010301
     *
     * @param serial_number 64-bit unique serial number
     *
     * @return 0 on success, an error code otherwise
     */
    int16_t stc3x_read_product_identifier(UInt* product_number,
                                          Byte* serial_number,
                                          Byte serial_number_size);

};

#endif /* STC31_H */
