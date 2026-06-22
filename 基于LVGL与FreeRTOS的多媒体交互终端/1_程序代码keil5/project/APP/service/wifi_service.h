#ifndef __WIFI_SERVICE_H
#define __WIFI_SERVICE_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"

#define WIFI_BUF_LEN  (1024)


typedef enum 
{
    AT_OK = 0,
    AT_ERROR,
    AT_TIMEOUT
} wifi_status_t;

typedef struct 
{
    int temperature;     
    // 缓存的温度值（如 -2）
    
    char info[8];       
    // 天气描述（如 "雾" 或 "Fog"）
    
    char city[12];       
    // 城市名称（如 "北京"）
} weather_data_t;

typedef struct 
{
	uint8_t p_data[WIFI_BUF_LEN];
	uint16_t length;
} uart_msg_t;

typedef struct 
{
	// 异步接收字节队列
    QueueHandle_t xRxQueue;
    
	 // 事件标志组（处理 OK/READY/CLOSED 等信号）
    EventGroupHandle_t xEvGrp;
   
	// 串口资源互斥锁，保证 AT 指令发送的原子性
    SemaphoreHandle_t xUartMutex;
    
	// 初始化 WiFi 驱动资源和句柄
    wifi_status_t (*init)(void);

	// 连接指定的 WiFi 热点
    wifi_status_t (*connect_wifi)(const char* ssid, const char* pwd);
    
	// 从心知天气 API 获取数据并触发解析
    wifi_status_t (*get_weather)(const char* city);

	// 读取当前结构体中缓存的温度值
    int (*read_weather_data)(weather_data_t* weather_data); 

	// 供解析任务调用的轮询处理逻辑
    void (*handler_loop)(void);      
    
	// 供串口接收中断调用的处理函数
    void (*wifi_rx_done_callback)(const uint8_t *data, uint16_t len);
} wifi_control_t;

extern wifi_control_t WifiControl;

#endif
