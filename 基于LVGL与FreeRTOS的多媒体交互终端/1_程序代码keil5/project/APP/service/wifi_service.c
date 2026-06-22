#include "wifi_service.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../devices/wifi/wifi_uart.h"
#include "../../middle/cjson/cJson.h"

// --- 私有宏定义 ---
#define BIT_OK      (1 << 0)
#define BIT_READY   (1 << 1) 
#define BIT_SEND_OK (1 << 2)
#define BIT_ERROR   (1 << 3)
#define BIT_CLOSED  (1 << 4)
#define BIT_DATA_READY  (1 << 5)

// 缓存最新的天气数据结构体
static weather_data_t g_weather;

// 用于存储 get_weather 解析前的原始 JSON 字符串
static char g_rx_buffer[WIFI_BUF_LEN];

// 心知天气私钥
static const char* API_KEY = "SL0NiP-yxPkzWZbI4"; 

static int _parse_weather_json(const char* json_str) ;
static wifi_status_t _wait_evt(uint32_t bits, uint32_t timeout_ms);


/**
 * @brief 清空 Wifi 接收队列
 */
void wifi_rx_queue_clear(void)
{
    if (WifiControl.xRxQueue != NULL) 
    {
        // xQueueReset 将队列设回初始空状态
        // 注意：此函数不能在中断中使用。如果在中断中，请使用 xQueueResetFromISR
        xQueueReset(WifiControl.xRxQueue);
    }
}

static wifi_status_t _impl_init(void) 
{
	// 创建接收队列
    WifiControl.xRxQueue = xQueueCreate(5, sizeof(uart_msg_t));

	// 创建事件组
    WifiControl.xEvGrp = xEventGroupCreate();
    
	// 创建互斥锁
    WifiControl.xUartMutex = xSemaphoreCreateMutex();
    
    // 初始化cJson
	cJSON_Hooks hooks = { .malloc_fn = pvPortMalloc, .free_fn = vPortFree };
	cJSON_InitHooks(&hooks);
	
    return (WifiControl.xRxQueue && WifiControl.xEvGrp && WifiControl.xUartMutex) ? AT_OK : AT_ERROR;
}

static wifi_status_t _impl_connect_wifi(const char* ssid, const char* pwd) 
{
    char cmd[128];
    wifi_status_t res;

    xSemaphoreTake(WifiControl.xUartMutex, portMAX_DELAY); 
    
    uart_send_string("AT+CWMODE=1\r\n");
    _wait_evt(BIT_OK, 1000);

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    uart_send_string(cmd);
    res = _wait_evt(BIT_OK, 12000); 
    
    uart_send_string("AT+CIPMUX=0\r\n");
    _wait_evt(BIT_OK, 1000);
    
    xSemaphoreGive(WifiControl.xUartMutex); 
    return res;
}

static wifi_status_t _impl_get_weather(const char* city) 
{
    char req[256], cmd[32];
    wifi_status_t res = AT_TIMEOUT;
    memset(g_rx_buffer, 0, WIFI_BUF_LEN);

	wifi_rx_queue_clear();
	
    xSemaphoreTake(WifiControl.xUartMutex, portMAX_DELAY);

    uart_send_string("AT+CIPCLOSE\r\n");
    _wait_evt(BIT_OK | BIT_ERROR, 500);
	
	
    // 1. 建立 TCP
    uart_send_string("AT+CIPSTART=\"TCP\",\"api.seniverse.com\",80\r\n");
    if (_wait_evt(BIT_OK, 3000) == AT_OK) 
    {
        // 2. 自动合成 HTTP
        int len = snprintf(req, sizeof(req),
            "GET /v3/weather/now.json?key=%s&location=%s HTTP/1.1\r\n"
            "Host: api.seniverse.com\r\n"
            "Connection: close\r\n\r\n", API_KEY, city);

        // 3. 发送 CIPSEND
        snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", len);
        xEventGroupClearBits(WifiControl.xEvGrp, BIT_READY);
        uart_send_string(cmd);

        if (_wait_evt(BIT_READY, 1000) == AT_OK) 
        {
            // 4. 发送真正的请求
            uart_send_string(req);
            
            // 5. 等待发送成功
            if(_wait_evt(BIT_SEND_OK, 3000) == AT_OK) 
            {
                // 6. 等待服务器下发数据并最终返回 CLOSED
                res = _wait_evt(BIT_DATA_READY, 5000); 
                
				printf("\n---%s---\n", g_rx_buffer);
                // --- 解析阶段 ---
				if(_parse_weather_json(g_rx_buffer)!= 0)
				{
					res = AT_ERROR;
				}
				else
				{
					res = AT_OK;
				}
            }
        }
    }
    
    //if(res != AT_OK) 
    {
        uart_send_string("AT+CIPCLOSE\r\n");
    }
    
    xSemaphoreGive(WifiControl.xUartMutex);
    return res;
}

static int _impl_read_weather_data(weather_data_t* weather_data) 
{ 
    memcpy(weather_data, &g_weather, sizeof(weather_data_t));
    return 1; 
}

uart_msg_t rx_pkg;
static void _impl_handler(void) 
{
    // 1. 等待队列中的整包数据包 (由 IDLE 中断的回调发出)
    if (xQueueReceive(WifiControl.xRxQueue, &rx_pkg, portMAX_DELAY)) 
    {
        // 将数据视为字符串处理，确保安全
        char *ptr = (char *)rx_pkg.p_data;
        uint16_t len = rx_pkg.length;

        // --- 逻辑 1: 处理透传模式的 ready 信号 ---
        if (strchr(ptr, '>')) {
            xEventGroupSetBits(WifiControl.xEvGrp, BIT_READY);
        }

        // --- 逻辑 2: 处理常规 AT 指令响应 ---
        if (strstr(ptr, "OK\r\n")) {
            xEventGroupSetBits(WifiControl.xEvGrp, BIT_OK);
        }
        
        if (strstr(ptr, "SEND OK")) {
            xEventGroupSetBits(WifiControl.xEvGrp, BIT_SEND_OK);
        }

        if (strstr(ptr, "CLOSED")) {
            xEventGroupSetBits(WifiControl.xEvGrp, BIT_CLOSED); 
        }

        // --- 核心逻辑 3: 处理包含 JSON 的天气数据包 ---
        // 使用 strstr 时，直接在这一包数据里查找关键字
        if (strstr(ptr, "results") && strstr(ptr, "location")) 
        {
            // 找到 JSON 的起始位置 (跳过 +IPD...)
            char *json_start = strchr(ptr, '{');
            if (json_start) 
            {
                // 拷贝纯 JSON 部分或整包数据到业务层缓冲区
                // 建议拷贝整包，因为 g_rx_buffer 通常用于后续的 cJSON_Parse
                memset(g_rx_buffer, 0, WIFI_BUF_LEN);
                strncpy(g_rx_buffer, json_start, len);
                
				//xEventGroupSetBits(WifiControl.xEvGrp, BIT_CLOSED); 
                // 触发业务层解析信号（如果有需要的话，可以再设一个位）
                 xEventGroupSetBits(WifiControl.xEvGrp, BIT_DATA_READY);
            }
        }
        
    }
}


/**
 * @brief 具体的回调实现：负责打包并推送到队列
 */
uart_msg_t msg;
void wifi_rx_done_callback(const uint8_t *data, uint16_t len)
{
    // 限制长度防止溢出
    msg.length = (len > WIFI_BUF_LEN) ? WIFI_BUF_LEN : len;
    
    // 数据快照拷贝
    memcpy(msg.p_data, data, msg.length);
    
    // 字符串安全终止符
    if (msg.length < WIFI_BUF_LEN) msg.p_data[msg.length] = '\0';

    // 推送至队列
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(WifiControl.xRxQueue, &msg, &xHigherPriorityTaskWoken);
    
    // ISR 上下文切换执行
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



// --- 内部工具：等待事件
static wifi_status_t _wait_evt(uint32_t bits, uint32_t timeout_ms) 
{
    EventBits_t uxBits = xEventGroupWaitBits(WifiControl.xEvGrp, bits, pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout_ms));
    return (uxBits & bits) ? AT_OK : AT_TIMEOUT;
}


// --- 内部工具：解析json字符串
static int _parse_weather_json(const char* json_str) 
{
	// --- 核心修改：跳过 AT 指令的报头 ---
    const char* valid_json_start = strchr(json_str, '{'); 
    
    if (valid_json_start == NULL) 
    {
        printf("未找到有效的 JSON 起始符 '{'\r\n");
        return 1;
    }

    // 从找到的 '{' 位置开始解析
    cJSON *root = cJSON_Parse(valid_json_start);
    
    if (!root) 
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        printf("cJSON 解析失败，错误位置: %s\n", error_ptr ? error_ptr : "unknown");
        return 1;
    }
	
    // 2. 定位到 results 数组
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (cJSON_IsArray(results) && cJSON_GetArraySize(results) > 0) 
    {
        cJSON *item = cJSON_GetArrayItem(results, 0);

        // --- 提取城市名 ---
        cJSON *location = cJSON_GetObjectItem(item, "location");
        if (location)
		{
            cJSON *name = cJSON_GetObjectItem(location, "name");
            if (cJSON_IsString(name)) {snprintf(g_weather.city, sizeof(g_weather.city), "%s", name->valuestring);}
        }
		else{return 1;}

        // --- 提取天气描述和温度 ---
        cJSON *now = cJSON_GetObjectItem(item, "now");
        if (now) 
		{
            cJSON *text = cJSON_GetObjectItem(now, "text");
            cJSON *temp = cJSON_GetObjectItem(now, "temperature");

            if (cJSON_IsString(text)){snprintf(g_weather.info, sizeof(g_weather.info), "%s", text->valuestring);}
            if (cJSON_IsString(temp)) {g_weather.temperature = atoi(temp->valuestring);}
        }
		else{return 1;}
    }

    // 3. 释放内存（极其重要！）
    // 这将释放 root 及其下所有 results/now/location 节点
    cJSON_Delete(root);

    // 打印结果验证
//    printf("\r\n--- Weather Update ---\r\n");
//    printf("City: %s\r\n", g_weather.city);
//    printf("Sky : %s\r\n", g_weather.info);
//    printf("Temp: %d C\r\n", g_weather.temperature);
	return 0;
}







/* --- 结构体实例化 --- */
wifi_control_t WifiControl =
 {
    .xRxQueue = NULL,
    .xEvGrp = NULL,
    .xUartMutex = NULL,
    .init = _impl_init,
    .connect_wifi = _impl_connect_wifi,
    .get_weather = _impl_get_weather,
    .read_weather_data = _impl_read_weather_data,
    .handler_loop = _impl_handler,
    .wifi_rx_done_callback = wifi_rx_done_callback,
};
