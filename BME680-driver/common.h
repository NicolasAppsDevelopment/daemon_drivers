#include "../types.h"

/**
 * BME68XCommon - BME68X driver common class
 */
class BME68XCommon
{
public:
	/**
	 * Initialize all hard- and software components that are needed for the I2C
	 * communication.
	 */
	static int i2c_hal_init(void);

	/**
	 * Automatically test the sensor return a success or error code.
	 *
	 * @return 0 on success, error code otherwise
	 */
	static int bme680_self_test(void);

	/**
	 * Get pressure measurement from the sensor.
	 *
	 * @param p pointer to store the pressure measurement.
	 * @return 0 on success, error code otherwise
	 */
	static int bme680_get_measure(float* p);

	/**
	 * Release all resources initialized by sensirion_i2c_hal_init().
	 */
	static void i2c_hal_free(void);

	/**
	 * Execute one read transaction on the I2C bus, reading a given number of bytes.
	 * If the device does not acknowledge the read command, an error shall be
	 * returned.
	 *
	 * @param address 7-bit I2C address to read from
	 * @param data    pointer to the buffer where the data is to be stored
	 * @param count   number of bytes to read from I2C and store in the buffer
	 * @returns 0 on success, error code otherwise
	 */
	static int8_t bme68x_i2c_read(Byte address, Byte* data, UInt count);

	/**
	 * Execute one write transaction on the I2C bus, sending a given number of
	 * bytes. The bytes in the supplied buffer must be sent to the given address. If
	 * the slave device does not acknowledge any of the bytes, an error shall be
	 * returned.
	 *
	 * @param address 7-bit I2C address to write to
	 * @param data    pointer to the buffer containing the data to write
	 * @param count   number of bytes to read from the buffer and send over I2C
	 * @returns 0 on success, error code otherwise
	 */
	static int8_t bme68x_i2c_write(Byte address, const Byte* data, UInt count);

	/**
	 * Sleep for a given number of microseconds. The function should delay the
	 * execution approximately, but no less than, the given time.
	 *
	 * When using hardware i2c:
	 * Despite the unit, a <10 millisecond precision is sufficient.
	 *
	 * When using software i2c:
	 * The precision needed depends on the desired i2c frequency, i.e. should be
	 * exact to about half a clock cycle (defined in
	 * `SENSIRION_I2C_CLOCK_PERIOD_USEC` in `sensirion_sw_i2c_gpio.h`).
	 *
	 * Example with 400kHz requires a precision of 1 / (2 * 400kHz) == 1.25usec.
	 *
	 * @param useconds the sleep time in microseconds
	 */
	static void bme68x_delay_us(UInt useconds);
};