#include "touch.h"
#include "touch_iic.h"
#include "../../system/delay/delay.h"
#include "../../system/debug/debug.h"



uint8_t lcd_touch_init(void)
{
    LCD_TOUCH_ENABLE();
    config_gpio_init(LCD_TOUCH_RST_GPIO, LCD_TOUCH_RST_PIN, 1, 0, 2, 1);
    config_gpio_init(LCD_TOUCH_INT_GPIO, LCD_TOUCH_INT_PIN, 0, 0, 2, 1);
                                                      
    lcd_touch_res_write(0);                                          
    sys_delay_ms(10);
    lcd_touch_res_write(1);                                         
    sys_delay_ms(100);
	
    // 初始化iic
    lcd_touch_iic_init();   
    sys_delay_ms(20);
	printf("[info] 屏幕触摸初始化成功\n");

	
    return 0;
}




bool lcd_touch_scan(int16_t* x, int16_t* y)
{
    uint8_t data[5];

    lcd_touch_read_reg(FT_TD_STATUS, data, 5);		// 读取状态寄存器
		
	if ((data[0] & 0xF) && (data[0] & 0xf) <= 5) 	// data[0]的低4位 触摸点的数量
	{
		*x = ((((uint16_t)(data[1]&0x0F)) << 8) | data[2]); // 获取X坐标
		*y = ((((uint16_t)(data[3]&0x0F)) << 8) | data[4]); // 获取Y坐标
		return true;
	}
	else
	{
		return false;
	}
}















