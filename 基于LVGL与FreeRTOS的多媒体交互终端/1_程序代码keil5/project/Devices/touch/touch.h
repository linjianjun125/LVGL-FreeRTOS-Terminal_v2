#ifndef __LCDMCU_TOUCH_H
#define __LCDMCU_TOUCH_H

#include "sys.h"
#include "sys.h"



#define LCD_TOUCH_ENABLE()           do{ConfigRCC_GPIOB_EN(); }while(0)
#define LCD_TOUCH_RST_GPIO           GPIOB
#define LCD_TOUCH_INT_GPIO           GPIOB
#define LCD_TOUCH_RST_PIN            ConfigIO_Pin8
#define LCD_TOUCH_INT_PIN            ConfigIO_Pin4

#define lcd_touch_res_write(x)       config_gpio_set(LCD_TOUCH_RST_GPIO, LCD_TOUCH_RST_PIN, x)
#define lcd_touch_int_write(x)       config_gpio_set(LCD_TOUCH_INT_GPIO, LCD_TOUCH_INT_PIN, x)


// 寄存器地址
#define FT_ID_G_CIPHER 			0xA3
#define FT_ID_G_THGROUP 		0x80
#define FT_ID_G_PERIODACTIVE 	0x88
#define FT_TD_STATUS 			0x02


uint8_t lcd_touch_init(void);
bool lcd_touch_scan(int16_t* x, int16_t* y);


#endif
