#ifndef __PWR_H
#define __PWR_H

#include "sys.h"


#define PWR_WKUP_GPIO_PORT              GPIOA
#define PWR_WKUP_GPIO_PIN               ConfigIO_Pin0
#define PWR_WKUP_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA口时钟使能 */
  
#define PWR_WKUP_INT_IRQn               EXTI0_IRQn
#define PWR_WKUP_INT_IRQHandler         EXTI0_IRQHandler


void pwr_enter_sleep(void);         /* 进入睡眠模式 */
void pwr_enter_stop(void);          /* 进入停止模式 */
void pwr_enter_standby(void);       /* 进入待机模式 */

#endif




















