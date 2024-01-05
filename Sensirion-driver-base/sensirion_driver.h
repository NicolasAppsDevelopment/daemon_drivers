#ifndef SENSIRIONDRIVER_H
#define SENSIRIONDRIVER_H

#include "sensirion_config.h"

/**
 * Linux specific configuration. Adjust the following define to the device path
 * of your sensor.
 */
#define I2C_DEVICE_PATH "/dev/i2c-1"

/**
 * The following define was taken from i2c-dev.h. Alternatively the header file
 * can be included. The define was added in Linux v3.10 and never changed since
 * then.
 */
#define I2C_SLAVE 0x0703

#define I2C_WRITE_FAILED -1
#define I2C_READ_FAILED -1

#define CRC_ERROR 1
#define I2C_BUS_ERROR 2
#define I2C_NACK_ERROR 3
#define BYTE_NUM_ERROR 4

#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF
#define CRC8_LEN 1

#define SENSIRION_COMMAND_SIZE 2
#define SENSIRION_WORD_SIZE 2
#define SENSIRION_NUM_WORDS(x) (sizeof(x) / SENSIRION_WORD_SIZE)
#define SENSIRION_MAX_BUFFER_WORDS 32

class SensirionDriver
{
protected:
    int i2c_device;
    uint8_t i2c_address;

    uint8_t sensirion_i2c_generate_crc(const uint8_t* data, uint16_t count);

    int8_t sensirion_i2c_check_crc(const uint8_t* data, uint16_t count,
                                   uint8_t checksum);

    /**
     * sensirion_i2c_general_call_reset() - Send a general call reset.
     *
     * @warning This will reset all attached I2C devices on the bus which support
     *          general call reset.
     *
     * @return  NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_general_call_reset(void);

    /**
     * sensirion_i2c_fill_cmd_send_buf() - create the i2c send buffer for a command
     * and a set of argument words. The output buffer interleaves argument words
     * with their checksums.
     * @buf:        The generated buffer to send over i2c. Then buffer length must
     *              be at least SENSIRION_COMMAND_LEN + num_args *
     *              (SENSIRION_WORD_SIZE + CRC8_LEN).
     * @cmd:        The i2c command to send. It already includes a checksum.
     * @args:       The arguments to the command. Can be NULL if none.
     * @num_args:   The number of word arguments in args.
     *
     * @return      The number of bytes written to buf
     */
    uint16_t sensirion_i2c_fill_cmd_send_buf(uint8_t* buf, uint16_t cmd,
                                             const uint16_t* args,
                                             uint8_t num_args);

    /**
     * sensirion_i2c_read_words() - read data words from sensor
     *
     * @address:    Sensor i2c address
     * @data_words: Allocated buffer to store the read words.
     *              The buffer may also have been modified in case of an error.
     * @num_words:  Number of data words to read (without CRC bytes)
     *
     * @return      NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_words(uint8_t address, uint16_t* data_words,
                                     uint16_t num_words);

    /**
     * sensirion_i2c_read_words_as_bytes() - read data words as byte-stream from
     *                                       sensor
     *
     * Read bytes without adjusting values to the uP's word-order.
     *
     * @address:    Sensor i2c address
     * @data:       Allocated buffer to store the read bytes.
     *              The buffer may also have been modified in case of an error.
     * @num_words:  Number of data words(!) to read (without CRC bytes)
     *              Since only word-chunks can be read from the sensor the size
     *              is still specified in sensor-words (num_words = num_bytes *
     *              SENSIRION_WORD_SIZE)
     *
     * @return      NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_words_as_bytes(uint8_t address, uint8_t* data,
                                              uint16_t num_words);

    /**
     * sensirion_i2c_write_cmd() - writes a command to the sensor
     * @address:    Sensor i2c address
     * @command:    Sensor command
     *
     * @return      NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_write_cmd(uint8_t address, uint16_t command);

    /**
     * sensirion_i2c_write_cmd_with_args() - writes a command with arguments to the
     *                                       sensor
     * @address:    Sensor i2c address
     * @command:    Sensor command
     * @data:       Argument buffer with words to send
     * @num_words:  Number of data words to send (without CRC bytes)
     *
     * @return      NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_write_cmd_with_args(uint8_t address, uint16_t command,
                                              const uint16_t* data_words,
                                              uint16_t num_words);

    /**
     * sensirion_i2c_delayed_read_cmd() - send a command, wait for the sensor to
     *                                    process and read data back
     * @address:    Sensor i2c address
     * @cmd:        Command
     * @delay:      Time in microseconds to delay sending the read request
     * @data_words: Allocated buffer to store the read data
     * @num_words:  Data words to read (without CRC bytes)
     *
     * @return      NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_delayed_read_cmd(uint8_t address, uint16_t cmd,
                                           uint32_t delay_us, uint16_t* data_words,
                                           uint16_t num_words);
    /**
     * sensirion_i2c_read_cmd() - reads data words from the sensor after a command
     *                            is issued
     * @address:    Sensor i2c address
     * @cmd:        Command
     * @data_words: Allocated buffer to store the read data
     * @num_words:  Data words to read (without CRC bytes)
     *
     * @return      NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_cmd(uint8_t address, uint16_t cmd,
                                   uint16_t* data_words, uint16_t num_words);

    /**
     * sensirion_i2c_add_command_to_buffer() - Add a command to the buffer at
     *                                         offset. Adds 2 bytes to the buffer.
     *
     * @param buffer  Pointer to buffer in which the write frame will be prepared.
     *                Caller needs to make sure that there is enough space after
     *                offset left to write the data into the buffer.
     * @param offset  Offset of the next free byte in the buffer.
     * @param command Command to be written into the buffer.
     *
     * @return        Offset of next free byte in the buffer after writing the data.
     */
    uint16_t sensirion_i2c_add_command_to_buffer(uint8_t* buffer, uint16_t offset,
                                                 uint16_t command);

    /**
     * sensirion_i2c_add_uint32_t_to_buffer() - Add a uint32_t to the buffer at
     *                                          offset. Adds 6 bytes to the buffer.
     *
     * @param buffer  Pointer to buffer in which the write frame will be prepared.
     *                Caller needs to make sure that there is enough space after
     *                offset left to write the data into the buffer.
     * @param offset  Offset of the next free byte in the buffer.
     * @param data    uint32_t to be written into the buffer.
     *
     * @return        Offset of next free byte in the buffer after writing the data.
     */
    uint16_t sensirion_i2c_add_uint32_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                                  uint32_t data);

    /**
     * sensirion_i2c_add_int32_t_to_buffer() - Add a int32_t to the buffer at
     *                                         offset. Adds 6 bytes to the buffer.
     *
     * @param buffer  Pointer to buffer in which the write frame will be prepared.
     *                Caller needs to make sure that there is enough space after
     *                offset left to write the data into the buffer.
     * @param offset  Offset of the next free byte in the buffer.
     * @param data    int32_t to be written into the buffer.
     *
     * @return        Offset of next free byte in the buffer after writing the data.
     */
    uint16_t sensirion_i2c_add_int32_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                                 int32_t data);

    /**
     * sensirion_i2c_add_uint16_t_to_buffer() - Add a uint16_t to the buffer at
     *                                          offset. Adds 3 bytes to the buffer.
     *
     * @param buffer  Pointer to buffer in which the write frame will be prepared.
     *                Caller needs to make sure that there is enough space after
     *                offset left to write the data into the buffer.
     * @param offset  Offset of the next free byte in the buffer.
     * @param data    uint16_t to be written into the buffer.
     *
     * @return        Offset of next free byte in the buffer after writing the data.
     */
    uint16_t sensirion_i2c_add_uint16_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                                  uint16_t data);

    /**
     * sensirion_i2c_add_int16_t_to_buffer() - Add a int16_t to the buffer at
     *                                         offset. Adds 3 bytes to the buffer.
     *
     * @param buffer  Pointer to buffer in which the write frame will be prepared.
     *                Caller needs to make sure that there is enough space after
     *                offset left to write the data into the buffer.
     * @param offset  Offset of the next free byte in the buffer.
     * @param data    int16_t to be written into the buffer.
     *
     * @return        Offset of next free byte in the buffer after writing the data.
     */
    uint16_t sensirion_i2c_add_int16_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                                 int16_t data);

    /**
     * sensirion_i2c_add_float_to_buffer() - Add a float to the buffer at offset.
     *                                       Adds 6 bytes to the buffer.
     *
     * @param buffer  Pointer to buffer in which the write frame will be prepared.
     *                Caller needs to make sure that there is enough space after
     *                offset left to write the data into the buffer.
     * @param offset  Offset of the next free byte in the buffer.
     * @param data    float to be written into the buffer.
     *
     * @return        Offset of next free byte in the buffer after writing the data.
     */
    uint16_t sensirion_i2c_add_float_to_buffer(uint8_t* buffer, uint16_t offset,
                                               float data);

    /**
     * sensirion_i2c_add_bytes_to_buffer() - Add a byte array to the buffer at
     *                                       offset.
     *
     * @param buffer      Pointer to buffer in which the write frame will be
     *                    prepared. Caller needs to make sure that there is
     *                    enough space after offset left to write the data
     *                    into the buffer.
     * @param offset      Offset of the next free byte in the buffer.
     * @param data        Pointer to data to be written into the buffer.
     * @param data_length Number of bytes to be written into the buffer. Needs to
     *                    be a multiple of SENSIRION_WORD_SIZE otherwise the
     *                    function returns BYTE_NUM_ERROR.
     *
     * @return            Offset of next free byte in the buffer after writing the
     *                    data.
     */
    uint16_t sensirion_i2c_add_bytes_to_buffer(uint8_t* buffer, uint16_t offset,
                                               const uint8_t* data,
                                               uint16_t data_length);

    /**
     * sensirion_i2c_write_data() - Writes data to the Sensor.
     *
     * @note This is just a wrapper for sensirion_i2c_hal_write() to
     *       not need to include the HAL in the drivers.
     *
     * @param address     I2C address to write to.
     * @param data        Pointer to the buffer containing the data to write.
     * @param data_length Number of bytes to send to the Sensor.
     *
     * @return        NO_ERROR on success, error code otherwise
     */
    int16_t sensirion_i2c_write_data(uint8_t address, const uint8_t* data,
                                     uint16_t data_length);

    /**
     * sensirion_i2c_read_data_inplace() - Reads data from the Sensor.
     *
     * @param address              Sensor I2C address
     * @param buffer               Allocated buffer to store data as bytes. Needs
     *                             to be big enough to store the data including
     *                             CRC. Twice the size of data should always
     *                             suffice.
     * @param expected_data_length Number of bytes to read (without CRC). Needs
     *                             to be a multiple of SENSIRION_WORD_SIZE,
     *                             otherwise the function returns BYTE_NUM_ERROR.
     *
     * @return            NO_ERROR on success, an error code otherwise
     */
    int16_t sensirion_i2c_read_data_inplace(uint8_t address, uint8_t* buffer,
                                            uint16_t expected_data_length);

public:
    SensirionDriver();

    /**
     * Select the current i2c bus by index.
     * All following i2c operations will be directed at that bus.
     *
     * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
     * bus)
     *
     * @param bus_idx   Bus index to select
     * @returns         0 on success, an error code otherwise
     */
    int16_t sensirion_i2c_hal_select_bus(uint8_t bus_idx);

    /**
     * Initialize all hard- and software components that are needed for the I2C
     * communication.
     */
    int16_t sensirion_i2c_hal_init(void);

    /**
     * Release all resources initialized by sensirion_i2c_hal_init().
     */
    void sensirion_i2c_hal_free(void);

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
    int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count);

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
    int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data,
                                   uint16_t count);

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
    void sensirion_i2c_hal_sleep_usec(uint32_t useconds);

};

#endif // SENSIRIONDRIVER_H
