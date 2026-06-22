#ifndef __LCDRGB_INIT_H
#define __LCDRGB_INIT_H

#include "sys.h"
#include "config_gpio.h"

#define LCD_INIT_GPIO_ENANLE()		do{ConfigRCC_GPIOD_EN();ConfigRCC_GPIOG_EN();}while(0)

#define LCD_SCK_GPIO 			GPIOD
#define LCD_SCK_PIN 			ConfigIO_Pin7
#define LCD_MOSI_GPIO 			GPIOD
#define LCD_MOSI_PIN 			ConfigIO_Pin6
#define LCD_NCS_GPIO 			GPIOG
#define LCD_NCS_PIN 			ConfigIO_Pin9
#define lcdrgb_sck_set() 		config_gpio_set(LCD_SCK_GPIO, LCD_SCK_PIN, 1)
#define lcdrgb_sck_clr() 		config_gpio_set(LCD_SCK_GPIO, LCD_SCK_PIN, 0)
#define lcdrgb_sda_set() 		config_gpio_set(LCD_MOSI_GPIO, LCD_MOSI_PIN, 1)
#define lcdrgb_sda_clr() 		config_gpio_set(LCD_MOSI_GPIO, LCD_MOSI_PIN, 0)
#define lcdrgb_cs_set() 		config_gpio_set(LCD_NCS_GPIO, LCD_NCS_PIN, 1)
#define lcdrgb_cs_clr() 		config_gpio_set(LCD_NCS_GPIO, LCD_NCS_PIN, 0)


void lcdrgb_param_init(void);
void SPI_WR_REG(uint8_t reg);
void SPI_WR_DATA8(uint8_t dat);


#endif

