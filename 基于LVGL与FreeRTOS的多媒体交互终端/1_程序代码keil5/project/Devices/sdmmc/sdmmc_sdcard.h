#ifndef __SDMMC_SDCARD_H
#define __SDMMC_SDCARD_H

#include "sys.h"
#include "sys.h"
#include "sdcard_define.h"


/*!< SD config定义 >!*/
#define SDMMC_ENABLE()					do{ConfigRCC_GPIOC_EN(); ConfigRCC_GPIOD_EN();RCC->AHB3ENR |= 1 << 16;}while(0)
#define SDMMC1_D0_GPIO                 	GPIOC
#define SDMMC1_D1_GPIO                 	GPIOC
#define SDMMC1_D2_GPIO                 	GPIOC
#define SDMMC1_D3_GPIO                 	GPIOC
#define SDMMC1_CLK_GPIO                	GPIOC
#define SDMMC1_CMD_GPIO                 GPIOD
#define SDMMC1_D0_PIN                  	ConfigIO_Pin8
#define SDMMC1_D1_PIN                  	ConfigIO_Pin9
#define SDMMC1_D2_PIN                  	ConfigIO_Pin10
#define SDMMC1_D3_PIN                  	ConfigIO_Pin11
#define SDMMC1_CLK_PIN                  ConfigIO_Pin12
#define SDMMC1_CMD_PIN                  ConfigIO_Pin2

/* 
 * SDMMC时钟计算公式: SDMMC_CK时钟 = sdmmc_ker_ck / [2 * clkdiv]; 
 * 其中,sdmmc_ker_ck我们设置来自pll1_q_ck,为200Mhz
 * 如果出现驱动错误,请尝试将SDMMC_TRANSFER_CLK_DIV频率降低
 * 直接使用pll1_q_ck时钟源的时候,操作SD卡时,非常容易出现CRC错误.
 */
#define SDMMC_INIT_CLK_DIV        250       /* SDMMC初始化频率，200M/(250*2)=400Khz,最大400Kh */
#define SDMMC_TRANSFER_CLK_DIV    3         /* SDMMC传输频率,该值太小可能会导致读写文件出错 */


/*!< API函数 >!*/
SD_Error sd_init(void);
SDCardState sdmmc_get_status(void);
uint8_t  sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);
uint8_t  sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);



/*!< static 函数 >!*/
static SD_Error sdmmc_cmd_error(void);
static SD_Error sdmmc_cmd_resp1_error(uint8_t cmd);
static SD_Error sdmmc_cmd_resp2_error(void);
static SD_Error sdmmc_cmd_resp3_error(void);
static SD_Error sdmmc_cmd_resp6_error(uint8_t cmd, uint16_t *prca);
static SD_Error sdmmc_cmd_resp7_error(void);

static SD_Error sdmmc_wide_bus_enable(uint8_t enx);
static SD_Error sdmmc_wide_bus_operation(uint32_t bwide);
static SD_Error sdmmc_is_card_programming(uint8_t *pstatus);

static SD_Error sdmmc_send_status(uint32_t *pstatus);
static SD_Error sdmmc_find_scr(uint16_t rca, uint32_t *pscr);

static SD_Error sdmmc_power_on(void);
static void sdmmc_clock_set(uint16_t clkdiv);
static SD_Error sdmmc_initialize_cards(void);
static SD_Error sdmmc_select_deselect(uint32_t addr);
static SD_Error sdmmc_get_card_info(SD_CardInfo *cardinfo);
static void sdmmc_send_cmd(uint8_t cmdindex, uint8_t waitrsp, uint32_t arg);
static void sdmmc_send_data_cfg(uint32_t datatimeout, uint32_t datalen, uint8_t blksize, uint8_t dir);

static SD_Error sdmmc_read_blocks(uint8_t *pbuf, long long addr, uint16_t blksize, uint32_t nblks);
static SD_Error sdmmc_write_blocks(uint8_t *pbuf, long long addr, uint16_t blksize, uint32_t nblks);

#endif





