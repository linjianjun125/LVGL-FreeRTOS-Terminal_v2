#include "key.h"
#include "../../system/delay/delay.h"
#include "FreeRTOS.h"
#include "task.h"



void power_enable(void)
{
	ConfigRCC_GPIOH_EN();
	config_gpio_init(GPIOH, ConfigIO_Pin8, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU);
	config_gpio_set(GPIOH, ConfigIO_Pin8, 1);
}

void power_disable(void)
{
	config_gpio_set(GPIOH, ConfigIO_Pin8, 0);
}















void key_init(void)
{
    KEY_ENABLE();

    config_gpio_init(KEY_UP_GPIO_PIN, 	 ConfigIO_Mode_IN, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_gpio_init(KEY_DOWM_GPIO_PIN,  ConfigIO_Mode_IN, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_gpio_init(KEY_POWER_GPIO_PIN, ConfigIO_Mode_IN, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
	
}

/**
 * @brief       按键扫描函数(响应优先级(同时按下多个按键): KEY1 > KEY3 > KEY3)
 * @param       mode:0 / 1, 具体含义如下:
 *   @arg       0,  不支持连续按(当按键按下不放时, 只有第一次调用会返回键值,必须松开以后, 再次按下才会返回其他键值)
 *   @arg       1,  支持连续按(当按键按下不放时, 每次调用该函数都会返回键值)
 * @retval      0, 未按下，其他，键值
 */
uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  
    uint8_t keyval = 0;
	uint8_t key1, key2, key3;
	
	key1 = key_up_read();
	key2 = key_down_read();
	key3 = key_power_read();
	
    if(mode == KEY_MODE_CONTIN) key_up = 1; 
	
    if (key_up && (key1 == 0 || key2 == 0 || key3 == 0)) // 按下
    {
		sys_delay_ms(10);
		
		key_up = 0;
        if (key_up_read() == 0)  	keyval = KEY_UP;
        if (key_down_read() == 0)  	keyval = KEY_DOWN;
		if (key_power_read() == 0)  keyval = KEY_POWER;
    }
    else if (key1 == 1 && key2 == 1 && key3 == 1)   // 松开
    {
        key_up = 1;
    }

    return keyval;       
}





















