#include "bh1750.h"
#include "sensor_iic_port.h"
#include "../../system/delay/delay.h"



void bh1750_init(void)
{
	bh1750_reset();
	bh1750_power_on();
	printf("[info] 光照传感器 初始化成功\r\n");
	
}

// 复位
uint8_t bh1750_reset(void)
{
    return bh1750_write_cmd(BH1750_RESET);
}

// 打开电源
uint8_t bh1750_power_on(void)
{
    return bh1750_write_cmd(BH1750_POWER_ON);
}

// 关闭电源
uint8_t bh1750_power_down(void)
{
    return bh1750_write_cmd(BH1750_POWER_DOWN);
}

// 单次读取光照值
uint16_t bh1750_single_read_light(uint8_t mode)
{  
    uint8_t temp[2];
    uint16_t result = 0;
    
    if(mode != BH1750_ONE_H_RES_MODE && mode != BH1750_ONE_H_RES_MODE2 && mode != BH1750_ONE_L_RES_MODE)
    {
        printf("[error] bh1750 single read measure mode error!\r\n");
        return 0;
    }
	
	bh1750_write_cmd(mode);			// 开始测量
	switch (mode)
	{
	case BH1750_ONE_H_RES_MODE:  	// 单次H分辨率模式（精度1lx，测量时间120ms）
		sys_delay_ms(120);
		break;
	case BH1750_ONE_H_RES_MODE2: 	// 单次H分辨率模式（精度0.5lx，测量时间120ms）
		sys_delay_ms(120);  
		break;
	case BH1750_ONE_L_RES_MODE:  	// 单次L分辨率模式（精度4lx，测量时间16ms）
		sys_delay_ms(16); 
		break;
	default:
		break;
	}
	
	bh1750_read_data(temp, 2);  						// 读取测量结果
	result = ((uint16_t)((temp[0] << 8) + temp[1]) / 1.2); // 换算成光照值
    
    return result;
}





