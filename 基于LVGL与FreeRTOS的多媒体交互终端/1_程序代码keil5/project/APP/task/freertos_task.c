#include "freertos_task.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"


// 定义全局句柄
TaskHandle_t xMusicTaskHandle     = NULL;
TaskHandle_t xVolumeTaskHandle    = NULL;
TaskHandle_t xKeyTaskHandle       = NULL;
TaskHandle_t xBacklightTaskHandle = NULL;
TaskHandle_t xRTCTaskHandle       = NULL;
TaskHandle_t xSensorTaskHandle    = NULL;
TaskHandle_t xUITaskHandle        = NULL;
TaskHandle_t xIMUTaskHandle 		= NULL;
TaskHandle_t xWifiAppaskHandle 		= NULL;
TaskHandle_t xWifiParserTaskHandle 	= NULL;

// 定义全局同步资源
SemaphoreHandle_t es8388_Semaphore = NULL;
SemaphoreHandle_t sdmmc_Semaphore  = NULL;


FATFS fs_sdmmc;

extern void ui_notify_progress_update(uint8_t progress);




/*
	STM32H7 的中断优先级数值越小，优先级越高。

	在 FreeRTOS 环境下，有一个硬性规定：任何调用了 FreeRTOS API（如 xQueueSendFromISR）的中断，
	其优先级必须低于（即数值大于）系统定义的 configMAX_SYSCALL_INTERRUPT_PRIORITY
	
	假设 configMAX_SYSCALL_INTERRUPT_PRIORITY 为 5。
	WiFi 串口中断（任何调用了 FreeRTOS API）：设置为 6 或 7。
	这样既保证了串口能及时响应，又不会破坏 FreeRTOS 的内核调度
	
*/





void vSystemLoaderTask(void *pvParameters)
{
    // --- 1. 系统同步资源与消息中心初始化 ---
    es8388_Semaphore = xSemaphoreCreateMutex();
    sdmmc_Semaphore  = xSemaphoreCreateMutex();
	xSemaphoreGive(sdmmc_Semaphore);
    
    // --- 2. 耗时硬件初始化 ---
	ui_notify_progress_update(5);
	led_init();
	ui_notify_progress_update(10);
	key_init();
	ui_notify_progress_update(15);
	rtc_init();						
	ui_notify_progress_update(20);
    sensor_init();					
	ui_notify_progress_update(25);
	max17048_init();				
	ui_notify_progress_update(30);
	es8388_init();           		
	ui_notify_progress_update(35);
	wifi_uart_init(100, 9600);	
	
	
	
	ui_notify_progress_update(40);
	at24cxx_init();				
	ui_notify_progress_update(45);
	norflash_init();				
	ui_notify_progress_update(50);
	sd_init();							
	ui_notify_progress_update(55);
	
	// --- 3.中间件初始化-------
	f_mount(&fs_sdmmc, "0:", 1);	
	fonts_init();					

	ui_notify_progress_update(80);

	
    // --- 3. 业务服务组件初始化 ---
	SensorHub.init();
	ui_notify_progress_update(85);
	MusicControl.init();    
	ui_notify_progress_update(90);
    VolumeControl.init();
	ui_notify_progress_update(93);
	BacklightControl.init();
	ui_notify_progress_update(95);
	NovelControl.init();
	ui_notify_progress_update(96);
	WifiControl.init();
	

    // --- 4. 创建常驻业务逻辑任务 ---
	// 任务创建（栈大小以字为单位，乘以4等于字节）
	xTaskCreate(vMusicTask,     "MusicTask",    	1024, NULL, 5, &xMusicTaskHandle);  
    xTaskCreate(vIMUTask,       "IMUTask",      	512,  NULL, 3, &xIMUTaskHandle);  
    xTaskCreate(vSensorTask,    "SensorTask",   	512,  NULL, 2, &xSensorTaskHandle);  
	xTaskCreate(vKeyTask,       "KeyTask",      	256,  NULL, 2, &xKeyTaskHandle);  
    xTaskCreate(vVolumeTask,    "VolTask",      	256,  NULL, 2, &xVolumeTaskHandle);
	xTaskCreate(vBacklightTask, "BLTask",       	128,  NULL, 2, &xBacklightTaskHandle);
    xTaskCreate(vRTCTask,       "vRTCTask",     	256,  NULL, 1, &xRTCTaskHandle);    
	xTaskCreate(vWifiAppTask, 	"vWifiAppTask", 	512,  NULL, 3, &xWifiAppaskHandle);
	xTaskCreate(vWifiParserTask, "vWifiParserTask", 256,  NULL, 2, &xWifiParserTaskHandle);
	SensorHub.bind_task_handles();


    ui_notify_progress_update(100);

    // --- 5. 自毁任务，释放内存 ---
    printf("[System] Loader task finished, deleting self.\r\n");
    vTaskDelete(NULL);
}




