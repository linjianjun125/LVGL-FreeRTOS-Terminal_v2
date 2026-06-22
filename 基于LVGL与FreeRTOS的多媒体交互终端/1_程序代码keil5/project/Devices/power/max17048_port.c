#include "max17048_port.h"
#include "max17048_iic.h"





/**
 * @brief   写入一次数据
 * @param   reg : 起始寄存器地址
 * @param   data : 数据
 * @retval  0, 成功; 1, 失败;
 */
uint8_t max17048_write_reg(uint8_t reg, uint8_t data)
{
    uint8_t ret = 0;
    max17048_iic_start();
    max17048_iic_send_byte((MAX17048_IIC_ADDR << 1)  | MAX17048_IIC_W);    	
    max17048_iic_wait_ack();
    max17048_iic_send_byte(reg);         	
    max17048_iic_wait_ack();

	max17048_iic_send_byte(data);       	
	ret = max17048_iic_wait_ack();
    max17048_iic_stop();                   
    return ret;
}

/**
 * @brief   IIC写入一次数据
 * @param   reg : 起始寄存器地址
 * @param   buf : 数据缓缓存区
 * @param   len : 写数据长度
 * @retval  0, 成功; 1, 失败;
 */
uint8_t max17048_write_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t ret = 0;
    max17048_iic_start();
    max17048_iic_send_byte((MAX17048_IIC_ADDR << 1)  | MAX17048_IIC_W);  
    max17048_iic_wait_ack();
    max17048_iic_send_byte(reg);        
    max17048_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        max17048_iic_send_byte(buf[i]);   
        ret = max17048_iic_wait_ack();
        if (ret)
            break;
    }
    max17048_iic_stop();               
    return ret;
}


/**
 * @brief   读出一次数据
 * @param   reg : 起始寄存器地址
 * @retval  数据
 */
uint8_t max17048_read_reg(uint8_t reg)
{
	uint8_t data = 0;
	
    max17048_iic_start();
    max17048_iic_send_byte((MAX17048_IIC_ADDR << 1) | MAX17048_IIC_W);       
    max17048_iic_wait_ack();
	
    max17048_iic_send_byte(reg);                             	
    max17048_iic_wait_ack();
	
    max17048_iic_start();
    max17048_iic_send_byte((MAX17048_IIC_ADDR << 1)  | MAX17048_IIC_R);       
    max17048_iic_wait_ack();

    data = max17048_iic_read_byte(0);  							
    max17048_iic_stop();                              
	
	return data;
}




/**
 * @brief   IIC读出一次数据
 * @param   reg : 起始寄存器地址
 * @param   buf : 数据缓缓存区
 * @param   len : 读数据长度
 * @retval  无
 */
void max17048_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    max17048_iic_start();
    max17048_iic_send_byte((MAX17048_IIC_ADDR << 1)  | MAX17048_IIC_W);    
    max17048_iic_wait_ack();
	
    max17048_iic_send_byte(reg);                           
    max17048_iic_wait_ack();
	
    max17048_iic_start();
    max17048_iic_send_byte((MAX17048_IIC_ADDR << 1)  | MAX17048_IIC_R);   
    max17048_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        buf[i] = max17048_iic_read_byte(i == (len - 1) ? 0 : 1); 
    }

    max17048_iic_stop();                            
}



