#include "freertos_task.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"


/**
 * @brief 声音处理后台任务
 */
void vVolumeTask(void *pvParameters) 
{
    uint32_t cmd_bits;
    
	// 强制修正句柄指向当前任务
    extern TaskHandle_t vVolumeTaskHandle;
    vVolumeTaskHandle = xTaskGetCurrentTaskHandle();
	
    // 首次开机同步快照
    SensorHub.push(SENSOR_VOLUME, &g_active_vol_cfg);
	
	uint16_t hw_gain = ((uint16_t)g_active_vol_cfg.current_vol * 31) / 100;
	if (hw_gain > 31) hw_gain = 31;
	es8388_hpvol_set((uint8_t)hw_gain);
	es8388_spkvol_set((uint8_t)hw_gain);

    while (1) 
    {
        // 阻塞等待指令，完全不耗 CPU
        if (VolumeControl.wait_command(&cmd_bits, portMAX_DELAY)) 
        {
            // 1. 同步影子变量
			if (cmd_bits & (VOL_CMD_SET_ALL | VOL_CMD_SET_VOL | VOL_CMD_SET_RANGE | VOL_CMD_SET_STEP)) 
            {
                VolumeControl.sync_from_shadow(&g_active_vol_cfg);
				printf("Vol Set: %d\n", g_active_vol_cfg.current_vol);
            }
            
            // 2. 步进调节（加/减后统一限幅）
            if (cmd_bits & VOL_CMD_UP) 
            {
                g_active_vol_cfg.current_vol = 
                CLAMP(g_active_vol_cfg.current_vol + g_active_vol_cfg.step, 
                      g_active_vol_cfg.min_vol, g_active_vol_cfg.max_vol);
            }
            
            if (cmd_bits & VOL_CMD_DOWN) 
            {
                g_active_vol_cfg.current_vol = 
                CLAMP(g_active_vol_cfg.current_vol - g_active_vol_cfg.step, 
                      g_active_vol_cfg.min_vol, g_active_vol_cfg.max_vol);
            }

            // 3. 真正的硬件驱动调用（例如 I2C 写 Codec）
            // HAL_Audio_SetVolume(g_active_vol_cfg.current_vol);
			uint16_t hw_gain = ((uint16_t)g_active_vol_cfg.current_vol * 31) / 100;
			if (hw_gain > 31) hw_gain = 31;
			es8388_hpvol_set((uint8_t)hw_gain);
			es8388_spkvol_set((uint8_t)hw_gain);

            // 4. 数据回传：UI 订阅了 SENSOR_VOLUME 就会刷新
            SensorHub.push(SENSOR_VOLUME, &g_active_vol_cfg);
			printf("Vol Pushed: %d\n", g_active_vol_cfg.current_vol);
        }

		vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void set_backlight_value(void)
{
	// 执行硬件操作
	uint8_t final_pwm = g_active_bl_cfg.is_turnoff_screen ? 0 : 
						CLAMP(g_active_bl_cfg.target_level, g_active_bl_cfg.min_level, g_active_bl_cfg.max_level);
	lcdrgb_set_pwm_duty_cycle(final_pwm);
}

void vBacklightTask(void *pvParameters) 
{
    uint32_t cmd_bits;
    const TickType_t xTicks100ms = pdMS_TO_TICKS(100);

    SensorHub.push(SENSOR_BACKLIGHT, &g_active_bl_cfg);
	
	set_backlight_value();
	bool is_turnoff_screen = g_active_bl_cfg.is_turnoff_screen;

    while (1) 
    {
        // 1. 动态计算等待时间
        // 如果已息屏，则进入无限期休眠 (portMAX_DELAY)，直到收到控制信号
        // 如果亮屏且有超时设置，则 100ms 轮询一次处理计时
        uint32_t wait_ticks = g_active_bl_cfg.is_turnoff_screen ? portMAX_DELAY : xTicks100ms;

        // 2. 等待指令或超时
        if (BacklightControl.wait_command(&cmd_bits, wait_ticks)) 
        {
            if (cmd_bits & (BL_CMD_SET_ALL | BL_CMD_SET_LEVEL | BL_CMD_SET_RANGE | BL_CMD_SET_TIMEOUT | BL_CMD_SET_AUTO))
            {
                BacklightControl.sync_from_shadow(&g_active_bl_cfg);
                g_active_bl_cfg.timer_cnt = 0; 
				
				set_backlight_value();
				SensorHub.push(SENSOR_BACKLIGHT, &g_active_bl_cfg);
            }
			
            if (cmd_bits & BL_CMD_WAKEUP) 
            {
                g_active_bl_cfg.is_turnoff_screen = false;
                g_active_bl_cfg.timer_cnt = 0; 
            }
			//printf("vBacklightTask %d\n", g_active_bl_cfg.timeout_100ms);
        }
        else 
        {
            // 3. 超时处理：只有在亮屏且非永不熄屏(timeout > 0)时才计时
            if (!g_active_bl_cfg.is_turnoff_screen && g_active_bl_cfg.timeout_100ms > 0) 
            {
                g_active_bl_cfg.timer_cnt++;
                if (g_active_bl_cfg.timer_cnt >= g_active_bl_cfg.timeout_100ms) 
                {
                    g_active_bl_cfg.is_turnoff_screen = true;
                    // 此时进入下一轮循环，wait_ticks 将变为 portMAX_DELAY，任务自动挂起
                }
            }
        }
		
		// 防止频繁进入
		if(g_active_bl_cfg.is_turnoff_screen != is_turnoff_screen)
		{
			is_turnoff_screen = g_active_bl_cfg.is_turnoff_screen;
			set_backlight_value();
		}
    }
}


