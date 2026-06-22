#include "led_service.h"
#include "../../devices/led/led.h"




static uint8_t __led_init(void)
{
	return 0;
}


static void __led_open(int index)
{
	led_set(index, 1);
	ledControl.status[index] = 1;
	
	
}

static void __led_close(int index)
{
	led_set(index, 0);
	ledControl.status[index] = 0;
}

led_control_t ledControl = 
{
	.status = {0},
	.init = __led_init,
	.open = __led_open,
	.close = __led_close,

};


