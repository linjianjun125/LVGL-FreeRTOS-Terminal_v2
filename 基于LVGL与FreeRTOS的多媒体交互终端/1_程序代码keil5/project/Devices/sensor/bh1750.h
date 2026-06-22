#ifndef __BH1750_H
#define __BH1750_H

#include "sys.h"

#define BH1750_POWER_ON			0x01	// power on
#define BH1750_POWER_DOWN   	0x00	// power down
#define BH1750_RESET			0x07	// reset	
#define BH1750_CON_H_RES_MODE	0x10	// Continuously H-Resolution Mode
#define BH1750_CON_H_RES_MODE2	0x11	// Continuously H-Resolution Mode2 
#define BH1750_CON_L_RES_MODE	0x13	// Continuously L-Resolution Mode
#define BH1750_ONE_H_RES_MODE	0x20	// One Time H-Resolution Mode
#define BH1750_ONE_H_RES_MODE2	0x21	// One Time H-Resolution Mode2
#define BH1750_ONE_L_RES_MODE	0x23	// One Time L-Resolution Mode


void bh1750_init(void);
uint8_t bh1750_reset(void);
uint8_t bh1750_power_on(void);
uint8_t bh1750_power_down(void);
uint16_t bh1750_single_read_light(uint8_t mode);








#endif



