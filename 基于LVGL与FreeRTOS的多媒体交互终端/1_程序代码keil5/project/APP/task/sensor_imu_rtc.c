#include "freertos_task.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"



void vRTCTask(void *pvParameters) 
{
	sensor_rtc_data_t rtc_env;

    while(1) 
	{
        // 获取硬件数据
        rtc_get_date(&rtc_env.year, &rtc_env.month, &rtc_env.day, &rtc_env.week);
        rtc_get_time(&rtc_env.hour, &rtc_env.minute, &rtc_env.second, &rtc_env.ap);

        // 推送给 SensorHub
        SensorHub.push(SENSOR_RTC, &rtc_env);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void vSensorTask(void *pvParameters) 
{
    // 1. 初始化传感器采集时间戳 
    static TickType_t last_env_time = 0;
    static TickType_t last_bat_time = 0;

    // 2. 定义本任务负责的传感器掩码（仅关注温湿度和电池） 
    const uint32_t my_sensors = SENSOR_ENV | SENSOR_BATTERY;

    // 3. 初始数据推送 确保 UI 启动时立即获得有效快照，无需等待循环 
    {
        float t = 0.0f, h = 0.0f; 
        if (sensor_get_temperature_humidity(&t, &h) == 0) 
        {
            sensor_env_data_t env_init = { .temperature = t, .humidity = h };
            SensorHub.push(SENSOR_ENV, &env_init);
        }

        sensor_battery_data_t bat_init;
        bat_init.voltage = max17048_get_voltage();
        bat_init.percentage = max17048_get_quantity();
        bat_init.is_charging = max17048_is_charging();
        SensorHub.push(SENSOR_BATTERY, &bat_init);

        last_env_time = xTaskGetTickCount();
        last_bat_time = last_env_time;
    }

    while(1) 
    {
        // 4. 同步状态：内部自动处理休眠、唤醒、及 STOP 信号的位清理 
        // 如果 my_sensors 中没有任何传感器被订阅，此函数将 portMAX_DELAY 阻塞 
        // 如果有订阅，则以 1000ms 为步进进行轮询检查 
        uint32_t active_bits = SensorHub.sync_status(my_sensors, pdMS_TO_TICKS(1000)); 

        // 5. 环境温湿度业务逻辑 (SENSOR_ENV) 
        if (active_bits & SENSOR_ENV) 
        {
            TickType_t xNow = xTaskGetTickCount();
            if ((xNow - last_env_time) >= pdMS_TO_TICKS(1000)) 
            {
                last_env_time = xNow;
                float t = 0.0f, h = 0.0f; 
                if (sensor_get_temperature_humidity(&t, &h) == 0) 
                {
                    sensor_env_data_t env_data = { .temperature = t, .humidity = h };
                    SensorHub.push(SENSOR_ENV, &env_data);
                }
            }
        }

        // 6. 电池业务逻辑 (SENSOR_BATTERY) 
        if (active_bits & SENSOR_BATTERY) 
        {
            TickType_t xNow = xTaskGetTickCount();
            if ((xNow - last_bat_time) >= pdMS_TO_TICKS(1000)) 
            {
                last_bat_time = xNow;
                sensor_battery_data_t bat_data;
				bat_data.has_bat = max17048_has_bat();
				if(bat_data.has_bat){
					bat_data.voltage = max17048_get_voltage();
					bat_data.percentage = max17048_get_quantity();
					bat_data.is_charging = max17048_is_charging();
				}
                SensorHub.push(SENSOR_BATTERY, &bat_data);
            }
        }
    }
}




void vIMUTask(void *pvParameters)
{
    sensor_imu_data_t imu_data;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // IMU 采样周期：20ms (50Hz) 保证 UI 水平仪丝滑 
    const TickType_t xFrequency = pdMS_TO_TICKS(20); 

    // 定义本任务负责的传感器 
    const uint32_t my_sensors = SENSOR_IMU;

    while (1) 
    {
        // 1. 同步 IMU 状态 
        // 如果无人订阅 SENSOR_IMU，此处将 portMAX_DELAY 阻塞，不消耗任何 CPU 
        // 如果有人订阅，则以 20ms 为步进进行状态检查（或根据需要调整轮询步进） 
        uint32_t active_bits = SensorHub.sync_status(my_sensors, 20);

        if (active_bits & SENSOR_IMU) 
        {
            // 2. 读取数据并推送 
            float p, r, y;
            if (sensor_get_imu(&p, &r, &y) == 0) // 假设 0 为读取成功 
            {
                imu_data.pitch = p;
                imu_data.roll = r;
                imu_data.yaw = y;
                imu_data.timestamp = xTaskGetTickCount(); // 记录时间戳 
				
                // 3. 推送到 SensorHub 
                SensorHub.push(SENSOR_IMU, &imu_data);
            }

            // 4. 保持稳定的 50Hz 采样频率 
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        }
        else 
        {
            // 如果 sync_status 返回 0（说明刚收到 STOP 信号或无订阅） 
            // 下一轮循环 sync_status 会自动进入 portMAX_DELAY 
            // 此处重置 LastWakeTime，防止醒来后瞬间追赶缺失的 Tick 
            xLastWakeTime = xTaskGetTickCount();
        }
    }
}

