#include "sensirion_driver.h"
#include "sensirion_common.h"
#include "sensirion_config.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
int16_t SensirionDriver::sensirion_i2c_hal_init(void) {
    /* open i2c adapter */
    i2c_device = open(I2C_DEVICE_PATH, O_RDWR);
    if (i2c_device == -1) {
        return -1;
    }
    return 0;
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
 */
void SensirionDriver::sensirion_i2c_hal_free(void) {
    if (i2c_device >= 0) {
        close(i2c_device);
        i2c_device = -1;
        i2c_address = 0;
    }
}

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
int8_t SensirionDriver::sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
    if (i2c_address != address) {
        ioctl(i2c_device, I2C_SLAVE, address);
        i2c_address = address;
    }

    if (read(i2c_device, data, count) != count) {
        return I2C_READ_FAILED;
    }
    return 0;
}

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
int8_t SensirionDriver::sensirion_i2c_hal_write(uint8_t address, const uint8_t* data,
                               uint16_t count) {
    if (i2c_address != address) {
        ioctl(i2c_device, I2C_SLAVE, address);
        i2c_address = address;
    }

    if (write(i2c_device, data, count) != count) {
        return I2C_WRITE_FAILED;
    }
    return 0;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void SensirionDriver::sensirion_i2c_hal_sleep_usec(uint32_t useconds) {
    usleep(useconds);
}

SensirionDriver::SensirionDriver()
{
    this->i2c_device = -1;
    this->i2c_address = 0;
}





uint8_t SensirionDriver::sensirion_i2c_generate_crc(const uint8_t* data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;

    /* calculates 8-Bit checksum with given polynomial */
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

int8_t SensirionDriver::sensirion_i2c_check_crc(const uint8_t* data, uint16_t count,
                               uint8_t checksum) {
    if (sensirion_i2c_generate_crc(data, count) != checksum)
        return CRC_ERROR;
    return NO_ERROR;
}

int16_t SensirionDriver::sensirion_i2c_general_call_reset(void) {
    const uint8_t data = 0x06;
    return sensirion_i2c_hal_write(0, &data, (uint16_t)sizeof(data));
}

uint16_t SensirionDriver::sensirion_i2c_fill_cmd_send_buf(uint8_t* buf, uint16_t cmd,
                                         const uint16_t* args,
                                         uint8_t num_args) {
    uint8_t i;
    uint16_t idx = 0;

    buf[idx++] = (uint8_t)((cmd & 0xFF00) >> 8);
    buf[idx++] = (uint8_t)((cmd & 0x00FF) >> 0);

    for (i = 0; i < num_args; ++i) {
        buf[idx++] = (uint8_t)((args[i] & 0xFF00) >> 8);
        buf[idx++] = (uint8_t)((args[i] & 0x00FF) >> 0);

        uint8_t crc = sensirion_i2c_generate_crc((uint8_t*)&buf[idx - 2],
                                                 SENSIRION_WORD_SIZE);
        buf[idx++] = crc;
    }
    return idx;
}

int16_t SensirionDriver::sensirion_i2c_read_words_as_bytes(uint8_t address, uint8_t* data,
                                          uint16_t num_words) {
    int16_t ret;
    uint16_t i, j;
    uint16_t size = num_words * (SENSIRION_WORD_SIZE + CRC8_LEN);
    uint16_t word_buf[SENSIRION_MAX_BUFFER_WORDS];
    uint8_t* const buf8 = (uint8_t*)word_buf;

    ret = sensirion_i2c_hal_read(address, buf8, size);
    if (ret != NO_ERROR)
        return ret;

    /* check the CRC for each word */
    for (i = 0, j = 0; i < size; i += SENSIRION_WORD_SIZE + CRC8_LEN) {

        ret = sensirion_i2c_check_crc(&buf8[i], SENSIRION_WORD_SIZE,
                                      buf8[i + SENSIRION_WORD_SIZE]);
        if (ret != NO_ERROR)
            return ret;

        data[j++] = buf8[i];
        data[j++] = buf8[i + 1];
    }

    return NO_ERROR;
}

int16_t SensirionDriver::sensirion_i2c_read_words(uint8_t address, uint16_t* data_words,
                                 uint16_t num_words) {
    int16_t ret;
    uint8_t i;

    ret = sensirion_i2c_read_words_as_bytes(address, (uint8_t*)data_words,
                                            num_words);
    if (ret != NO_ERROR)
        return ret;

    for (i = 0; i < num_words; ++i) {
        const uint8_t* word_bytes = (uint8_t*)&data_words[i];
        data_words[i] = ((uint16_t)word_bytes[0] << 8) | word_bytes[1];
    }

    return NO_ERROR;
}

int16_t SensirionDriver::sensirion_i2c_write_cmd(uint8_t address, uint16_t command) {
    uint8_t buf[SENSIRION_COMMAND_SIZE];

    sensirion_i2c_fill_cmd_send_buf(buf, command, NULL, 0);
    return sensirion_i2c_hal_write(address, buf, SENSIRION_COMMAND_SIZE);
}

int16_t SensirionDriver::sensirion_i2c_write_cmd_with_args(uint8_t address, uint16_t command,
                                          const uint16_t* data_words,
                                          uint16_t num_words) {
    uint8_t buf[SENSIRION_MAX_BUFFER_WORDS];
    uint16_t buf_size;

    buf_size =
        sensirion_i2c_fill_cmd_send_buf(buf, command, data_words, num_words);
    return sensirion_i2c_hal_write(address, buf, buf_size);
}

int16_t SensirionDriver::sensirion_i2c_delayed_read_cmd(uint8_t address, uint16_t cmd,
                                       uint32_t delay_us, uint16_t* data_words,
                                       uint16_t num_words) {
    int16_t ret;
    uint8_t buf[SENSIRION_COMMAND_SIZE];

    sensirion_i2c_fill_cmd_send_buf(buf, cmd, NULL, 0);
    ret = sensirion_i2c_hal_write(address, buf, SENSIRION_COMMAND_SIZE);
    if (ret != NO_ERROR)
        return ret;

    if (delay_us)
        sensirion_i2c_hal_sleep_usec(delay_us);

    return sensirion_i2c_read_words(address, data_words, num_words);
}

int16_t SensirionDriver::sensirion_i2c_read_cmd(uint8_t address, uint16_t cmd,
                               uint16_t* data_words, uint16_t num_words) {
    return sensirion_i2c_delayed_read_cmd(address, cmd, 0, data_words,
                                          num_words);
}

uint16_t SensirionDriver::sensirion_i2c_add_command_to_buffer(uint8_t* buffer, uint16_t offset,
                                             uint16_t command) {
    buffer[offset++] = (uint8_t)((command & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((command & 0x00FF) >> 0);
    return offset;
}

uint16_t SensirionDriver::sensirion_i2c_add_uint32_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                              uint32_t data) {
    buffer[offset++] = (uint8_t)((data & 0xFF000000) >> 24);
    buffer[offset++] = (uint8_t)((data & 0x00FF0000) >> 16);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;
    buffer[offset++] = (uint8_t)((data & 0x0000FF00) >> 8);
    buffer[offset++] = (uint8_t)((data & 0x000000FF) >> 0);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;

    return offset;
}

uint16_t SensirionDriver::sensirion_i2c_add_int32_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                             int32_t data) {
    return sensirion_i2c_add_uint32_t_to_buffer(buffer, offset, (uint32_t)data);
}

uint16_t SensirionDriver::sensirion_i2c_add_uint16_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                              uint16_t data) {
    buffer[offset++] = (uint8_t)((data & 0xFF00) >> 8);
    buffer[offset++] = (uint8_t)((data & 0x00FF) >> 0);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;

    return offset;
}

uint16_t SensirionDriver::sensirion_i2c_add_int16_t_to_buffer(uint8_t* buffer, uint16_t offset,
                                             int16_t data) {
    return sensirion_i2c_add_uint16_t_to_buffer(buffer, offset, (uint16_t)data);
}

uint16_t SensirionDriver::sensirion_i2c_add_float_to_buffer(uint8_t* buffer, uint16_t offset,
                                           float data) {
    union {
        uint32_t uint32_data;
        float float_data;
    } convert;

    convert.float_data = data;

    buffer[offset++] = (uint8_t)((convert.uint32_data & 0xFF000000) >> 24);
    buffer[offset++] = (uint8_t)((convert.uint32_data & 0x00FF0000) >> 16);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;
    buffer[offset++] = (uint8_t)((convert.uint32_data & 0x0000FF00) >> 8);
    buffer[offset++] = (uint8_t)((convert.uint32_data & 0x000000FF) >> 0);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;

    return offset;
}

uint16_t SensirionDriver::sensirion_i2c_add_bytes_to_buffer(uint8_t* buffer, uint16_t offset,
                                           const uint8_t* data,
                                           uint16_t data_length) {
    uint16_t i;

    if (data_length % SENSIRION_WORD_SIZE != 0) {
        return BYTE_NUM_ERROR;
    }

    for (i = 0; i < data_length; i += 2) {
        buffer[offset++] = data[i];
        buffer[offset++] = data[i + 1];

        buffer[offset] = sensirion_i2c_generate_crc(
            &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
        offset++;
    }

    return offset;
}

int16_t SensirionDriver::sensirion_i2c_write_data(uint8_t address, const uint8_t* data,
                                 uint16_t data_length) {
    return sensirion_i2c_hal_write(address, data, data_length);
}

int16_t SensirionDriver::sensirion_i2c_read_data_inplace(uint8_t address, uint8_t* buffer,
                                        uint16_t expected_data_length) {
    int16_t error;
    uint16_t i, j;
    uint16_t size = (expected_data_length / SENSIRION_WORD_SIZE) *
                    (SENSIRION_WORD_SIZE + CRC8_LEN);

    if (expected_data_length % SENSIRION_WORD_SIZE != 0) {
        return BYTE_NUM_ERROR;
    }

    error = sensirion_i2c_hal_read(address, buffer, size);
    if (error) {
        return error;
    }

    for (i = 0, j = 0; i < size; i += SENSIRION_WORD_SIZE + CRC8_LEN) {

        error = sensirion_i2c_check_crc(&buffer[i], SENSIRION_WORD_SIZE,
                                        buffer[i + SENSIRION_WORD_SIZE]);
        if (error) {
            return error;
        }
        buffer[j++] = buffer[i];
        buffer[j++] = buffer[i + 1];
    }

    return NO_ERROR;
}
