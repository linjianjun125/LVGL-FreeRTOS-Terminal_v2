#ifndef __LCDMCU_TOUCH_IIC_H
#define __LCDMCU_TOUCH_IIC_H

#include "sys.h"
#include "sys.h"

// D-FT6336U

/*!< 宏定义 >!*/
#define LCD_TOUCH_IIC_ENABLE()       do{ConfigRCC_GPIOB_EN();}while(0)
#define LCD_TOUCH_SCL_GPIO           GPIOB
#define LCD_TOUCH_SDA_GPIO           GPIOB
#define LCD_TOUCH_SCL_PIN            ConfigIO_Pin7
#define LCD_TOUCH_SDA_PIN            ConfigIO_Pin5

#define lcd_touch_scl_write(x)       config_gpio_set(LCD_TOUCH_SCL_GPIO, LCD_TOUCH_SCL_PIN, x)
#define lcd_touch_sda_write(x)       config_gpio_set(LCD_TOUCH_SDA_GPIO, LCD_TOUCH_SDA_PIN, x)
#define lcd_touch_scl_read()         config_gpio_get(LCD_TOUCH_SCL_GPIO, LCD_TOUCH_SCL_PIN)
#define lcd_touch_sda_read()         config_gpio_get(LCD_TOUCH_SDA_GPIO, LCD_TOUCH_SDA_PIN)

/*!< 器件地址 >!*/
#define TOUCH_CMD_WR       	0X90        // 写命令
#define TOUCH_CMD_RD       	0X91        // 读命令

/*!< API >!*/
void lcd_touch_iic_init(void);				 						
uint8_t lcd_touch_write_reg(uint16_t reg, uint8_t data);
void lcd_touch_read_reg(uint16_t reg, uint8_t *buf, uint8_t len); 

/*!< static >!*/
void lcd_touch_iic_start(void);				 /* 发送IIC开始信号 */
void lcd_touch_iic_stop(void);				 /* 发送IIC停止信号 */
void lcd_touch_iic_ack(void);				 /* IIC发送ACK信号  */
void lcd_touch_iic_nack(void);				 /* IIC不发送ACK信号 */
uint8_t lcd_touch_iic_wait_ack(void);		 /* IIC等待ACK信号  */
void lcd_touch_iic_send_byte(uint8_t data);	 /* IIC发送一个字节 */
uint8_t lcd_touch_iic_read_byte(uint8_t ack); /* IIC读取一个字节 */

#endif
