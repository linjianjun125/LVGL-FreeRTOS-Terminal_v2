#ifndef __QSPI_H
#define __QSPI_H

#include "sys.h"
#include "sys.h"

#define NORFLASH_QSPI_ENABLE()			do{ConfigRCC_GPIOB_EN();ConfigRCC_GPIOF_EN();RCC->AHB3ENR |= 1 << 14;}while(0)

#define NORFLASH_QSPI_IO0_GPIO			GPIOF
#define NORFLASH_QSPI_IO1_GPIO			GPIOF
#define NORFLASH_QSPI_IO2_GPIO			GPIOF
#define NORFLASH_QSPI_IO3_GPIO			GPIOF
#define NORFLASH_QSPI_CLK_GPIO			GPIOB
#define NORFLASH_QSPI_NCS_GPIO			GPIOB
#define NORFLASH_QSPI_IO0_PIN			ConfigIO_Pin8
#define NORFLASH_QSPI_IO1_PIN			ConfigIO_Pin9
#define NORFLASH_QSPI_IO2_PIN			ConfigIO_Pin6
#define NORFLASH_QSPI_IO3_PIN			ConfigIO_Pin7
#define NORFLASH_QSPI_CLK_PIN			ConfigIO_Pin2
#define NORFLASH_QSPI_NCS_PIN			ConfigIO_Pin6
#define NORFLASH_QSPI_IO0_AF			10
#define NORFLASH_QSPI_IO1_AF			10
#define NORFLASH_QSPI_IO2_AF			9
#define NORFLASH_QSPI_IO3_AF			9
#define NORFLASH_QSPI_CLK_AF			9
#define NORFLASH_QSPI_NCS_AF			10





uint8_t qspi_init(void);    													/* 初始化QSPI */
uint8_t qspi_wait_flag(uint32_t flag, uint8_t sta, uint32_t wtime); 			/* QSPI等待某个状态 */
void qspi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t mode, uint8_t dmcycle);  /* QSPI发送命令 */
uint8_t qspi_receive(uint8_t *buf, uint32_t datalen);   						/* QSPI接收数据 */
uint8_t qspi_transmit(uint8_t *buf, uint32_t datalen);  						/* QSPI发送数据 */

#endif



















