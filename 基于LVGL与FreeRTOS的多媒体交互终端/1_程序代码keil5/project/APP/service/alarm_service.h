#ifndef __ALARM_MANAGER_H
#define __ALARM_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ALARMS 20






typedef struct __attribute__((packed)) 
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
	bool enable;
    uint32_t total_seconds; 
} time_alarm_t;

// --- 闹钟配置/数据属性结构体 ---
typedef struct 
{
    time_alarm_t alarms[MAX_ALARMS];
    int count;
} alarm_config_t;



/**
 * @brief  单宏实现闹钟全量信息打印
 * @note   移除冗余索引，实现纯粹的代码路径式打印
 */
#define DUMP_ALARM_CONFIG(cfg) do { \
    printf("\r\n----------- [DEBUG] Alarm Table: <%s> ----------\n", #cfg); \
    printf("%s.count = %d\n", #cfg, (cfg).count); \
    for (int i = 0; i < (cfg).count; i++) { \
        printf("%s.alarms[%d].enable        = %s\n", #cfg, i, (cfg).alarms[i].enable ? "TRUE" : "FALSE"); \
        printf("%s.alarms[%d].time          = %02d:%02d:%02d\n", #cfg, i, \
                (cfg).alarms[i].hour, (cfg).alarms[i].minute, (cfg).alarms[i].second); \
        printf("%s.alarms[%d].total_seconds = %u\n", #cfg, i, (cfg).alarms[i].total_seconds); \
        if(i < (cfg).count - 1) printf("    ---\n"); \
    } \
    printf("--------------------------------------------------\r\n"); \
} while(0)

// 调用示例：
// DUMP_ALARM_CONFIG(g_alarm_cfg);

// 使用方法：
// DUMP_ALARM_CONFIG(g_alarm_cfg);



// --- 闹钟管理器结构体 ---
typedef struct 
{
    // 嵌套数据属性
    alarm_config_t config;

    // --- 接口映射 ---
    int (*get_count)(void);
    time_alarm_t (*get_alarm)(int index);
    bool (*add)(uint8_t h, uint8_t m, uint8_t s);   
    void (*enable)(int index, bool status);
    bool (*del)(int index);
    bool (*check)(uint8_t h, uint8_t m, uint8_t s);   
    
    void (*init)(void);
} alarm_control_t;

// 外部声明全局实例
extern alarm_control_t AlarmControl;

#endif
