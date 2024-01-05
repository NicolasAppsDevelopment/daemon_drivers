/* Enable usleep function */
#define _DEFAULT_SOURCE

#include "bme68x.h"
#include "bme68x_defs.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

static struct bme68x_dev bme_api_dev;
static struct bme68x_conf conf;
static struct bme68x_heatr_conf heatr_conf;
static struct bme68x_data data[10];

#define I2C_WRITE_FAILED -1
#define I2C_READ_FAILED -1

static int i2c_device = -1;
static uint8_t i2c_address = 0;


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
int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    uint8_t reg[1];
    reg[0]=reg_addr;

    if (write(i2c_device, reg, 1) != 1) {
        printf("user_i2c_read_reg");
        rslt = I2C_READ_FAILED;
    }
    if (read(i2c_device, reg_data, len) != len) {
        printf("user_i2c_read_data");
        rslt = I2C_READ_FAILED;
    }

    return rslt;
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
int8_t bme68x_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    uint8_t reg[16];
    reg[0]=reg_addr;

    for (int i=1; i<len+1; i++)
       reg[i] = reg_data[i-1];

    if (write(i2c_device, reg, len+1) != len+1) {
        printf("user_i2c_write");
        rslt = I2C_READ_FAILED;
    }

    return rslt;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void bme68x_delay_us(uint32_t useconds) {
    usleep(useconds);
}


void bme68x_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BME68X_OK:

            /* Do nothing */
            break;
        case BME68X_E_NULL_PTR:
            printf("API name [%s]  Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BME68X_E_COM_FAIL:
            printf("API name [%s]  Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BME68X_E_INVALID_LENGTH:
            printf("API name [%s]  Error [%d] : Incorrect length parameter\r\n", api_name, rslt);
            break;
        case BME68X_E_DEV_NOT_FOUND:
            printf("API name [%s]  Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        case BME68X_E_SELF_TEST:
            printf("API name [%s]  Error [%d] : Self test error\r\n", api_name, rslt);
            break;
        case BME68X_W_NO_NEW_DATA:
            printf("API name [%s]  Warning [%d] : No new data found\r\n", api_name, rslt);
            break;
        default:
            printf("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
int i2c_hal_init(void) {
    /* open i2c adapter */
    i2c_device = open(I2C_DEVICE_PATH, O_RDWR);
    if (i2c_device == -1) {
        return -1;
    }

    i2c_address = BME68X_I2C_ADDR_LOW;
    if (ioctl(i2c_device, I2C_SLAVE, BME68X_I2C_ADDR_LOW) < 0) {
        printf("ioctl error");
    }

    int8_t rslt = BME68X_OK;

    bme_api_dev.read = bme68x_i2c_read;
    bme_api_dev.write = bme68x_i2c_write;
    bme_api_dev.intf = BME68X_I2C_INTF;
    bme_api_dev.delay_us = bme68x_delay_us;
    bme_api_dev.intf_ptr = &i2c_address;
    bme_api_dev.amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */

    rslt = bme68x_init(&bme_api_dev);
    bme68x_check_rslt("bme68x_init",rslt);

    rslt = bme680_set_mode_forced();//default mode, can be overridden by bme688_set_mode()

    return (int)rslt;
}

int bme680_set_mode_forced(){
    int8_t rslt = BME68X_OK;

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf,&bme_api_dev);
    bme68x_check_rslt("bme68x_set_conf",rslt);

    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf,&bme_api_dev);
    bme68x_check_rslt("bme68x_set_heatr_conf",rslt);

    return (int)rslt;
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
 */
void i2c_hal_free(void) {
    if (i2c_device >= 0) {
        close(i2c_device);
        i2c_device = -1;
        i2c_address = 0;
    }
}


int bme680_self_test() {
    int8_t rslt;

    rslt = bme68x_selftest_check(&bme_api_dev);
    bme68x_check_rslt("bme68x_selftest_check", rslt);

    if (rslt == BME68X_OK)
    {
        printf("Self-test passed\n");
    }

    if (rslt == BME68X_E_SELF_TEST)
    {
        printf("Self-test failed\n");
    }

    return (int)rslt;
}


int bme680_get_measure(float* t, float* p, float* h) {
    int8_t rslt;

    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme_api_dev);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);

    /* Calculate delay period in microseconds */
    uint32_t del_period;
    del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme_api_dev) + (heatr_conf.heatr_dur * 1000);
    bme_api_dev.delay_us(del_period, bme_api_dev.intf_ptr);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    uint8_t n_fields;
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme_api_dev);
    bme68x_check_rslt("bme68x_get_data", rslt);

    if (n_fields) {
        *t = data->temperature;
        *p = data->pressure;
        *h = data->humidity;

        printf("Data received from BME680: %f °C, %f Pa, %f %\n", data->temperature, data->pressure, data->humidity);
    }

    return (int)rslt;
}
