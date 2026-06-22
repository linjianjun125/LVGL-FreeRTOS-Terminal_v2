#ifndef __CONFIG_GPIO_H
#define __CONFIG_GPIO_H

#include "stm32h7xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


#define ConfigIO_Mode_IN            0       /* 普通输入模式 */
#define ConfigIO_Mode_OUT           1       /* 普通输出模式 */
#define ConfigIO_Mode_AF            2       /* AF功能模式   */
#define ConfigIO_Mode_AIN           3       /* 模拟输入模式 */

#define ConfigIO_Speed_LOW          0       /* GPIO速度(低速,2M)   */
#define ConfigIO_Speed_MID          1       /* GPIO速度(中速,25M)  */
#define ConfigIO_Speed_FAS          2        /* GPIO速度(快速,50M)  */
#define ConfigIO_Speed_HIG          3        /* GPIO速度(高速,100M) */

#define ConfigIO_Pupd_NO          	0       /* 不带上下拉 */
#define ConfigIO_Pupd_PU            1       /* 上拉 */
#define ConfigIO_Pupd_PD            2       /* 下拉 */

#define ConfigIO_Omode_PP           0       /* 推挽输出 */
#define ConfigIO_Omode_OD           1       /* 开漏输出 */

#define     ConfigIO_Pin0          (1 << 0)
#define     ConfigIO_Pin1          (1 << 1)
#define     ConfigIO_Pin2          (1 << 2)
#define     ConfigIO_Pin3          (1 << 3)
#define     ConfigIO_Pin4          (1 << 4)
#define     ConfigIO_Pin5          (1 << 5)
#define     ConfigIO_Pin6          (1 << 6)
#define     ConfigIO_Pin7          (1 << 7)
#define     ConfigIO_Pin8          (1 << 8)
#define     ConfigIO_Pin9          (1 << 9)
#define     ConfigIO_Pin10         (1 << 10)
#define     ConfigIO_Pin11         (1 << 11)
#define     ConfigIO_Pin12         (1 << 12)
#define     ConfigIO_Pin13         (1 << 13)
#define     ConfigIO_Pin14         (1 << 14)
#define     ConfigIO_Pin15         (1 << 15)

void config_gpio_init(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint32_t mode, uint32_t otype, uint32_t ospeed, uint32_t pupd);
void config_afio_init(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t afx);
void config_gpio_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t status);
uint8_t config_gpio_get(GPIO_TypeDef *p_gpiox, uint16_t pinx);




#endif

