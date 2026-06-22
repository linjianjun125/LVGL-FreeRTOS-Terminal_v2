#include "sensor_hub.h"
#include "../task/freertos_task.h"

extern TaskHandle_t xWifiAppaskHandle;
extern TaskHandle_t xWifiParserTaskHandle;
// 
// 内部观察者列表 
static sensor_observer_t observers[SENSOR_COUNT_MAX];

// 
// 全局活跃传感器掩码 (低16位表示对应 ID 是否有订阅) 
 volatile uint32_t global_active_mask = 0;

// 
// 静态任务映射表配置 
static const sensor_task_map_t g_sensor_task_map[] = 
{
    { SENSOR_ENV,            &xSensorTaskHandle },
    { SENSOR_BATTERY,        &xSensorTaskHandle },
    { SENSOR_RTC,            NULL },
    { SENSOR_VOLUME,         NULL },
    { SENSOR_BACKLIGHT,      NULL },
    { SENSOR_IMU,            &xIMUTaskHandle    },
    { SENSOR_MUSIC_INFO,     NULL},
	{ SENSOR_WEATHER_DATA,   &xWifiParserTaskHandle  },
	{ SENSOR_MUSIC_STATUS,	 NULL},
	
	
};
#define MAP_COUNT (sizeof(g_sensor_task_map) / sizeof(sensor_task_map_t))

// 实现：同步特定传感器的状态并自动清理取消位 
static uint32_t _sync_status(uint32_t interest_mask, uint32_t poll_timeout) 
{
    uint32_t current_bits = 0;
    
    // 1. 动态决定等待方式 
    uint32_t wait_ticks = ((global_active_mask & interest_mask) == 0) ? 
                          portMAX_DELAY : pdMS_TO_TICKS(poll_timeout);
 
    // 2. 获取任务通知值 
	//printf("ActiveMask: 0x%08X, Interest: 0x%08X\n", global_active_mask, interest_mask);
    xTaskNotifyWait(0, 0, &current_bits, wait_ticks);
    //printf("NotifyVal: 0x%08X\n", current_bits);

    // 3. 简洁的自动清理：利用位移合并掩码，一次性清除 
    uint32_t relevant_stops = (current_bits >> 16) & interest_mask;
    
    if (relevant_stops != 0) 
    {
        // 同时清除低 16 位的 READ 位和高 16 位的 STOP 位 
        ulTaskNotifyValueClear(NULL, relevant_stops | (relevant_stops << 16));
        
        // 更新 current_bits 以反映清除后的状态 
        xTaskNotifyAndQuery(xTaskGetCurrentTaskHandle(), 0, eNoAction, &current_bits);
    }

    // 4. 返回过滤后的活跃位 
    return (current_bits & interest_mask);
}



// 
// 实现：自动绑定任务句柄到索引数组 
static void _bind_task_handles(void) 
{
    for (int i = 0; i < MAP_COUNT; i++) 
    {
        sensor_id_t id = g_sensor_task_map[i].id;
        TaskHandle_t *ph = g_sensor_task_map[i].p_handle;
        if (ph != NULL && *ph != NULL) 
        {
            SensorHub.task_handles[GET_SENSOR_INDEX(id)] = *ph;
        }
    }
}


// 实现：初始化资源 
static bool _init(void) 
{
    SensorHub.queue = xQueueCreate(SENSOR_HUB_QLEN, sizeof(sensor_hub_msg_t));
    memset(observers, 0, sizeof(observers));
    memset(SensorHub.snapshot, 0, sizeof(SensorHub.snapshot));
    memset(SensorHub.task_handles, 0, sizeof(SensorHub.task_handles));
    _bind_task_handles();
    return (SensorHub.queue != NULL);
}

// 
// 实现：推送新数据 
static bool _push(sensor_id_t id, const void* src_data) 
{
    if (id == SENSOR_NONE || src_data == NULL) return false;
    int idx = GET_SENSOR_INDEX(id);

    // 更新数据快照 
    memcpy(SensorHub.snapshot[idx], src_data, SENSOR_DATA_BUF_SIZE);

    // 若当前 ID 无人订阅则直接返回 
    if (!(global_active_mask & (uint32_t)id)) 
	{
		//printf("没有订阅 msg ID: %d\n", id);
		return true;
	}

	// 封装消息入队 
	sensor_hub_msg_t msg = { .id = id, .timestamp = xTaskGetTickCount() };
	memcpy(msg.data, src_data, SENSOR_DATA_BUF_SIZE);

	if (xQueueSend(SensorHub.queue, &msg, 0) != pdPASS) {
		//printf("Error: SensorHub queue full! Dropped msg ID: %d\n", id);
		return false;
	}
	return true;
}


// 实现：获取队列消息 
static bool _pop(sensor_hub_msg_t* out_msg) 
{
    return (xQueueReceive(SensorHub.queue, out_msg, 0) == pdPASS);
}


// 实现：订阅传感器及唤醒任务 
static void _subscribe(sensor_id_t id, sensor_hub_callback_t cb) 
{
    if (id == SENSOR_NONE || cb == NULL) return;
    int idx = GET_SENSOR_INDEX(id);

    for (int i = 0; i < SENSOR_SUB_MAX; i++) 
    {
        if (observers[idx].cb[i] == NULL) 
        {
            observers[idx].cb[i] = cb;
            
            // 更新全局活跃掩码位 
            global_active_mask |= (uint32_t)id;

            // 立即同步快照数据 
            cb(id, SensorHub.snapshot[idx]);
            // 发送 READ 指令 (低16位) 
            if (SensorHub.task_handles[idx] != NULL) 
            {
				//printf(" ------------------_subscribe id = %x\n", id);
                xTaskNotify(SensorHub.task_handles[idx], SENSOR_OP_READ(id), eSetBits);
            }
            break;
        }
    }
}


// 实现：取消订阅 
static void _unsubscribe(sensor_id_t id, sensor_hub_callback_t cb) 
{
    if (id == SENSOR_NONE) return;
    int idx = GET_SENSOR_INDEX(id);
    bool still_has_listener = false;

    for (int i = 0; i < SENSOR_SUB_MAX; i++) 
    {
        if (observers[idx].cb[i] == cb) observers[idx].cb[i] = NULL;
        if (observers[idx].cb[i] != NULL) still_has_listener = true;
    }

    // 若无人订阅则关闭掩码位并发送 STOP 指令 
    if (!still_has_listener) 
    {
        global_active_mask &= ~((uint32_t)id);
        if (SensorHub.task_handles[idx] != NULL) 
        {
            // 发送 STOP 指令 (高16位) 
            xTaskNotify(SensorHub.task_handles[idx], SENSOR_OP_STOP(id), eSetBits);
        }
    }
}


// 实现：消息分发 
static void _dispatch(sensor_hub_msg_t* msg) 
{
    int idx = GET_SENSOR_INDEX(msg->id);
    for (int i = 0; i < SENSOR_SUB_MAX; i++) 
    {
        if (observers[idx].cb[i] != NULL) 
        {
            observers[idx].cb[i](msg->id, msg->data);
        }
    }
}

// 实现：获取活跃掩码 
static uint32_t _get_active_mask(void) 
{
    return global_active_mask;
}

/**
 * @brief 清空传感器枢纽的消息队列
 */
static void _sensor_hub_clear(void)
{
    if (SensorHub.queue != NULL) 
    {
        // 直接将队列重置为空状态
        xQueueReset(SensorHub.queue);
    }
}

// 接口对象单例实例化 
sensor_hub_t SensorHub = 
{
    .init = _init,
	.bind_task_handles = _bind_task_handles,
    .push = _push,
    .pop = _pop,
    .subscribe = _subscribe,
    .unsubscribe = _unsubscribe,
    .dispatch = _dispatch,
    .get_active_mask = _get_active_mask,
    .sync_status = _sync_status,
	.clear = _sensor_hub_clear,
};

