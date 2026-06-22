#include "freertos_task.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"



// 应用业务任务
void vWifiAppTask(void *pvParameters) 
{
	weather_data_t weather;
	sensor_weather_data_t sensor_weather;
	
	memset(sensor_weather.info, '\0', 8);
	sensor_weather.temp = (int16_t)-200;
	SensorHub.push(SENSOR_WEATHER_DATA, &sensor_weather);
	
	wifi_uart_set_callback(WifiControl.wifi_rx_done_callback);
	
    // 1. 连接 WiFi
    while (WifiControl.connect_wifi("jun", "Ljj999888") != AT_OK) 
	{
        printf("WiFi Connect Fail, Retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    printf("WiFi Connected!\n");
	

    for (;;) 
	{
        // 2. 获取天气
        if (WifiControl.get_weather("maoming") == AT_OK) 
		{
			WifiControl.read_weather_data(&weather);
			printf("Update Weather OK\n");
			printf("City: %s\r\n", weather.city);
			printf("Sky : %s\r\n", weather.info);
			printf("Temp: %d C\r\n", weather.temperature);
			
			memset(sensor_weather.info, '\0', 8);
			memcpy(sensor_weather.info, weather.info, 8);
			sensor_weather.temp = (int16_t)weather.temperature;
			SensorHub.push(SENSOR_WEATHER_DATA, &sensor_weather);
        } 
		else 
		{
            printf("Update Weather Failed.\n");
        }
        
		
        // 每 1 分钟更新一次
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}




// 解析任务：优先级建议设高一点，保证串口不丢包
void vWifiParserTask(void *pvParameters)
{
    for (;;) 
	{
        WifiControl.handler_loop();
    }
}










///**
// * @brief  纯流水线方式获取天气数据
// * @param  api_key: 你在心知天气申请的私钥
// * @param  city: 城市拼音，如 "beijing"
// */
//void wifi_get_weather_raw_flow(const char* api_key, const char* city) {
//    char http_payload[256];
//    char at_cmd[64];
//    int len;

//    // --- STEP 1: 准备 HTTP 请求报文 ---
//    // 使用 snprintf 自动合成并计算长度，注意末尾必须有两个 \r\n
//    len = snprintf(http_payload, sizeof(http_payload),
//        "GET /v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c HTTP/1.1\r\n"
//        "Host: api.seniverse.com\r\n"
//        "Connection: close\r\n\r\n", 
//        api_key, city);

//    printf("--- Initializing Connection ---\r\n");

//    // --- STEP 2: 建立 TCP 连接 ---
//    // 连接心知天气服务器，端口 80
//    uart_send_string("AT+CIPSTART=\"TCP\",\"api.seniverse.com\",80\r\n");
//    sys_delay_ms(2000); // 等待 CONNECT OK

//    // --- STEP 3: 发送长度预告 ---
//    // 根据刚才计算的 len 发送 AT+CIPSEND
//    snprintf(at_cmd, sizeof(at_cmd), "AT+CIPSEND=%d\r\n", len);
//    uart_send_string(at_cmd);
//    
//    // 必须等待模块返回 '>' 符号，裸机调试给 500ms 足够了
//    sys_delay_ms(500); 

//    // --- STEP 4: 发送真正的请求体 ---
//    printf("--- Sending HTTP Request (%d bytes) ---\r\n", len);
//    uart_send_string(http_payload);

//    // --- STEP 5: 等待接收并观察 ---
//    // 此时你应该在串口中断的 printf 中看到大量的 JSON 数据回传
//    sys_delay_ms(3000); 
//    printf("\r\n--- Request Finished ---\r\n");
//}

//    // 1. 初始化串口 (假设你的函数定义是这样)
//    wifi_uart_init(100,115200); 
//    sys_delay_ms(1000); // 等待模块稳定

//    // 3. 基础连接（只需要做一次）
//    // 注意：这里的引号需要用 \ 转义
//    uart_send_string("AT+CWMODE=1\r\n");
//    sys_delay_ms(500);
//    uart_send_string("AT+CWJAP=\"USER_55801F\",\"68821775\"\r\n");
//    sys_delay_ms(8000); // 连接 WiFi 比较慢，多给点时间

//    // 4. 执行获取天气逻辑
//    // 请将下方替换为你真实的 API KEY
//    wifi_get_weather_raw_flow("SL0NiP-yxPkzWZbI4", "beijing");
//	
//	sys_delay_ms(30000); 
//	wifi_get_weather_raw_flow("SL0NiP-yxPkzWZbI4", "beijing");

//	sys_delay_ms(30000); 
//	wifi_get_weather_raw_flow("SL0NiP-yxPkzWZbI4", "beijing");
//    while(1)
//    {
//        // 之后进入死循环，你可以每 5 秒查一次状态，看看会不会掉线
//       // uart_send_string("AT+CIPSTATUS\r\n");
//        sys_delay_ms(5000);
//    }
