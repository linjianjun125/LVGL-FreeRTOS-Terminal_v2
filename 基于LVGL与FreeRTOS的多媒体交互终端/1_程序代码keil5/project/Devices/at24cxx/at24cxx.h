#ifndef __AT24CXX_H
#define __AT24CXX_H

#include "sys.h"



void at24cxx_init(void);        
void at24cxx_write_data(uint16_t addr, uint8_t *pbuf, uint16_t datalen); 
void at24cxx_read_data(uint16_t addr, uint8_t *pbuf, uint16_t datalen); 

#endif
