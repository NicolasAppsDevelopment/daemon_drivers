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
int8_t SensirionDriver::sensirion_i2c_hal_read(Byte address, Byte* data, UShort count) {
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
int8_t SensirionDriver::sensirion_i2c_hal_write(Byte address, const Byte* data,
                               UShort count) {
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
void SensirionDriver::sensirion_i2c_hal_sleep_usec(UInt useconds) {
    usleep(useconds);
}

SensirionDriver::SensirionDriver()
{
    this->i2c_device = -1;
    this->i2c_address = 0;
}



Byte SensirionDriver::sensirion_i2c_generate_crc(const Byte* data, UShort count) {
    UShort current_byte;
    Byte crc = CRC8_INIT;
    Byte crc_bit;

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

int8_t SensirionDriver::sensirion_i2c_check_crc(const Byte* data, UShort count,
                               Byte checksum) {
    if (sensirion_i2c_generate_crc(data, count) != checksum) {
        printf("CRC error detected!");
        return CRC_ERROR;
    }
    return NO_ERROR;
}

int16_t SensirionDriver::sensirion_i2c_general_call_reset(void) {
    const Byte data = 0x06;
    return sensirion_i2c_hal_write(0, &data, (UShort)sizeof(data));
}

UShort SensirionDriver::sensirion_i2c_fill_cmd_send_buf(Byte* buf, UShort cmd,
                                         const UShort* args,
                                         Byte num_args) {
    Byte i;
    UShort idx = 0;

    buf[idx++] = (Byte)((cmd & 0xFF00) >> 8);
    buf[idx++] = (Byte)((cmd & 0x00FF) >> 0);

    for (i = 0; i < num_args; ++i) {
        buf[idx++] = (Byte)((args[i] & 0xFF00) >> 8);
        buf[idx++] = (Byte)((args[i] & 0x00FF) >> 0);

        Byte crc = sensirion_i2c_generate_crc((Byte*)&buf[idx - 2],
                                                 SENSIRION_WORD_SIZE);
        buf[idx++] = crc;
    }
    return idx;
}

int16_t SensirionDriver::sensirion_i2c_read_words_as_bytes(Byte address, Byte* data,
                                          UShort num_words) {
    int16_t ret;
    UShort i, j;
    UShort size = num_words * (SENSIRION_WORD_SIZE + CRC8_LEN);
    UShort word_buf[SENSIRION_MAX_BUFFER_WORDS];
    Byte* const buf8 = (Byte*)word_buf;

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

int16_t SensirionDriver::sensirion_i2c_read_words(Byte address, UShort* data_words,
                                 UShort num_words) {
    int16_t ret;
    Byte i;

    ret = sensirion_i2c_read_words_as_bytes(address, (Byte*)data_words,
                                            num_words);
    if (ret != NO_ERROR)
        return ret;

    for (i = 0; i < num_words; ++i) {
        const Byte* word_bytes = (Byte*)&data_words[i];
        data_words[i] = ((UShort)word_bytes[0] << 8) | word_bytes[1];
    }

    return NO_ERROR;
}

int16_t SensirionDriver::sensirion_i2c_write_cmd(Byte address, UShort command) {
    Byte buf[SENSIRION_COMMAND_SIZE];

    sensirion_i2c_fill_cmd_send_buf(buf, command, NULL, 0);
    return sensirion_i2c_hal_write(address, buf, SENSIRION_COMMAND_SIZE);
}

int16_t SensirionDriver::sensirion_i2c_write_cmd_with_args(Byte address, UShort command,
                                          const UShort* data_words,
                                          UShort num_words) {
    Byte buf[SENSIRION_MAX_BUFFER_WORDS];
    UShort buf_size;

    buf_size =
        sensirion_i2c_fill_cmd_send_buf(buf, command, data_words, num_words);
    return sensirion_i2c_hal_write(address, buf, buf_size);
}

int16_t SensirionDriver::sensirion_i2c_delayed_read_cmd(Byte address, UShort cmd,
                                       UInt delay_us, UShort* data_words,
                                       UShort num_words) {
    int16_t ret;
    Byte buf[SENSIRION_COMMAND_SIZE];

    sensirion_i2c_fill_cmd_send_buf(buf, cmd, NULL, 0);
    ret = sensirion_i2c_hal_write(address, buf, SENSIRION_COMMAND_SIZE);
    if (ret != NO_ERROR)
        return ret;

    if (delay_us)
        sensirion_i2c_hal_sleep_usec(delay_us);

    return sensirion_i2c_read_words(address, data_words, num_words);
}

int16_t SensirionDriver::sensirion_i2c_read_cmd(Byte address, UShort cmd,
                               UShort* data_words, UShort num_words) {
    return sensirion_i2c_delayed_read_cmd(address, cmd, 0, data_words,
                                          num_words);
}

UShort SensirionDriver::sensirion_i2c_add_command_to_buffer(Byte* buffer, UShort offset,
                                             UShort command) {
    buffer[offset++] = (Byte)((command & 0xFF00) >> 8);
    buffer[offset++] = (Byte)((command & 0x00FF) >> 0);
    return offset;
}

UShort SensirionDriver::sensirion_i2c_add_uint32_t_to_buffer(Byte* buffer, UShort offset,
                                              UInt data) {
    buffer[offset++] = (Byte)((data & 0xFF000000) >> 24);
    buffer[offset++] = (Byte)((data & 0x00FF0000) >> 16);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;
    buffer[offset++] = (Byte)((data & 0x0000FF00) >> 8);
    buffer[offset++] = (Byte)((data & 0x000000FF) >> 0);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;

    return offset;
}

UShort SensirionDriver::sensirion_i2c_add_int32_t_to_buffer(Byte* buffer, UShort offset,
                                             int32_t data) {
    return sensirion_i2c_add_uint32_t_to_buffer(buffer, offset, (UInt)data);
}

UShort SensirionDriver::sensirion_i2c_add_UShort_to_buffer(Byte* buffer, UShort offset,
                                              UShort data) {
    buffer[offset++] = (Byte)((data & 0xFF00) >> 8);
    buffer[offset++] = (Byte)((data & 0x00FF) >> 0);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;

    return offset;
}

UShort SensirionDriver::sensirion_i2c_add_int16_t_to_buffer(Byte* buffer, UShort offset,
                                             int16_t data) {
    return sensirion_i2c_add_UShort_to_buffer(buffer, offset, (UShort)data);
}

UShort SensirionDriver::sensirion_i2c_add_float_to_buffer(Byte* buffer, UShort offset,
                                           float data) {
    union {
        UInt uint32_data;
        float float_data;
    } convert;

    convert.float_data = data;

    buffer[offset++] = (Byte)((convert.uint32_data & 0xFF000000) >> 24);
    buffer[offset++] = (Byte)((convert.uint32_data & 0x00FF0000) >> 16);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;
    buffer[offset++] = (Byte)((convert.uint32_data & 0x0000FF00) >> 8);
    buffer[offset++] = (Byte)((convert.uint32_data & 0x000000FF) >> 0);
    buffer[offset] = sensirion_i2c_generate_crc(
        &buffer[offset - SENSIRION_WORD_SIZE], SENSIRION_WORD_SIZE);
    offset++;

    return offset;
}

UShort SensirionDriver::sensirion_i2c_add_bytes_to_buffer(Byte* buffer, UShort offset,
                                           const Byte* data,
                                           UShort data_length) {
    UShort i;

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

int16_t SensirionDriver::sensirion_i2c_write_data(Byte address, const Byte* data,
                                 UShort data_length) {
    return sensirion_i2c_hal_write(address, data, data_length);
}

int16_t SensirionDriver::sensirion_i2c_read_data_inplace(Byte address, Byte* buffer,
                                        UShort expected_data_length) {
    int16_t error;
    UShort i, j;
    UShort size = (expected_data_length / SENSIRION_WORD_SIZE) *
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
