#ifndef __LED_SERVICE_H
#define __LED_SERVICE_H



#include <stdint.h>



typedef struct 
{
    
    uint8_t (*init)(void);
	
	uint8_t status[8];		// 0:关闭 1: =打开
	
	void (*open)(int index);
	void (*close)(int index);

} led_control_t;

extern led_control_t ledControl;





#endif

