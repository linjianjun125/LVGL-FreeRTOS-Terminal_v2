#ifndef _SDRAM_H
#define _SDRAM_H

#include "sys.h"
#include "sys.h"


/*!< SDRAM起始地址 >!*/
#define BANK5_SDRAM_ADDR        ((uint32_t)(0XC0000000))            

/*!< API函数 >!*/
void sdram_init(void);
void fmc_sdram_write_buffer(uint8_t *pbuf, uint32_t writeaddr, uint32_t n);
void fmc_sdram_read_buffer(uint8_t *pbuf, uint32_t readaddr, uint32_t n);

#endif



































