#include "volume_service.h"
#include "semphr.h"
#include "at24cxx_data.h"

TaskHandle_t vVolumeTaskHandle = NULL;
static SemaphoreHandle_t xVolMutex = NULL;

// 实际运行配置：硬件当前状态
volume_config_t g_active_vol_cfg ;// = {40, 0, 100, 5};

#define VOLUME_CONFIG_DEFAULT { \
    .current_vol = 20,          \
    .min_vol     = 0,           \
    .max_vol     = 100,         \
    .step        = 5,           \
    .reserved    = {0}          \
}



// 影子配置：UI 提交的意图缓冲区
static volume_config_t g_shadow_vol_cfg;

static void _init(void) 
{
    if (xVolMutex == NULL) xVolMutex = xSemaphoreCreateMutex();
	
    // 尝试从 EEPROM 加载音量配置
    uint8_t res = at24cxx_get_volume_manager_info(&g_active_vol_cfg);
    if (res == 0) 
	{
        printf("[info] VolumeControl: Restore Success\r\n");
		PRINT_VOL_CFG(g_active_vol_cfg);
    } else 
	{
        // 读取失败则加载宏定义的默认值
        printf("[error] VolumeControl: Restore defaults\n");
        g_active_vol_cfg = (volume_config_t)VOLUME_CONFIG_DEFAULT;
		PRINT_VOL_CFG(g_active_vol_cfg);
    }
	
	memcpy(&g_shadow_vol_cfg, &g_active_vol_cfg, sizeof(volume_config_t));
	
}

static bool _wait_command(uint32_t* cmd_out, uint32_t timeout) 
{
    return (xTaskNotifyWait(0, 0xFFFFFFFF, cmd_out, timeout) == pdTRUE);
}

static void _sync_from_shadow(volume_config_t* dest) 
{
    if (xSemaphoreTake(xVolMutex, pdMS_TO_TICKS(10)) == pdTRUE) 
    {
        memcpy(dest, &g_shadow_vol_cfg, sizeof(volume_config_t));
        xSemaphoreGive(xVolMutex);
    }
}

static void _set_all(volume_config_t* cfg) 
{
    if (xSemaphoreTake(xVolMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        memcpy(&g_shadow_vol_cfg, cfg, sizeof(volume_config_t));
        xSemaphoreGive(xVolMutex);
        if (vVolumeTaskHandle) xTaskNotify(vVolumeTaskHandle, VOL_CMD_SET_ALL, eSetBits);
    }
}


static void _set_volume(uint8_t vol) 
{
    if (xSemaphoreTake(xVolMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        // 自动限幅：确保目标音量在最小和最大值之间
        if (vol < g_shadow_vol_cfg.min_vol) vol = g_shadow_vol_cfg.min_vol;
        if (vol > g_shadow_vol_cfg.max_vol) vol = g_shadow_vol_cfg.max_vol;

        g_shadow_vol_cfg.current_vol = vol;
        xSemaphoreGive(xVolMutex);
        
        // 发送通知给处理任务
        if (vVolumeTaskHandle) xTaskNotify(vVolumeTaskHandle, VOL_CMD_SET_VOL, eSetBits);
    }
}

static void _set_vol_range(uint8_t min, uint8_t max) 
{
    if (xSemaphoreTake(xVolMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        g_shadow_vol_cfg.min_vol = min;
        g_shadow_vol_cfg.max_vol = max;
        
        // 如果当前音量超出了新范围，同步调整当前音量
        if (g_shadow_vol_cfg.current_vol < min) g_shadow_vol_cfg.current_vol = min;
        if (g_shadow_vol_cfg.current_vol > max) g_shadow_vol_cfg.current_vol = max;

        xSemaphoreGive(xVolMutex);
        
        if (vVolumeTaskHandle) xTaskNotify(vVolumeTaskHandle, VOL_CMD_SET_RANGE, eSetBits);
    }
}

static void _set_vol_step(uint8_t step) 
{
    if (xSemaphoreTake(xVolMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        // 步进值通常不应为 0，否则会导致调节无效
        g_shadow_vol_cfg.step = (step > 0) ? step : 1;
        
        xSemaphoreGive(xVolMutex);
        
        if (vVolumeTaskHandle) xTaskNotify(vVolumeTaskHandle, VOL_CMD_SET_STEP, eSetBits);
    }
}

static void _step_up(void) 
{
	if (vVolumeTaskHandle) 
	{
        xTaskNotify(vVolumeTaskHandle, VOL_CMD_UP, eSetBits);
    } 
	else 
	{
        printf("Error: vVolumeTaskHandle is NULL!\n");
    }
}

static void _step_down(void) 
{
	if (vVolumeTaskHandle) 
	{
        xTaskNotify(vVolumeTaskHandle, VOL_CMD_DOWN, eSetBits);
    } 
	else 
	{
        printf("Error: vVolumeTaskHandle is NULL!\n");
    }
}

volume_control_t VolumeControl = 
{
    .init = _init,
    .set_all = _set_all,
	.set_volume = _set_volume,
    .set_range   = _set_vol_range,
    .set_step    = _set_vol_step,
    .step_up = _step_up,
    .step_down = _step_down,
    .sync_from_shadow = _sync_from_shadow,
    .wait_command = _wait_command
};

