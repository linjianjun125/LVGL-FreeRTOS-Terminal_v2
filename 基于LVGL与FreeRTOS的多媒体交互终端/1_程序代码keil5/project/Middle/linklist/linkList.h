#ifndef __LINK_LIST_H
#define __LINK_LIST_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


struct LinkList;

// --- 链表节点结构定义 ---
typedef struct LinkList_Item
{
    struct LinkList_Item* pxPrev;      // 指向前驱节点
    struct LinkList_Item* pxNext;      // 指向后继节点
    struct LinkList* pxContainer;      // 指向所属链表容器
    void* pvData;                      // 指向用户数据载荷
} LinkList_Item;

// --- 链表管理结构定义 ---
typedef struct LinkList
{
    size_t uxNumberOfItems;            // 链表有效节点数量
    size_t uxDataSize;                 // 载荷数据的大小（字节）
    LinkList_Item* pxIndex;            // 用于遍历的索引指针
    LinkList_Item* pxHead;             // 指向头节点（哨兵）
    LinkList_Item  xHeadItem;          // 头节点实例（哨兵节点，不存储数据）
} LinkList;

// --- API 函数声明 ---
void List_Init(LinkList* pxList, size_t uxDataSize);
LinkList_Item* List_Insert(LinkList* const pxList);
LinkList_Item* List_HeadInsert(LinkList* const pxList);
bool List_Remove(LinkList_Item* const pxItemToRemove);

uint8_t List_SetNowItem(LinkList* const pxList, LinkList_Item* pxNowItem);
LinkList_Item* List_GetNowItem(LinkList* const pxList);
LinkList_Item* List_GetNextItem(LinkList* const pxList);
LinkList_Item* List_GetPrevItem(LinkList* const pxList);

void* List_ListItemGetData(LinkList_Item* pxListItem);
bool List_ListItemWriteData(LinkList_Item* pxListItem, void* data);

// --- 快捷工具宏 ---
#define List_GetListFirstItem( pxList )         ( ( pxList )->pxHead->pxNext )
#define List_GetListEndItem( pxList )           ( ( pxList )->pxHead->pxPrev )
#define List_ListIsEmpty( pxList )              ( ( ( pxList )->uxNumberOfItems == 0 ) ? true : false )
#define List_CurrentNumberOfItems( pxList )     ( ( pxList )->uxNumberOfItems )

// 从头向尾遍历循环宏
#define List_HeadFor(pxList, pxListItem)        for(pxListItem = List_GetListFirstItem(pxList); \
                                                    pxListItem != ( (pxList)->pxHead); \
                                                    pxListItem = pxListItem->pxNext)

// 从尾向头遍历循环宏
#define List_TailFor(pxList, pxListItem)        for(pxListItem = List_GetListEndItem(pxList); \
                                                    pxListItem != ( (pxList)->pxHead); \
                                                    pxListItem = pxListItem->pxPrev)

#endif

