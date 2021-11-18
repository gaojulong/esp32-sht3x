#include <stdio.h>
#include "sht30.h"

void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());

    //通过参数修改测量时间间隔n-1秒
    xTaskCreate(i2c_sht30_task, "i2c_sht30_task", 1024 * 2, (void *)10-1, 10, NULL);
}