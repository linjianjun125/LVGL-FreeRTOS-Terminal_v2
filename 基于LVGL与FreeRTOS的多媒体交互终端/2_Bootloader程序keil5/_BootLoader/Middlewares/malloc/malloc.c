#include "malloc.h"



/*-----------------------------------------------------------
               1. 内存池基地址定义 (Data Region)
-------------------------------------------------------------*/
// 0: AXI SRAM (D1 域)
static __align(64) uint8_t mem0_block_base[MEM0_MAX_SIZE];    

// 1: SDRAM (外部)
static __align(64) uint8_t mem1_block_base[MEM1_MAX_SIZE] __attribute__((at(0xc0000000)));

// 2: SRAM1/2 (D2 域)
static __align(64) uint8_t mem2_block_base[MEM2_MAX_SIZE] __attribute__((at(0x30000000))); 

// 3: SRAM4 (D3 域)
static __align(64) uint8_t mem3_block_base[MEM3_MAX_SIZE] __attribute__((at(0x38000000)));

// 4: DTCM
static __align(64) uint8_t mem4_block_base[MEM4_MAX_SIZE] __attribute__((at(0x20000000))); 

// 5: ITCM
static __align(64) uint8_t mem5_block_base[MEM5_MAX_SIZE] __attribute__((at(0x00000000))); 

/*-----------------------------------------------------------
                2. 内存管理表定义 (Management Table)
-------------------------------------------------------------*/
static uint32_t mem0_table_base[MEM0_BLOCK_NUM];              
static uint32_t mem1_table_base[MEM1_BLOCK_NUM] __attribute__((at(0xc0000000 + MEM1_MAX_SIZE))); 
static uint32_t mem2_table_base[MEM2_BLOCK_NUM] __attribute__((at(0x30000000 + MEM2_MAX_SIZE))); 
static uint32_t mem3_table_base[MEM3_BLOCK_NUM] __attribute__((at(0x38000000 + MEM3_MAX_SIZE))); 
static uint32_t mem4_table_base[MEM4_BLOCK_NUM] __attribute__((at(0x20000000 + MEM4_MAX_SIZE)));
static uint32_t mem5_table_base[MEM5_BLOCK_NUM] __attribute__((at(0x00000000 + MEM5_MAX_SIZE)));



/*-----------------------------------------------------------
               3. 内存管理参数表 (用于统一接口调用)
-------------------------------------------------------------*/
// 各内存池的管理块数量
const uint32_t mem_block_number[RAM_NUMBER] = {
    MEM0_BLOCK_NUM, MEM1_BLOCK_NUM, MEM2_BLOCK_NUM, 
    MEM3_BLOCK_NUM, MEM4_BLOCK_NUM, MEM5_BLOCK_NUM
};

// 各内存池的块大小
const uint32_t mem_block_size[RAM_NUMBER] = {
    MEM0_BLOCK_SIZE, MEM1_BLOCK_SIZE, MEM2_BLOCK_SIZE, 
    MEM3_BLOCK_SIZE, MEM4_BLOCK_SIZE, MEM5_BLOCK_SIZE
};

// 各内存池的总容量
const uint32_t mem_size[RAM_NUMBER] = {
    MEM0_MAX_SIZE, MEM1_MAX_SIZE, MEM2_MAX_SIZE, 
    MEM3_MAX_SIZE, MEM4_MAX_SIZE, MEM5_MAX_SIZE
};


// 内存管理控制器
struct _m_mallco_dev mallco_dev =
{
    my_mem_init,                                            // 内存初始化
    my_mem_perused,                                         // 内存使用率
	{0, 0, 0, 0, 0, 0},                                     // 内存管理未就绪
    {mem0_block_base, mem1_block_base, mem2_block_base, 
	 mem3_block_base, mem4_block_base, mem5_block_base},	// 内存池
    {mem0_table_base, mem1_table_base, mem2_table_base, 
	 mem3_table_base, mem4_table_base, mem5_table_base }	// 内存管理状态表
};


static void my_mem_set(void *s, uint8_t c, uint32_t count); 
static void my_mem_copy(void *des, void *src, uint32_t n);   
static uint32_t my_mem_malloc(uint8_t index, uint32_t size); 
static uint8_t my_mem_free(uint8_t index, uint32_t offset);   


/**
 * @brief   内存管理初始化
 * @param   index : 所属内存块
 * @retval  无
 */
void my_mem_init(mem_pool_index_t index)
{
    my_mem_set(mallco_dev.mem_table_base[index], 0, mem_block_number[index] * 4); 	/* 内存状态表数据清零 */
    mallco_dev.memrdy[index] = 1;                                  					/* 内存管理初始化OK */
}

/**
 * @brief   获取内存使用率
 * @param   index : 所属内存块
 * @retval  使用率(扩大了10倍,0~1000,代表0.0%~100.0%)
 */
uint16_t my_mem_perused(mem_pool_index_t index)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < mem_block_number[index]; i++)
    {
        if (mallco_dev.mem_table_base[index][i])
        {
            used++;
        }
    }

    return (used * 1000) / (mem_block_number[index]);
}


/**
 * @brief   释放内存
 * @param   index : 所属内存块
 * @param   ptr  : 内存首地址
 * @retval  无
 */
void myfree(mem_pool_index_t index, void *ptr)
{
    uint32_t offset;

    if (ptr == NULL) return;

	{
		offset = (uint32_t)ptr - (uint32_t)mallco_dev.mem_block_base[index];
		my_mem_free(index, offset); 
	}
}

/**
 * @brief   分配内存(外部调用)
 * @param   index : 所属内存块
 * @param   size : 要分配的内存大小(字节)
 * @retval  分配到的内存首地址.
 */
void *mymalloc(mem_pool_index_t index, uint32_t size)
{
    uint32_t offset;
	
	{
		offset = my_mem_malloc(index, size);
	}
	
	
    if (offset == 0xFFFFFFFF) return NULL;
    return (void *)((uint32_t)mallco_dev.mem_block_base[index] + offset);
}


/**
 * @brief   重新分配内存(外部调用)
 * @param   index : 所属内存块
 * @param   *ptr : 旧内存首地址
 * @param   size : 要分配的内存大小(字节)
 * @retval  新分配到的内存首地址.
 */
void *myrealloc(mem_pool_index_t index, void *ptr, uint32_t size)
{
    uint32_t offset;
	
	{
		offset = my_mem_malloc(index, size);
		if (offset == 0xFFFFFFFF) return NULL; 
        my_mem_copy((void *)((uint32_t)mallco_dev.mem_block_base[index] + offset), ptr, size); 
        myfree(index, ptr);    
	}		

	
    return (void *)((uint32_t)mallco_dev.mem_block_base[index] + offset);           
}




/*--------------------------------------
				static函数
-----------------------------------------*/
/**
 * @brief   复制内存
 * @param   *des : 目的地址
 * @param   *src : 源地址
 * @param   n    : 需要复制的内存长度(字节为单位)
 * @retval  无
 */
static void my_mem_copy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;
    while (n--)
        *xdes++ = *xsrc++;
}

/**
 * @brief   设置内存值
 * @param   *s    : 内存首地址
 * @param   c     : 要设置的值
 * @param   count : 需要设置的内存大小(字节为单位)
 * @retval  无
 */
static void my_mem_set(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;
    while (count--)
        *xs++ = c;
}


/**
 * @brief   内存分配(内部调用)
 * @param   index : 所属内存块
 * @param   size : 要分配的内存大小(字节)
 * @retval  内存偏移地址
 *   @arg   0 ~ 0xFFFFFFFE : 有效的内存偏移地址
 *   @arg   0xFFFFFFFF: 无效的内存偏移地址
 */
uint32_t my_mem_malloc(uint8_t index, uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb;     /* 需要的内存块数 */
    uint32_t cmemb = 0; /* 连续空内存块数 */
    uint32_t i;

    if (!mallco_dev.memrdy[index])
    {
        mallco_dev.init(index); /* 未初始化,先执行初始化 */
    }

    if (size == 0)
    {
        return 0XFFFFFFFF; /* 不需要分配 */
    }
    nmemb = size / mem_block_size[index]; /* 获取需要分配的连续内存块数 */

    if (size % mem_block_size[index])
    {
        nmemb++;
    }

    for (offset = mem_block_number[index] - 1; offset >= 0; offset--) /* 搜索整个内存控制区 */
    {
        if (!mallco_dev.mem_table_base[index][offset])
        {
            cmemb++; /* 连续空内存块数增加 */
        }
        else
        {
            cmemb = 0; /* 连续内存块清零 */
        }

        if (cmemb == nmemb) /* 找到了连续nmemb个空内存块 */
        {
            for (i = 0; i < nmemb; i++) /* 标注内存块非空  */
            {
                mallco_dev.mem_table_base[index][offset + i] = nmemb;
            }

            return (offset * mem_block_size[index]); /* 返回偏移地址  */
        }
    }

    return 0XFFFFFFFF; /* 未找到符合分配条件的内存块 */
}

/**
 * @brief   释放内存(内部调用)
 * @param   index   : 所属内存块
 * @param   offset : 内存地址偏移
 * @retval  释放结果
 *   @arg   0, 释放成功;1, 释放失败;2, 超区域了(失败);
 */
uint8_t my_mem_free(uint8_t index, uint32_t offset)
{
    int i;

    if (!mallco_dev.memrdy[index]) /* 未初始化,先执行初始化 */
    {
        mallco_dev.init(index);
        return 1; /* 未初始化 */
    }

    if (offset < mem_size[index]) /* 偏移在内存池内. */
    {
        int ind = offset / mem_block_size[index];      /* 偏移所在内存块号码 */
        int nmemb = mallco_dev.mem_table_base[index][ind]; /* 内存块数量 */

        for (i = 0; i < nmemb; i++) /* 内存块清零 */
        {
            mallco_dev.mem_table_base[index][ind + i] = 0;
        }

        return 0;
    }
    else
    {
        return 2; /* 偏移超区了. */
    }
}
