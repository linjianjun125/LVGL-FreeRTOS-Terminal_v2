#ifndef SENSOR_HUB_H
#define SENSOR_HUB_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// 
// 统一数据缓冲区大小 
#define SENSOR_DATA_BUF_SIZE  32

// 
// 每个 ID 支持的最大并发订阅回调数 
#define SENSOR_SUB_MAX        8

// 
// 消息队列深度 
#define SENSOR_HUB_QLEN       32

// 
// 最大支持的传感器数量 (0-15) 
#define SENSOR_COUNT_MAX      16

// 
// 辅助宏：将位掩码 ID 转换为数组索引 (0-15) 
#define GET_SENSOR_INDEX(id) ( \
    (id) == SENSOR_ENV            ? 0  : \
    (id) == SENSOR_IMU            ? 1  : \
    (id) == SENSOR_VOLUME         ? 2  : \
    (id) == SENSOR_KEY            ? 3  : \
    (id) == SENSOR_BACKLIGHT      ? 4  : \
    (id) == SENSOR_RTC            ? 5  : \
    (id) == SENSOR_BATTERY        ? 6  : \
    (id) == SENSOR_MUSIC_INFO     ? 7  : \
    (id) == SENSOR_WEATHER_DATA   ? 8  : \
    (id) == SENSOR_MUSIC_STATUS   ? 9  : \
    (id) == SENSOR_ID_10          ? 10 : \
    (id) == SENSOR_ID_11          ? 11 : \
    (id) == SENSOR_ID_12          ? 12 : \
    (id) == SENSOR_ID_13          ? 13 : \
    (id) == SENSOR_ID_14          ? 14 : \
    (id) == SENSOR_ID_15          ? 15 : 0 )

// 
// 传感器控制位操作宏 (低16位读，高16位停) 
#define SENSOR_OP_READ(id)   ((uint32_t)(id))           
#define SENSOR_OP_STOP(id)   ((uint32_t)(id) << 16)     

// 
// 传感器物理 ID：位掩码定义 (0x0001 - 0x8000) 
typedef enum 
{
    SENSOR_NONE           = 0,
    SENSOR_ENV            = 0x0001, // 位 0 
    SENSOR_IMU            = 0x0002, // 位 1 
    SENSOR_VOLUME         = 0x0004, // 位 2 
    SENSOR_KEY            = 0x0008, // 位 3 
    SENSOR_BACKLIGHT      = 0x0010, // 位 4 
    SENSOR_RTC            = 0x0020, // 位 5 
    SENSOR_BATTERY        = 0x0040, // 位 6 
    SENSOR_MUSIC_INFO     = 0x0080, // 位 7 
    SENSOR_WEATHER_DATA   = 0x0100, // 位 8 
    SENSOR_MUSIC_STATUS   = 0x0200, // 位 9 
    SENSOR_ID_10	      = 0x0400, // 位 10 
    SENSOR_ID_11          = 0x0800, // 位 11 
    SENSOR_ID_12          = 0x1000, // 位 12 
    SENSOR_ID_13          = 0x2000, // 位 13 
    SENSOR_ID_14          = 0x4000, // 位 14 
    SENSOR_ID_15          = 0x8000, // 位 15 
} sensor_id_t;

// 
// 温湿度环境数据结构 
typedef struct __attribute((packed))
{
    float temperature; // 温度 (°C)
    float humidity;    // 湿度 (%)
} sensor_env_data_t;

// 
// 电池传感器数据结构 
typedef struct __attribute((packed))
 {
    float voltage;      // 电压 (V)
    float percentage;   // 电量百分比 (0-100.0)
    bool is_charging;   // 充电状态
	bool has_bat;
} sensor_battery_data_t;

// 
// RTC 时间传感器数据结构 
typedef struct __attribute((packed))
 {
    uint8_t year;       
    uint8_t month;      
    uint8_t day;        
    uint8_t week;       
    uint8_t hour;       
    uint8_t minute;     
    uint8_t second;     
    uint8_t ap;         
} sensor_rtc_data_t;

// IMU 传感器数据结构 
typedef struct __attribute((packed))
{
    float pitch; float roll; float yaw; 
    struct __attribute((packed)) { float x, y, z; } accel;
    struct __attribute((packed)) { float x, y, z; } gyro;
    uint32_t timestamp; 
} sensor_imu_data_t;

// WIFI 天气数据结构 
typedef struct __attribute((packed))
{
    char info[8];
	int16_t temp;
} sensor_weather_data_t;

// 音乐播放 数据结构 
typedef struct __attribute((packed))
{
    uint8_t  status;			// 1:播放 2:暂停
	void*	music_info;			// 当前播放的音乐; NULL: 无效参数
	uint8_t current_percent;	// 播放进度0-100;  其他：无效参数  
} sensor_music_data_t;


// 传感器消息载荷结构 
typedef struct 
{
    sensor_id_t id;
    uint8_t data[SENSOR_DATA_BUF_SIZE]; // sensor_xxx_data_t
    uint32_t timestamp;
} sensor_hub_msg_t;

// 
// 订阅者回调函数原型 
typedef void (*sensor_hub_callback_t)(sensor_id_t id, const uint8_t* data);

// 
// 订阅者槽位结构 
typedef struct 
{
    sensor_hub_callback_t cb[SENSOR_SUB_MAX];
} sensor_observer_t;

// 
// 任务映射结构 
typedef struct {
    sensor_id_t id;
    TaskHandle_t *p_handle;
} sensor_task_map_t;

// 
// Sensor Hub 管理对象结构 
typedef struct 
{
    QueueHandle_t queue;
    uint8_t snapshot[SENSOR_COUNT_MAX][SENSOR_DATA_BUF_SIZE]; 
    TaskHandle_t task_handles[SENSOR_COUNT_MAX];
    
    bool (*init)(void);
	void (*clear)(void);
	void (*bind_task_handles)(void) ;
    bool (*push)(sensor_id_t id, const void* src_data);
    bool (*pop)(sensor_hub_msg_t* out_msg);
    void (*subscribe)(sensor_id_t id, sensor_hub_callback_t cb);
    void (*unsubscribe)(sensor_id_t id, sensor_hub_callback_t cb);
    void (*dispatch)(sensor_hub_msg_t* msg);
    uint32_t (*get_active_mask)(void);
    uint32_t (*sync_status)(uint32_t interest_mask, uint32_t poll_timeout) ;
} sensor_hub_t;

extern sensor_hub_t SensorHub;

#endif
