#include "sys.h"
#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"


void freertos_task(void);
void vMonitoringTask(void *pvParameters);

void fw_app_read_current_info(void);

extern FATFS fs_sdmmc;
int main(void)
{
	config_stm32_clock_init(160, 5, 2, 4);   
	__enable_irq();  // 开启全局中断 (清除 PRIMASK)
    sys_nvic_set_vector_table(FLASH_BASE, BOOTLOADER_SIZE); 
    sys_nvic_priority_group_config(4);
	
	___POWER_ENABLE();
	

    // --- 1. 基础硬件极速初始化 ---
    sys_delay_init(400);        
	sys_debug_init(100, 115200);
	sys_random_init();
    sdram_init();                               
	lcdrgb_init();

	
    // --- 2. 内存池初始化 ---
	mpu_memory_protection();                
    my_mem_init(SRAMIN);   
    my_mem_init(SRAMEX);   
    my_mem_init(SRAMDTCM); 
    my_mem_init(SRAMITCM); 

    // --- 3. UI 引擎基础初始化 (不含业务数据) ---
    lv_init();
    lv_port_disp_init();                         // 注册显示接口
    lv_port_indev_init();                        // 注册输入接口

    // --- 4. 进入 RTOS 任务创建阶段 ---
	fw_global_info_init();
	freertos_task();

	
    while(1);

	
}





void freertos_task(void)
{
    // --- 1. 立即启动 UI 任务 (高优先级，确保开机动画流畅) ---
	xTaskCreate(vUITask, "ui_task", 2048, NULL, 4, &xUITaskHandle); 

    // --- 2. 启动系统异步初始化任务 (一次性任务) ---
    xTaskCreate(vSystemLoaderTask, "loader", 1024, NULL, 3, NULL);
	
	// --- 任务堆栈使用率任务 ---
	//xTaskCreate(vMonitoringTask, "vMonitoringTask", 256, NULL, 1, NULL);

    // --- 3. 启动内核调度器 ---
    vTaskStartScheduler();
}



/* 动态内存申请失败时调用 */
void vApplicationMallocFailedHook(void)
{
    /* 这里的打印非常重要，它告诉你：别找逻辑 Bug 了，赶紧加内存或者减堆栈！ */
    printf("\r\n[CRITICAL] FreeRTOS Malloc Failed!\r\n");
    printf("Check: configTOTAL_HEAP_SIZE or Task Stack sizes.\r\n");
    
    /* 屏蔽中断，防止调度器继续运行，原地卡死方便调试 */
    taskDISABLE_INTERRUPTS();
    while(1); 
}


/* 任务栈溢出时调用 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* pcTaskName 会告诉你具体是哪个任务炸了 */
    printf("\r\n[CRITICAL] Stack Overflow in Task: [%s]\r\n", pcTaskName);
    
    /* * 这里的 xTask 是指向违规任务的句柄。
     * 触发这里通常意味着你需要调大该任务在 xTaskCreate 里的 StackDepth。
     */
    taskDISABLE_INTERRUPTS();
    while(1);
}

// 建议将缓冲区加大到 1024，因为 H7 任务多且字符串格式化较长
char pcWriteBuffer[1024]; 

void vMonitoringTask(void *pvParameters) 
{
//    HeapStats_t xStats;
//	vTaskDelay(pdMS_TO_TICKS(50000));
    for(;;) 
    {
//        // 1. 打印任务堆栈使用情况 (High Water Mark)
//        printf("\n--- 任务状态统计 (Stack High Water Mark) ---\n");
//        printf("Name          State  Priority  Stack   Num\n");
//        vTaskList(pcWriteBuffer);
//        printf("%s", pcWriteBuffer);
        
        // 2. 打印 CPU 占用率统计 (Run Time Stats)
        // 这一部分依赖于你之前配置的 TIM5 寄存器代码
//        printf("\n--- CPU 占用率统计 ---\n");
//        printf("Name          Abs Time      %% Time\n"); 
//        vTaskGetRunTimeStats(pcWriteBuffer);
//        printf("%s", pcWriteBuffer);

//        // 3. 打印堆内存统计
//        vPortGetHeapStats(&xStats);
//        printf("\n--- 堆内存统计 ---\n");
//        printf("剩余总空间: %d bytes\n", xStats.xAvailableHeapSpaceInBytes);
//        printf("最大空闲块: %d bytes\n", xStats.xSizeOfLargestFreeBlockInBytes); 
//        printf("成功分配次数: %d\n", xStats.xNumberOfSuccessfulAllocations);
//        printf("成功释放次数: %d\n", xStats.xNumberOfSuccessfulFrees);
//        printf("------------------------------------------\n");
        
        // 每 5 秒监控一次
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}





/**
 * @brief 初始化 TIM5 供 FreeRTOS 运行时间统计使用
 * 时钟源: 200MHz, 目标计数频率: 10kHz
 */
void StartTimerForRunTimeStats(void) 
{
    // 1. 使能 TIM5 时钟 (位于 D2 域 APB1L)
    RCC->APB1LENR |= RCC_APB1LENR_TIM5EN;
    
    // 2. 停止计数器，清空状态
    TIM5->CR1 = 0;
    TIM5->SR = 0;
    
    // 3. 设置预分频器 (PSC)
    // 200,000,000 / 20,000 = 10,000 Hz
    TIM5->PSC = 20000 - 1; 
    
    // 4. 设置自动重装载 (ARR) 为最大值 (32位)
    // 这样计数器可以跑约 4.9 天才溢出
    TIM5->ARR = 0xFFFFFFFF;
    
    // 5. 产生更新事件使 PSC 生效
    TIM5->EGR |= TIM_EGR_UG;
    
    // 6. 启动计数器
    TIM5->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief 获取当前计数值 (由 FreeRTOS 内核调用)
 */
uint32_t GetTimerCounterValue(void) {
    return TIM5->CNT;
}
