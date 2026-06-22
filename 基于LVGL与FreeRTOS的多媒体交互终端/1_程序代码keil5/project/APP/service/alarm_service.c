#include "alarm_service.h"
#include "at24cxx_data.h"
#include "../../system/debug/debug.h"
#include <stdio.h>

// 闹钟默认配置宏：初始计数为0，数组清零
#define ALARM_CONFIG_DEFAULT { \
    .count = 0,                \
    .alarms = {0}              \
}


// 内部辅助函数：时间转秒数
static uint32_t alarm_manager_to_sec(uint8_t h, uint8_t m, uint8_t s) {
    return (uint32_t)h * 3600UL + (uint32_t)m * 60UL + (uint32_t)s;
}

// 1. 获取当前有效闹钟数量
static int alarm_manager_get_count(void) {
    if(AlarmControl.config.count > MAX_ALARMS) {
        return MAX_ALARMS;
    }
    return AlarmControl.config.count;
}

// 2. 获取指定索引的闹钟数据
static time_alarm_t alarm_manager_get_alarm(int index) {
    if (index >= 0 && index < AlarmControl.config.count) {
        return AlarmControl.config.alarms[index];
    }
    return (time_alarm_t){0};
}

// 3. 添加闹钟：保持升序排列
static bool alarm_manager_add(uint8_t h, uint8_t m, uint8_t s) {
    if (AlarmControl.config.count >= MAX_ALARMS) return false;

    uint32_t new_total = alarm_manager_to_sec(h, m, s);
    int pos = 0;
    
    // 寻找插入位置
    while (pos < AlarmControl.config.count && 
           AlarmControl.config.alarms[pos].total_seconds < new_total) {
        pos++;
    }

    // 重复性检查
    if (pos < AlarmControl.config.count && 
        AlarmControl.config.alarms[pos].total_seconds == new_total) {
        return false;
    }

    // 后移腾出空间
    for (int i = AlarmControl.config.count; i > pos; i--) {
        AlarmControl.config.alarms[i] = AlarmControl.config.alarms[i - 1];
    }

    // 赋值
    AlarmControl.config.alarms[pos] = (time_alarm_t){
        .hour = h, 
        .minute = m, 
        .second = s, 
        .total_seconds = new_total, 
        .enable = false
    };
    AlarmControl.config.count++;
    
    return true;
}

// 4. 检查某个时间点是否已有闹钟
static bool alarm_manager_check(uint8_t h, uint8_t m, uint8_t s) {
    uint32_t target = alarm_manager_to_sec(h, m, s);
    for (int i = 0; i < AlarmControl.config.count; i++) {
        if (AlarmControl.config.alarms[i].total_seconds == target) return false;
        if (AlarmControl.config.alarms[i].total_seconds > target) break; 
    }
    return true;
}

// 5. 启用或禁用闹钟
static void alarm_manager_enable(int index, bool status) {
    if (index >= 0 && index < AlarmControl.config.count) {
        AlarmControl.config.alarms[index].enable = status;
    }
}

// 6. 删除指定索引的闹钟
static bool alarm_manager_del(int index) {
    if (index < 0 || index >= AlarmControl.config.count) return false;
    
    for (int i = index; i < AlarmControl.config.count - 1; i++) {
        AlarmControl.config.alarms[i] = AlarmControl.config.alarms[i + 1];
    }
    
    AlarmControl.config.count--;
    AlarmControl.config.alarms[AlarmControl.config.count] = (time_alarm_t){0};
    return true;
}

/**
 * @brief 闹钟管理器数据初始化
 */
static void alarm_manager_init_data(void) 
{
    // 1. 尝试从 EEPROM 加载数据
    uint8_t res = at24cxx_get_alarm_manager_info(&AlarmControl.config);
    
    if (res == 0) 
	{
        printf("[info] AlarmControl: Restore Success\r\n");
		DUMP_ALARM_CONFIG(AlarmControl.config);
    } 
    else 
	{
        // 2. 读取失败（魔数错误或CRC不匹配），使用宏一键初始化
		printf("[info] AlarmControl: Restore defaults\r\n");
        AlarmControl.config = (alarm_config_t)ALARM_CONFIG_DEFAULT;
		DUMP_ALARM_CONFIG(AlarmControl.config);
    }
}



/* --- 全局单例初始化 --- */
alarm_control_t AlarmControl = 
{
    // 初始值引用宏，保持代码简洁一致
    .config = ALARM_CONFIG_DEFAULT,

    // 绑定函数指针接口
    .get_count   = alarm_manager_get_count,
    .get_alarm   = alarm_manager_get_alarm,
    .add         = alarm_manager_add,
    .enable      = alarm_manager_enable,
    .del         = alarm_manager_del,
    .check       = alarm_manager_check,
    
    .init       = alarm_manager_init_data,
};

