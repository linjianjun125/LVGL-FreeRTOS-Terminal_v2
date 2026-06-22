#ifndef __BOOTLOADER_CORE_H
#define __BOOTLOADER_CORE_H

#include "fw_manager.h"
#include "../middlewares/fatfs/source/ff.h"

typedef void (*pFunction)(void);


HAL_StatusTypeDef fw_process_upgrade_flow(const char *sd_path);
void jump_to_app(uint32_t app_addr);

#endif /* __BOOTLOADER_CORE_H */
