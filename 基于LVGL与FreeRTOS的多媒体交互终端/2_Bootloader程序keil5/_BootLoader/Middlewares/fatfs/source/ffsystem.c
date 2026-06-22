#include "ff.h"
#include "../middlewares/malloc/malloc.h"
#include "sys.h"




/**
 * @brief       动态分配内存
 * @param       size : 要分配的内存大小(字节)
 * @retval      分配到的内存首地址.
 */
void *ff_memalloc (UINT size)
{
    return (void*)mymalloc(SRAMIN, size);
}

/**
 * @brief       释放内存
 * @param       mf  : 内存首地址
 * @retval      无
 */
void ff_memfree (void* mf)
{
    myfree(SRAMIN, mf);
}

















