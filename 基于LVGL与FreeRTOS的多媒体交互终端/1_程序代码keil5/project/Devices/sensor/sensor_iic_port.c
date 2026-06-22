#include "sensor_iic_port.h"
#include "sensor_iic.h"





uint8_t sensor_addr_scan(uint8_t *buf, uint8_t length)
{
	uint8_t addr;
	uint8_t i = 0;
	
	for(addr = 0x01; addr < 0x7f; addr++)
	{
		sensor_iic_start();
		sensor_iic_send_byte(addr << 1);
		if(!sensor_iic_wait_ack())
		{
			buf[i] = addr;
			i++;
			addr++;
			if(i > length - 1) break;
		}
		sensor_iic_stop();
	}
	
	return i;
}



/*-------------------------------------------------------------
                        三轴磁力计传感器
--------------------------------------------------------------*/

uint8_t qmc5883_write_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t ret = 0;
    sensor_iic_start();
    sensor_iic_send_byte((QMC5883_IIC_ADDR << 1)  | QMC5883_IIC_W);  
    sensor_iic_wait_ack();
    sensor_iic_send_byte(reg);        
    sensor_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        sensor_iic_send_byte(buf[i]);   
        ret = sensor_iic_wait_ack();
        if (ret)
            break;
    }
    sensor_iic_stop();               
    return ret;
}


void qmc5883_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    sensor_iic_start();
    sensor_iic_send_byte((QMC5883_IIC_ADDR << 1)  | QMC5883_IIC_W);    
    sensor_iic_wait_ack();
	
    sensor_iic_send_byte(reg);                           
    sensor_iic_wait_ack();
	
    sensor_iic_start();
    sensor_iic_send_byte((QMC5883_IIC_ADDR << 1)  | QMC5883_IIC_R);   
    sensor_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        buf[i] = sensor_iic_read_byte(i == (len - 1) ? 0 : 1); 
    }

    sensor_iic_stop();                            
}


/*-------------------------------------------------------------
                        温湿度传感器
--------------------------------------------------------------*/

uint8_t sht30_write_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t ret = 0;
    
    sensor_iic_start();
    sensor_iic_send_byte((SHT30_IIC_ADDR << 1) | SHT30_IIC_W);  
    if(sensor_iic_wait_ack()) { sensor_iic_stop(); return 1; }

    // 发送指令高字节 (MSB)
    sensor_iic_send_byte(reg);        
    if(sensor_iic_wait_ack()) { sensor_iic_stop(); return 1; }

    // 发送后续字节（如指令低字节 LSB）
    for (i = 0; i < len; i++)
    {
        sensor_iic_send_byte(buf[i]);   
        ret = sensor_iic_wait_ack();
        if (ret) break;
    }
    sensor_iic_stop();               
    return ret;
}

void sht30_read_regs(uint8_t *buf, uint8_t len)
{
    uint8_t i;
    
    // SHT30 读取数据只需：Start -> Device_Addr(R) -> Read Data... -> Stop
    sensor_iic_start();
    sensor_iic_send_byte((SHT30_IIC_ADDR << 1) | SHT30_IIC_R);   
    if(sensor_iic_wait_ack()) 
    {
        sensor_iic_stop();
        return;
    }

    for (i = 0; i < len; i++)
    {
        // 最后一个字节给 NACK (0)，其余给 ACK (1)
        buf[i] = sensor_iic_read_byte(i == (len - 1) ? 0 : 1); 
    }

    sensor_iic_stop();                           
}



/*-------------------------------------------------------------
                        光照传感器
--------------------------------------------------------------*/

uint8_t bh1750_write_cmd(uint8_t cmd)
{
	
	uint8_t status = 0;
	
    sensor_iic_start();
    sensor_iic_send_byte((BH1750_IIC_ADDR << 1) | 0x00);      
    sensor_iic_wait_ack();
	
    sensor_iic_send_byte(cmd);                             		
    sensor_iic_wait_ack();					
    sensor_iic_stop();                                        
	
	return status;
}


uint8_t bh1750_read_data(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    sensor_iic_start();
    sensor_iic_send_byte((BH1750_IIC_ADDR << 1)  | 0x01);     
    sensor_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        buf[i] = sensor_iic_read_byte(i == (len - 1) ? 0 : 1); 
    }

    sensor_iic_stop();       

	return 0;
}







