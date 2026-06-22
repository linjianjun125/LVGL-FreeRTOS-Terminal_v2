//#include "at24cxx_port.h"
//#include "at24cxx_iic.h"



//uint8_t at24cxx_addr_scan(uint8_t *buf, uint8_t length)
//{
//	uint8_t addr;
//	uint8_t i = 0;
//	uint8_t res;
//	
//	for(addr = 0x01; addr < 0x7f; addr++)
//	{
//		at24cxx_iic_start();
//		at24cxx_iic_send_byte(addr << 1);
//		if(!at24cxx_iic_wait_ack())
//		{
//			buf[i] = addr;
//			i++;
//			if(i > length - 1) break;
//		}
//		at24cxx_iic_stop();
//	}
//	
//	res = (i == 0 ? 1 : 0);
//	return res;
//}



///**
// * @brief   IIC写入一次数据
// * @param   reg : 起始寄存器地址
// * @param   buf : 数据缓缓存区
// * @param   len : 写数据长度
// * @retval  0, 成功; 1, 失败;
// */
//uint8_t at24cxx_write_regs(uint16_t reg, uint8_t *buf, uint8_t len)
//{
//    uint8_t i;
//    uint8_t ret = 0;
//    at24cxx_iic_start();
//    at24cxx_iic_send_byte((AT24CXX_IIC_ADDR << 1)  | AT24CXX_IIC_W);  
//    at24cxx_iic_wait_ack();
//	
//	
//	at24cxx_iic_send_byte((uint8_t)(reg >> 8));        
//    at24cxx_iic_wait_ack();
//	at24cxx_iic_send_byte((uint8_t)(reg & 0xFF));       
//    at24cxx_iic_wait_ack();


//    for (i = 0; i < len; i++)
//    {
//        at24cxx_iic_send_byte(buf[i]);   
//        ret = at24cxx_iic_wait_ack();
//        if (ret)
//            break;
//    }
//    at24cxx_iic_stop();               
//    return ret;
//}



///**
// * @brief   IIC读出一次数据
// * @param   reg : 起始寄存器地址
// * @param   buf : 数据缓缓存区
// * @param   len : 读数据长度
// * @retval  无
// */
//void at24cxx_read_regs(uint16_t reg, uint8_t *buf, uint8_t len)
//{
//    uint8_t i;
//    at24cxx_iic_start();
//    at24cxx_iic_send_byte((AT24CXX_IIC_ADDR << 1)  | AT24CXX_IIC_W);    
//    at24cxx_iic_wait_ack();
//	
//	at24cxx_iic_send_byte((uint8_t)(reg >> 8));        
//    at24cxx_iic_wait_ack();
//	at24cxx_iic_send_byte((uint8_t)(reg & 0xFF));       
//    at24cxx_iic_wait_ack();
//	
//    at24cxx_iic_start();
//    at24cxx_iic_send_byte((AT24CXX_IIC_ADDR << 1)  | AT24CXX_IIC_R);   
//    at24cxx_iic_wait_ack();

//    for (i = 0; i < len; i++)
//    {
//        buf[i] = at24cxx_iic_read_byte(i == (len - 1) ? 0 : 1); 
//    }

//    at24cxx_iic_stop();                            
//}



