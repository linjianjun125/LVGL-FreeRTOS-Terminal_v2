#ifndef BACKLIGHT_SERVICE_H
#define BACKLIGHT_SERVICE_H

#include "sensor_hub.h"
#include <stdbool.h>
#include <string.h>

// 通用限幅宏：确保 x 在 [min, max] 范围内，防止硬件溢出
#ifndef CLAMP
#define CLAMP(x, min, max)  ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))
#endif

// 亮度控制指令枚举（位掩码形式）
typedef enum 
{
    BL_CMD_NONE        = 0,
    BL_CMD_SET_ALL     = (1 << 0), // 全量更新
    BL_CMD_WAKEUP      = (1 << 1), // 唤醒/重置计时
    BL_CMD_SET_LEVEL   = (1 << 2), // 仅修改亮度等级
    BL_CMD_SET_RANGE   = (1 << 3), // 仅修改亮度范围
    BL_CMD_SET_TIMEOUT = (1 << 4),  // 仅修改超时时间
	BL_CMD_SET_AUTO    = (1 << 5),  // 设置自动调节模式
} backlight_cmd_t;

// 屏幕亮度配置结构体
// 使用 packed 确保内存布局紧凑，方便后续 EEPROM 存储
typedef struct __attribute__((packed)) 
{
    uint8_t  target_level;      // 目标亮度 (0-100)
    uint8_t  min_level;         // 亮度下限
    uint8_t  max_level;         // 亮度上限
    bool     is_turnoff_screen; // 当前是否处于息屏状态
    uint32_t timeout_100ms;     // 自动熄屏触发时间 (单位100ms)，0：永不息屏
    uint16_t timer_cnt;         // 自动熄屏计时累加器
    bool     auto_adjust;       // 自动调节模式开关（预留）
    uint8_t  reserved[1];       // 填充字节，保持结构体对齐
} backlight_config_t;
/**
 * @brief 打印背光结构体成员信息
 * @note  针对布尔型变量进行了字符串化处理，提升 Log 可读性
 */
#define PRINT_BACKLIGHT_CFG(cfg) do { \
    printf("\r\n--------------- <%s> ---------------\n", #cfg); \
    printf("%s.target_level      = %u\n", #cfg, (cfg).target_level); \
    printf("%s.min_level         = %u\n", #cfg, (cfg).min_level); \
    printf("%s.max_level         = %u\n", #cfg, (cfg).max_level); \
    printf("%s.is_turnoff_screen = %s\n", #cfg, (cfg).is_turnoff_screen ? "TRUE" : "FALSE"); \
    printf("%s.timeout_100ms     = %u\n", #cfg, (cfg).timeout_100ms); \
    printf("%s.timer_cnt         = %u\n", #cfg, (cfg).timer_cnt); \
    printf("%s.auto_adjust       = %s\n", #cfg, (cfg).auto_adjust ? "ENABLE" : "DISABLE"); \
    printf("---------------------------------------------------\r\n"); \
} while(0)



// 统一 API 接口结构体
typedef struct 
{
    // 初始化服务资源（如互斥锁）
    void (*init)(void);
    // UI 调用：提交新的配置意图
    void (*set_all)(backlight_config_t* cfg);
    void (*set_target)(uint8_t level);
    void (*set_range)(uint8_t min, uint8_t max);
    void (*set_timeout)(uint32_t timeout_100ms);
	void (*set_auto_adjust_status)(bool enable);
	
    // 外部唤醒：触摸屏或按键触发亮屏并重置计时
    void (*wakeup)(void); 

    // --- 供后台任务 vBacklightTask 调用的私有接口 --- 
    // 从影子变量同步最新意图到运行变量
    void (*sync_from_shadow)(backlight_config_t* dest);
    // 阻塞等待指令通知
    bool (*wait_command)(uint32_t* cmd_out, uint32_t timeout);
} backlight_control_t;

// 暴露接口单例
extern backlight_control_t BacklightControl;

// 暴露运行时配置，供逻辑计算任务使用
extern backlight_config_t g_active_bl_cfg;

#endif // BACKLIGHT_SERVICE_H


