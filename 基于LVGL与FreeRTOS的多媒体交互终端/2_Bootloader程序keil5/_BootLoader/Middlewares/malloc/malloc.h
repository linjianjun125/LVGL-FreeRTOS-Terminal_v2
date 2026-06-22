#ifndef __MALLOC_H
#define __MALLOC_H

#include <stdint.h>
#include <stdbool.h>
#include "sys.h"

/*
 * 内存池空间划分
 * +-----------------------------------------------------------+--------------+
 * |                       Data Region                         |   Metadata   |
 * |                 (n * Block_Size Bytes)                    | (n * 4 Byte) |
 * +-----------------------------------------------------------+--------------+
 * | Block 0 | Block 1 | Block 2 | ...         | Block n-1     |T0|T1|...|Tn-1 |
 * +---------+---------+---------+-------------+---------------+----+----+----+
 * \___________________________________________________________/ \____________/
 * 				( 用户申请内存时，指针返回此处 )                ( 内部管理专用 )
 */

typedef enum 
{
    SRAMIN   = 0,    // 0-内部 AXI 内存池
    SRAMEX   = 1,    // 1-外部 SDRAM 内存池
    SRAM12   = 2,    // 2-内部 SRAM1/SRAM2 内存池
    SRAM4    = 3,    // 3-内部 SRAM4 内存池
    SRAMDTCM = 4,    // 4-内部 DTCM 内存池
    SRAMITCM = 5,    // 5-内部 ITCM 内存池
    RAM_NUMBER       // 自动累计为 6
} mem_pool_index_t;       


/*-----------------------------------------------------------
          1. 内存区域总容量定义 (MAX_SIZE) -- 可修改
-------------------------------------------------------------*/
#define MEM0_MAX_SIZE            (5 * 1024)                        // 0-内部 AXI 内存大小
#define MEM1_MAX_SIZE            (64)                   		// 1-外部 SDRAM 内存大小
#define MEM2_MAX_SIZE            (64)                        // 2-内部 SRAM1/SRAM2 内存大小
#define MEM3_MAX_SIZE            (64)                         // 3-内部 SRAM4 内存大小
#define MEM4_MAX_SIZE            (64)                        // 4-内部 DTCM 内存大小
#define MEM5_MAX_SIZE            (64)                         // 5-内部 ITCM 内存大小

/*-----------------------------------------------------------
          2. 内存管理粒度定义 (BLOCK_SIZE) -- 可修改
-------------------------------------------------------------*/
#define MEM0_BLOCK_SIZE          64                                  // 0-内部 AXI 内存块大小
#define MEM1_BLOCK_SIZE          64                                  // 1-外部 SDRAM 内存块大小
#define MEM2_BLOCK_SIZE          64                                  // 2-内部 SRAM1/SRAM2 内存块大小
#define MEM3_BLOCK_SIZE          64                                  // 3-内部 SRAM4 内存块大小
#define MEM4_BLOCK_SIZE          64                                  // 4-内部 DTCM 内存块大小
#define MEM5_BLOCK_SIZE          64                                  // 5-内部 ITCM 内存块大小

/*-----------------------------------------------------------
          3. 内存管理块数量计算 (BLOCK_NUM) -- 不用修改
-------------------------------------------------------------*/
#define MEM0_BLOCK_NUM           (MEM0_MAX_SIZE / MEM0_BLOCK_SIZE)    // 0-内部 AXI 内存块数量
#define MEM1_BLOCK_NUM           (MEM1_MAX_SIZE / MEM1_BLOCK_SIZE)    // 1-外部 SDRAM 内存块数量
#define MEM2_BLOCK_NUM           (MEM2_MAX_SIZE / MEM2_BLOCK_SIZE)    // 2-内部 SRAM1/SRAM2 内存块数量
#define MEM3_BLOCK_NUM           (MEM3_MAX_SIZE / MEM3_BLOCK_SIZE)    // 3-内部 SRAM4 内存块数量
#define MEM4_BLOCK_NUM           (MEM4_MAX_SIZE / MEM4_BLOCK_SIZE)    // 4-内部 DTCM 内存块数量
#define MEM5_BLOCK_NUM           (MEM5_MAX_SIZE / MEM5_BLOCK_SIZE)    // 5-内部 ITCM 内存块数量




// 内存管理控制器
struct _m_mallco_dev
{
    void (*init)(uint8_t);              	// 初始化
    uint16_t (*perused)(uint8_t);       	// 内存使用率
	uint8_t  memrdy[RAM_NUMBER];          	// 内存管理是否就绪
    uint8_t *mem_block_base[RAM_NUMBER];    // 内存池
    uint32_t *mem_table_base[RAM_NUMBER];   // 内存管理状态表
};
extern struct _m_mallco_dev mallco_dev;


  
void my_mem_init(mem_pool_index_t memx); 
uint16_t my_mem_perused(mem_pool_index_t memx) ; 
void myfree(mem_pool_index_t memx, void *ptr);                  
void *mymalloc(mem_pool_index_t memx, uint32_t size);            
void *myrealloc(mem_pool_index_t memx, void *ptr, uint32_t size);


#endif



