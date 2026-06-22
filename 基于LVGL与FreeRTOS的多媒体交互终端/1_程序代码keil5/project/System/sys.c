#include "sys.h"
#include "../../system/debug/debug.h"
#include "../../system/delay/delay.h"



/*--- 2. 公用 SDRAM 内存分配 ---*/
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __attribute__((at(0xC0000000))) uint8_t game_common_buffer[480 * 480 * 2];
#else
    __attribute__((section(".sdram"))) uint8_t game_common_buffer[480 * 480 * 2];
#endif











// 执行: WFI指令(执行完该指令进入低功耗状态, 等待中断唤醒)
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

// 关闭所有中断(但是不包括fault和NMI中断)
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

// 开启所有中断
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

// 设置栈顶地址
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);
}



// 进入待机模式
void sys_standby(void)
{
    PWR->WKUPEPR &= ~(1 << 0);  /* WKUPEN0 = 0, PA0不用于WKUP唤醒 */
    PWR->WKUPEPR |= 1 << 0;     /* WKUPEN0 = 1, PA0用于WKUP唤醒 */
    PWR->WKUPEPR &= ~(1 << 8);  /* WKUPP0 = 0, PA0高电平唤醒(上升沿) */
    PWR->WKUPEPR &= ~(3 << 16); /* 清除WKUPPUPD原来的设置 */
    PWR->WKUPEPR |= 2 << 16;    /* WKUPPUPD = 10, PA0下拉 */
    PWR->WKUPCR |= 0X3F << 0;   /* 清除WKUP0~5唤醒标志 */
    PWR->CPUCR |= 7 << 0;       /* PDDS_D1/D2/D3 = 1, 允许D1/D2/D3进入深度睡眠模式(PDDS) */
    SCB->SCR |= 1 << 2;         /* 使能SLEEPDEEP位 (SYS->CTRL) */
    sys_wfi_set();              /* 执行WFI指令, 进入待机模式 */
}

// 系统软复位
void sys_soft_reset(void)
{
    SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;
}

// 使能STM32H7的L1-Cache, 同时开启D cache的强制透写
void sys_cache_enable(void)
{
    SCB_EnableICache(); /* 使能I-Cache,函数在core_cm7.h里面定义 */
    SCB_EnableDCache(); /* 使能D-Cache,函数在core_cm7.h里面定义 */
    SCB->CACR |= 1 << 2;/* 强制D-Cache透写,如不开启透写,实际使用中可能遇到各种问题 */
}
















