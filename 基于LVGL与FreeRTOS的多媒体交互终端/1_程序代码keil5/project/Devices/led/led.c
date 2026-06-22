#include "led.h"
#include "../../system/delay/delay.h"


void led_init(void)
{
	ConfigRCC_GPIOB_EN();
	config_gpio_init(GPIOB, ConfigIO_Pin13, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU);
	config_gpio_set(GPIOB, ConfigIO_Pin13, 0);

	ConfigRCC_GPIOB_EN();
	config_gpio_init(GPIOB, ConfigIO_Pin14, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU);
	config_gpio_set(GPIOB, ConfigIO_Pin14, 0);
}




void led_set(int index, uint8_t status)
{
	
	if(index == 0){
		config_gpio_set(GPIOB, ConfigIO_Pin13, status);
	}
	else
	{
		
		config_gpio_set(GPIOB, ConfigIO_Pin14, status);
	}
}

