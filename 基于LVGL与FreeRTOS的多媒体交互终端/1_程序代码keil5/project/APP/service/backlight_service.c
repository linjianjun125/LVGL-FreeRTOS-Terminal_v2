#include "backlight_service.h"
#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"

// 内部私有互斥锁，保护影子寄存器数据安全
static SemaphoreHandle_t xBlMutex = NULL;

// 硬件真实运行配置 (真相来源/运行时变量)
backlight_config_t g_active_bl_cfg ;//= {80, 10, 100, false, 300, 0, false};
#define BACKLIGHT_CONFIG_DEFAULT { \
    .target_level = 50,            \
    .min_level = 30,               \
    .max_level = 100,              \
    .is_turnoff_screen = false,    \
    .timeout_100ms = 300,          \
    .timer_cnt = 0,                \
    .auto_adjust = false,          \
    .reserved = {0}                \
}


// 影子配置 (交换缓冲区)
// UI 任务写入此变量，Backlight 任务通过同步接口读取
static backlight_config_t g_shadow_bl_cfg;

// 初始化服务资源
static void _init(void) 
{
    if (xBlMutex == NULL) 
    {
        xBlMutex = xSemaphoreCreateMutex();
    }
	
    uint8_t res = at24cxx_get_backlight_manager_info(&g_active_bl_cfg);
    if (res == 0) 
	{
        printf("[info] BacklightControl: Restore Success\r\n");
		PRINT_BACKLIGHT_CFG(g_active_bl_cfg);
    } else 
	{
        // 读取失败则加载宏定义的默认值
        printf("[error] BacklightControl: Restore defaults\n");
        g_active_bl_cfg = (backlight_config_t)BACKLIGHT_CONFIG_DEFAULT;
		PRINT_BACKLIGHT_CFG(g_active_bl_cfg);
    }
	
	memcpy(&g_shadow_bl_cfg, &g_active_bl_cfg, sizeof(backlight_config_t));
}


// 封装 RTOS 指令等待逻辑：隐藏 xTaskNotifyWait 的复杂性
static bool _wait_command(uint32_t* cmd_out, uint32_t timeout) 
{
    // 接收通知并清空位标志
    return (xTaskNotifyWait(0, 0xFFFFFFFF, cmd_out, timeout) == pdTRUE);
}

// 内部同步：将 UI 提交的影子变量拷贝到运行变量中
static void _sync_from_shadow(backlight_config_t* dest) 
{
    if (xSemaphoreTake(xBlMutex, pdMS_TO_TICKS(10)) == pdTRUE) 
    {
        memcpy(dest, &g_shadow_bl_cfg, sizeof(backlight_config_t));
        xSemaphoreGive(xBlMutex);
    }
}

// API: UI 设置背光全量参数
static void _set_all(backlight_config_t* cfg) 
{
    if (cfg == NULL || xBlMutex == NULL) return;

    if (xSemaphoreTake(xBlMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        memcpy(&g_shadow_bl_cfg, cfg, sizeof(backlight_config_t));
        xSemaphoreGive(xBlMutex);
        
        // 发送通知，唤醒后台任务执行
        if (xBacklightTaskHandle) 
        {
            xTaskNotify(xBacklightTaskHandle, BL_CMD_SET_ALL, eSetBits);
        }
    }
}

static void _set_target_level(uint8_t level) 
{
    if (xBlMutex == NULL) return;

    if (xSemaphoreTake(xBlMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        // 自动限幅：确保目标亮度在硬件/配置允许的范围内
        if (level < g_shadow_bl_cfg.min_level) level = g_shadow_bl_cfg.min_level;
        if (level > g_shadow_bl_cfg.max_level) level = g_shadow_bl_cfg.max_level;

        g_shadow_bl_cfg.target_level = level;
        xSemaphoreGive(xBlMutex);
        
        if (xBacklightTaskHandle) 
        {
            xTaskNotify(xBacklightTaskHandle, BL_CMD_SET_LEVEL, eSetBits);
        }
    }
}

static void _set_level_range(uint8_t min, uint8_t max) 
{
    if (xBlMutex == NULL) return;

    if (xSemaphoreTake(xBlMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        g_shadow_bl_cfg.min_level = min;
        g_shadow_bl_cfg.max_level = max;

        // 如果当前亮度超出了新范围，自动纠正影子变量中的目标亮度
        if (g_shadow_bl_cfg.target_level < min) g_shadow_bl_cfg.target_level = min;
        if (g_shadow_bl_cfg.target_level > max) g_shadow_bl_cfg.target_level = max;

        xSemaphoreGive(xBlMutex);
        
        if (xBacklightTaskHandle) 
        {
            xTaskNotify(xBacklightTaskHandle, BL_CMD_SET_RANGE, eSetBits);
        }
    }
}

static void _set_timeout(uint32_t timeout_100ms) 
{
    if (xBlMutex == NULL) return;

    if (xSemaphoreTake(xBlMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        g_shadow_bl_cfg.timeout_100ms = timeout_100ms;
        // 修改超时时间后，通常建议重置当前的计时累加器
        g_shadow_bl_cfg.timer_cnt = 0; 
		//printf("_set_timeout=%d\n", timeout_100ms);
        xSemaphoreGive(xBlMutex);
        
        if (xBacklightTaskHandle) 
        {
            xTaskNotify(xBacklightTaskHandle, BL_CMD_SET_TIMEOUT, eSetBits);
        }
    }
}


static void _set_auto_adjust(bool enable) 
{
    if (xBlMutex && xSemaphoreTake(xBlMutex, pdMS_TO_TICKS(20)) == pdTRUE) 
    {
        g_shadow_bl_cfg.auto_adjust = enable;
        xSemaphoreGive(xBlMutex);
        
        if (xBacklightTaskHandle) 
        {
            xTaskNotify(xBacklightTaskHandle, BL_CMD_SET_AUTO, eSetBits);
        }
    }
}

// API: 外部触发唤醒（如触摸屏按下或物理按键）
static void _wakeup(void) 
{
    if (xBacklightTaskHandle) 
    {
        xTaskNotify(xBacklightTaskHandle, BL_CMD_WAKEUP, eSetBits);
    }
}

// 接口单例实例化
backlight_control_t BacklightControl = 
{
    .init = _init,
    .set_all = _set_all,
	.set_target  = _set_target_level,
    .set_range   = _set_level_range,
    .set_timeout = _set_timeout,
	.set_auto_adjust_status = _set_auto_adjust,
    .wakeup = _wakeup,
    .sync_from_shadow = _sync_from_shadow,
    .wait_command = _wait_command
};

