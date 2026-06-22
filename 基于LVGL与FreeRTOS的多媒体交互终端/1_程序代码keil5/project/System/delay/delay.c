#include "sys.h"
#include "delay.h"


#if SYS_USE_RTOS
    #include "FreeRTOS.h"
    #include "task.h"
    extern void xPortSysTickHandler(void);
#endif

static uint16_t g_fac_us = 0; 
uint32_t g_sys_ms_tick = 0;




void sys_delay_init(uint16_t sysclk)
{
	uint32_t reload = 0;
    g_fac_us = sysclk; 

    reload = sysclk*1000*1000;  
	
#if (!SYS_USE_RTOS)
    reload = reload / 1000;               // 每1/1000秒中断一次
#else 
    reload = reload / configTICK_RATE_HZ; // 每1/configTICK_RATE_HZ秒中断一次
#endif

    SysTick->CTRL |= (1 << 2);  // 内核时钟源
    SysTick->LOAD = reload;     // 计数值
    SysTick->CTRL |= (1 << 1);  // 使能SYSTICK中断
    SysTick->CTRL |= (1 << 0);  // 使能SYSTICK
}

void SysTick_Handler(void)
{
    g_sys_ms_tick++;

#if SYS_USE_RTOS

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
	
#endif
}


uint32_t SYS_GetTick(void)
{
    return g_sys_ms_tick;
}



/*--------------------------------------------------------
                           延时函数
---------------------------------------------------------*/
 
void sys_delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;   
	ticks = nus * g_fac_us;       
	told = SysTick->VAL;  
	while (1)
	{
		tnow = SysTick->VAL;
		if (tnow != told)
		{
			if (tnow < told)
			{
				tcnt += told - tnow;   
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			if (tcnt >= ticks) 
			{
				break;    
			}
		}
	}
} 


void sys_delay_ms(uint16_t nms)
{
	#if SYS_USE_RTOS
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		vTaskDelay(nms);
	}
	else
	#endif
	{
		uint32_t i;

		for (i=0; i<nms; i++)
		{
			sys_delay_us(1000);
		}
	}
}


