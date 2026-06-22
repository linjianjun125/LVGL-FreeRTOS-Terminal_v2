#include "../drivers/system/sys/sys.h"
#include "../drivers/system/usart/usart.h"
#include "../drivers/system/delay/delay.h"
#include "../drivers/bsp/led/led.h"
#include "../drivers/bsp/lcdrgb/lcdrgb_blk.h"
#include "../drivers/bsp/lcdrgb/lcdrgb.h"
#include "../drivers/bsp/sdmmc/sdmmc_sdcard.h"
#include "../middlewares/fatfs/source/ff.h"
#include "bootloader_core.h"
#include <string.h>


int main(void)
{	
	
    FATFS fs;
    FRESULT res;
    char *update_file = "0:/atk_h743_packed.bin";

    sys_cache_enable();                            
    HAL_Init();                                    
    sys_stm32_clock_init(160, 5, 2, 4); // 400MHz
    delay_init(400);                                
    usart_init(115200);    
	led_init();   	
	//lcdrgb_init();
    
    printf("\r\n--- STM32H7 Bootloader (Linearized) --- \n");

    // 1. 初始化 SD 卡
    if (sd_init() != 0) 
	{
        printf("[Error] SD Init Failed!\r\n");
        goto jump_to_app; // 硬件故障，尝试启动现有 APP
    }

    // 2. 挂载文件系统
    if (f_mount(&fs, "0:", 1) != FR_OK) 
	{
        printf("[info] 未发现 SD 卡或文件系统挂载失败\n");
        goto jump_to_app;
    }
    printf("[info] 挂载 SD 卡成功\n");


	//flash_atomic_erase(FLASH_BANK_2, 7, 1);
	// 3. 更新程序
	fw_process_upgrade_flow("0:/atk_h743_packed.bin");

jump_to_app:
    f_mount(NULL, "0:", 0);
	jump_to_app(ADDR_APP1_START);
	
    while (1) 
	{
        delay_ms(500);
        printf("Jump Failed: System Halted.\r\n");
    }
}




















