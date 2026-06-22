#ifndef VOLUME_SERVICE_H
#define VOLUME_SERVICE_H

#include "sensor_hub.h"
#include <stdbool.h>

// 通用限幅宏
#define CLAMP(x, min, max)  ((x) > (max) ? (max) : ((x) < (min) ? (min) : (x)))

// 声音任务指令枚举（位掩码）
typedef enum 
{
    VOL_CMD_NONE      = 0,
    VOL_CMD_UP        = (1 << 0),  // 音量+
    VOL_CMD_DOWN      = (1 << 1),  // 音量-
    VOL_CMD_SET_ALL   = (1 << 2),  // 设置全部参数
    VOL_CMD_SET_VOL   = (1 << 3),  // 仅设置当前音量值
    VOL_CMD_SET_RANGE = (1 << 4),  // 仅设置最值范围
    VOL_CMD_SET_STEP  = (1 << 5)   // 仅设置步进
} volume_cmd_t;

// 声音配置结构体
typedef struct __attribute__((packed)) 
{
    uint8_t current_vol;    // 当前音量
    uint8_t min_vol;        // 最小限制
    uint8_t max_vol;        // 最大限制
    uint8_t step;           // 步进
    uint8_t reserved[4];    
} volume_config_t;

/**
 * @brief 打印音量结构体成员信息
 * @note  利用 #cfg 字符串化变量名，完美匹配你的变量命名规范
 */
#define PRINT_VOL_CFG(cfg) do { \
    printf("\r\n-------------- <%s> -------------\n", #cfg); \
    printf("%s.current_vol = %d\n", #cfg, (cfg).current_vol); \
    printf("%s.min_vol     = %d\n", #cfg, (cfg).min_vol); \
    printf("%s.max_vol     = %d\n", #cfg, (cfg).max_vol); \
    printf("%s.step        = %d\n", #cfg, (cfg).step); \
    printf("--------------------------------------------\r\n"); \
} while(0)

// 使用方法：
// PRINT_VOL_CFG(g_active_vol_cfg);


// 统一 API 接口
typedef struct 
{
    void (*init)(void);
    void (*set_all)(volume_config_t* cfg);
	void (*set_volume)(uint8_t vol);
    void (*set_range)(uint8_t min, uint8_t max);
    void (*set_step)(uint8_t step);
    void (*step_up)(void);
    void (*step_down)(void);
    
    // 供外部 Task 使用的私有封装
    void (*sync_from_shadow)(volume_config_t* dest);
    bool (*wait_command)(uint32_t* cmd_out, uint32_t timeout);
} volume_control_t;

extern volume_control_t VolumeControl;
extern TaskHandle_t xSoundTaskHandle;
extern volume_config_t g_active_vol_cfg; // 运行时配置

#endif

