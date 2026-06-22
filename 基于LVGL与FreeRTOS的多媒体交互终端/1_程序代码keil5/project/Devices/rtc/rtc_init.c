#include "rtc_init.h"
#include "rtc_time.h"
#include "../../system/debug/debug.h"
#include "../../system/delay/delay.h"

static uint32_t rtc_read_bkr(uint32_t bkrx);             // 读后备寄存器
static void rtc_write_bkr(uint32_t bkrx, uint32_t data); // 写后备寄存器
static uint8_t rtc_init_lse(void);
static uint8_t rtc_init_lsi(void);

#define RTC_INIT_LSE_FLAG		(0X5513)
#define RTC_INIT_LSI_FLAG		(0X1511)
#define RTC_UNINIT_FLAG			(0x5353)

/**
 * @brief   RTC初始化
 * @param   无
 * @retval  0,成功 1,失败
 */
uint8_t rtc_init(void)
{
	uint16_t bkpflag, ms_to_wait = 10, ssr;    
	
    PWR->CR1 |= 1 << 8; 				// DBP=1,后备区域写使能
    bkpflag = rtc_read_bkr(0); 			// 读取BKP0的值
	//printf("---%x\r", bkpflag);
	
	// 第一次配置
    if (bkpflag != RTC_INIT_LSE_FLAG && bkpflag != RTC_INIT_LSI_FLAG) 
    {
		if(rtc_init_lse() == 0)
		{
			printf("[info] RTC 外部时钟源选择成功\r\n");
		}
		else
		{
			printf("[info] RTC 外部时钟源选择失败\r\n");
			rtc_init_lsi();
			printf("[info] RTC 内部时钟源选择成功\r\n");
		}
		rtc_set_time(22, 07, 0, 0);
		rtc_set_date(26, 5, 4, 1);
    }
	else if(bkpflag == RTC_INIT_LSE_FLAG)
	{
		printf("[info] RTC 外部时钟源选择成功\r\n");
	}
	else if(bkpflag == RTC_INIT_LSI_FLAG)
	{
		printf("[info] RTC 内部时钟源选择成功\r\n");
	}
	
	
	// 判断RCC是否正常 
	ssr = RTC->SSR; 
	while((RTC->SSR == ssr) && (ms_to_wait > 0))
	{
		sys_delay_ms(1);
		ms_to_wait--;
	}
	if(ms_to_wait == 0)
	{
		rtc_write_bkr(0, RTC_UNINIT_FLAG); 	
		printf("[info] RTC 初始化失败\r\n");
		return 1;
	}
	
	printf("[info] RTC 初始化成功\r\n");
    return 0;
}


static uint8_t rtc_init_lse(void)
{
	uint8_t ms_to_wait = 10;
	
	RCC->BDCR |= 1 << 0;			// LSE使能	
	SYS_WAIT_FLAG_BIT_SET_1(RCC->BDCR, (1 << 1), ms_to_wait);
	if(ms_to_wait == 0) return 1;

	RCC->BDCR &= ~(3 << 8); 
	RCC->BDCR |= 1 << 8; 			// 选择LSE时钟
	RCC->BDCR |= 1 << 15;  			// 使能RTC时钟

	RTC->WPR = 0xCA;				// 关闭RTC寄存器写保护
	RTC->WPR = 0x53;				// 关闭RTC寄存器写保护
	RTC->CR = 0;
	if (rtc_init_mode())
	{
		rtc_write_bkr(0, RTC_UNINIT_FLAG);
		return 1;      		
	}
	RTC->PRER = 0XFF;        		// RTC同步分频系数(0~7FFF)
	RTC->PRER |= 0X7F << 16; 		// RTC异步分频系数(1~0X7F)
	RTC->CR &= ~(1 << 6);    		// RTC设置为,24小时格式
	RTC->ISR &= ~(1 << 7);   		// 退出RTC初始化模式
	RTC->WPR = 0xFF;         		// 使能RTC寄存器写保护
	
	rtc_write_bkr(0, RTC_INIT_LSE_FLAG);
	return 0;
}


static uint8_t rtc_init_lsi(void)
{
	uint8_t ms_to_wait = 10;
	
	RCC->CSR |= 1 << 0; 			// LSI使能		
	SYS_WAIT_FLAG_BIT_SET_1(RCC->CSR, (1 << 1), ms_to_wait);
	if(ms_to_wait == 0) return 1;

	RCC->BDCR &= ~(3 << 8); 
	RCC->BDCR |= 1 << 9; 			// 选择LSI时钟
	RCC->BDCR |= 1 << 15;  			// 使能RTC时钟

	RTC->WPR = 0xCA;				// 关闭RTC寄存器写保护
	RTC->WPR = 0x53;				// 关闭RTC寄存器写保护
	RTC->CR = 0;
	if (rtc_init_mode())
	{
		rtc_write_bkr(0, RTC_UNINIT_FLAG);
		return 1;      		
	}
	
	RTC->PRER = 0XFF;        		// RTC同步分频系数(0~7FFF)
	RTC->PRER |= 0X7F << 16; 		// RTC异步分频系数(1~0X7F)
	RTC->CR &= ~(1 << 6);    		// RTC设置为,24小时格式
	RTC->ISR &= ~(1 << 7);   		// 退出RTC初始化模式
	RTC->WPR = 0xFF;         		// 使能RTC寄存器写保护
	
	rtc_write_bkr(0, RTC_INIT_LSI_FLAG);
	return 0;
}











/**
 * @brief   RTC进入初始化模式
 * @param   无
 * @retval  0,成功;1,失败;
 */
uint8_t rtc_init_mode(void)
{
    if (RTC->ISR & (1 << 6))
        return 0;
	
    RTC->ISR |= 1 << 7; 
	int retry = 0XFFFFF;
    while ((RTC->ISR & (1 << 6)) == 0x00)
    {
        retry--; 
		if(retry < 0) return 1;
    }

    return 0; 
}


/**
 * @brief   等待RSF同步
 * @param   无
 * @retval  0,成功;1,失败;
 */
uint8_t rtc_wait_synchro(void)
{
    // 关闭RTC寄存器写保护
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
	
	// 清除RSF位
    RTC->ISR &= ~(1 << 5);
	int retry = 0XFFFFF;
    while ((RTC->ISR & (1 << 5)) != 0x00)
    {
		retry--; 
		if(retry < 0) return 1;
    }

	// 使能RTC寄存器写保护
    RTC->WPR = 0xFF; 
	
    return 0;
}

/**
 * @brief       十进制转换为BCD码
 * @param       val : 要转换的十进制数 
 * @retval      BCD码
 */
uint8_t rtc_dec2bcd(uint8_t val)
{
    uint8_t bcdhigh = 0;

    while (val >= 10)
    {
        bcdhigh++;
        val -= 10;
    }

    return ((uint8_t)(bcdhigh << 4) | val);
}

/**
 * @brief       BCD码转换为十进制数据
 * @param       val : 要转换的BCD码 
 * @retval      十进制数据
 */
uint8_t rtc_bcd2dec(uint8_t val)
{
    uint8_t temp = 0;
    temp = (val >> 4) * 10;
    return (temp + (val & 0X0F));
}




/**
 * @brief   RTC写入后备区域SRAM
 * @param   bkrx : 后备区寄存器编号,范围:0~31
 * @param   data : 要写入的数据,32位长度
 * @retval  无
 */
static void rtc_write_bkr(uint32_t bkrx, uint32_t data)
{
    uint32_t temp = 0;
    temp = RTC_BASE + 0x50 + bkrx * 4;
    (*(uint32_t *)temp) = data;
}

/**
 * @brief   RTC读取后备区域SRAM
 * @param   bkrx : 后备区寄存器编号,范围:0~31
 * @retval  读取到的值
 */
static uint32_t rtc_read_bkr(uint32_t bkrx)
{
    uint32_t temp = 0;
    temp = RTC_BASE + 0x50 + bkrx * 4;
    return (*(uint32_t *)temp); /* 返回读取到的值 */
}



