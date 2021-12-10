#include "sht30.h"



static const char *TAG = "SHT30";

const uint8_t g_polynom = 0x31;
static uint8_t crc8 (uint8_t data[], int len)
{
    uint8_t crc = 0xff;
    for (int i=0; i < len; i++)
    {
        crc ^= data[i];  
    
        for (int i = 0; i < 8; i++)
        {
            bool xor = crc & 0x80;
            crc = crc << 1;
            crc = xor ? crc ^ g_polynom : crc;
        }
    }
    return crc;
} 

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
// static esp_err_t i2c_master_sensor_sht30_init(i2c_port_t i2c_num)
// {
//     int ret;
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, SHT30_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_DIS);
//     i2c_master_write_byte(cmd, 0xF3, ACK_CHECK_DIS);
//     i2c_master_write_byte(cmd, 0x2D, ACK_CHECK_DIS);
//     i2c_master_stop(cmd);
//     ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
//     i2c_cmd_link_delete(cmd);
//     if (ret != ESP_OK) {
//         return ret;
//     }
//     vTaskDelay(30 / portTICK_RATE_MS);
//     return ret;
// }

static esp_err_t i2c_master_sensor_sht30_init(i2c_port_t i2c_num)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT30_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_DIS);
    i2c_master_write_byte(cmd, 0xF3, ACK_CHECK_DIS);
    i2c_master_write_byte(cmd, 0x2D, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }
    vTaskDelay(30 / portTICK_RATE_MS);

    uint8_t data[3];
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT30_SENSOR_ADDR<< 1 | READ_BIT, ACK_CHECK_DIS);
    i2c_master_read(cmd, &data[0], sizeof(data), ACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(50 / portTICK_RATE_MS);
    for (uint8_t i = 0; i < sizeof(data); i++)
    {
        printf("status:=%d:%d\n",i,data[i]);
    }
    if (crc8(data,2) != data[2])
    {
        ESP_LOGE(TAG,"CRC check for failed\n");
        return ret;
    }
    return ret;
}

static esp_err_t i2c_master_sensor_sht30_read(i2c_port_t i2c_num, float *temp_data, float *hum_data)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT30_SENSOR_ADDR << 1 | WRITE_BIT, ACK_CHECK_DIS);
    i2c_master_write_byte(cmd, 0x2C, ACK_CHECK_DIS);
    i2c_master_write_byte(cmd, 0x06, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }
    vTaskDelay(30 / portTICK_RATE_MS);
    uint8_t data[6];
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SHT30_SENSOR_ADDR<< 1 | READ_BIT, ACK_CHECK_DIS);
    i2c_master_read(cmd, &data[0], sizeof(data), ACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(50 / portTICK_RATE_MS);
    if (crc8(data,2) != data[2])
    {
        ESP_LOGE(TAG,"CRC check for temp data failed");
        return ret;
    }
    // check humidity crc
    if (crc8(data+3,2) != data[5])
    {
        ESP_LOGE(TAG,"CRC check for humidity data failed");
        return ret;
    }
    *temp_data =((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
	*hum_data = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);
    return ret;
}

void i2c_sht30_task(void *arg)
{
    int ret;
    uint32_t task_idx = (uint32_t)arg;
    float temp_data, hum_data;
    
    int cnt = 0;
    while (1) {
        vTaskDelay(1000/ portTICK_RATE_MS);
        ESP_LOGI(TAG, "TASK[%d] test cnt: %d", task_idx, cnt++);
        ret = i2c_master_sensor_sht30_read(I2C_MASTER_NUM, &temp_data, &hum_data);
        if (ret == ESP_ERR_TIMEOUT) {
            ESP_LOGE(TAG, "I2C Timeout");
        } else if (ret == ESP_OK) {
            printf("*******************\n");
            printf("TASK[%d]  MASTER READ SENSOR( SHT30 )\n", task_idx);
            printf("*******************\n");
            printf("temp_data: %.01f\n", temp_data);
            printf("hum_data: %.01f\n", hum_data);
        } else {
            ESP_LOGW(TAG, "%s: No ack, sensor not connected...skip...", esp_err_to_name(ret));
        }
        vTaskDelay((DELAY_TIME_BETWEEN_ITEMS_MS * (task_idx + 1)) / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}
