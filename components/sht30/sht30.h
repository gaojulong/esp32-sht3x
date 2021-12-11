#ifndef _SHT30_H_
#define _SHT30_H_
#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/i2c.h"

#define DELAY_TIME_BETWEEN_ITEMS_MS 1000

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO 5                         /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 4                         /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(0)                /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 10000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */

#define SHT30_SENSOR_ADDR 0x44
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

esp_err_t i2c_master_init(void);
void i2c_sht30_task(void *arg);

#endif