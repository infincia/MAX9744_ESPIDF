#include "sdkconfig.h"

#if defined(CONFIG_MAX9744_ESPIDF_ENABLED)
#include "max9744_espidf.h"

#include "esp_log.h"

#include "driver/i2c.h"

/**
 *
 * @brief I2C port configuration
 */
#define MAX9744_I2C_MASTER_SCL_IO CONFIG_MAX9744_ESPIDF_I2C_SCL_GPIO /*!< gpio number for I2C master clock */
#define MAX9744_I2C_MASTER_SDA_IO CONFIG_MAX9744_ESPIDF_I2C_SDA_GPIO /*!< gpio number for I2C master data  */
#define MAX9744_I2C_MASTER_NUM I2C_NUM_0 /*!< I2C port number for master dev */
#define MAX9744_I2C_MASTER_TX_BUF_DISABLE 0
#define MAX9744_I2C_MASTER_RX_BUF_DISABLE 0
#define MAX9744_I2C_MASTER_FREQ_HZ 100000


/**
 *
 * @brief MAX9744 command set details
 */
#define MAX9744_ADDR 0x4B /*!< Default address for MAX9744 */
#define MAX9744_WRITE_BIT I2C_MASTER_WRITE
#define MAX9744_ACK_CHECK_EN 0x1



static const char *TAG = "[max9744]";


/**
 * @brief i2c master initialization
 */

void max9744_init() {
    ESP_LOGV(TAG, "initializing I2C");

    i2c_port_t i2c_master_port = MAX9744_I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)MAX9744_I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)MAX9744_I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = MAX9744_I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port,
                       conf.mode,
                       MAX9744_I2C_MASTER_RX_BUF_DISABLE,
                       MAX9744_I2C_MASTER_TX_BUF_DISABLE,
                       0);

    ESP_LOGV(TAG, "initialization done");
}


/**
 *
 * @brief High level I2C handling
 */

static int max9744_send_command(uint8_t data) {
    ESP_LOGV(TAG, "sending command");

    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, MAX9744_ADDR << 1 | MAX9744_WRITE_BIT, MAX9744_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, MAX9744_ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(MAX9744_I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
        ESP_LOGV(TAG, "MAX9744 command succeeded");
    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "I2C timeout");
    } else {
        ESP_LOGE(TAG, "MAX9744 not connected: %d", ret);
    }
    return ret;
}

/**
 *
 * @brief High level MAX9744 commands
 */

int max9744_set_filterless_modulation() {
    ESP_LOGD(TAG, "set filterless modulation");

    uint8_t command = 0;
    command = command | (1 << 7);
    command = command | (0 << 1);
    return max9744_send_command(command);
}


int max9744_set_classic_pwm_modulation() {
    ESP_LOGD(TAG, "set classic pwm modulation");

    uint8_t command = 0;
    command = command | (1 << 7);
    command = command | (1 << 1);
    return max9744_send_command(command);
}


int max9744_increase_volume() {
    ESP_LOGD(TAG, "increase volume");

    uint8_t command = 0;
    command = command | (1 << 8);
    command = command | (1 << 7);
    command = command | (1 << 3);
    command = command | (0 << 1);
    return max9744_send_command(command);
}


int max9744_decrease_volume() {
    ESP_LOGD(TAG, "decrease volume");

    uint8_t command = 0;
    command = command | (1 << 8);
    command = command | (1 << 7);
    command = command | (1 << 3);
    command = command | (1 << 1);
    return max9744_send_command(command);
}


int max9744_set_volume(uint8_t volume) {
    ESP_LOGD(TAG, "set volume");

    // by a quirk of the MAX9744 command set, we can send the volume directly
    // because the 2 register selection bits are already zero in a uint8_t with
    // a range of only 0-63
    return max9744_send_command(volume);
}

#endif