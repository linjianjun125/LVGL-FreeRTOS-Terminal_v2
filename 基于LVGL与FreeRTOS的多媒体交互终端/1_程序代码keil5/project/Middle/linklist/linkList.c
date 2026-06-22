#include "linkList.h"
#include "../../middle/malloc/malloc.h" // 你的自定义内存管理
#include <stdio.h>

// --- 链表初始化 ---
void List_Init(LinkList* pxList, size_t uxDataSize)
{
    // 哨兵节点自循环，形成空的双向循环链表
    pxList->xHeadItem.pxPrev = (LinkList_Item*)&(pxList->xHeadItem);
    pxList->xHeadItem.pxNext = (LinkList_Item*)&(pxList->xHeadItem);
    pxList->xHeadItem.pxContainer = pxList;
    pxList->xHeadItem.pvData = NULL; // 哨兵不携带数据载荷

    pxList->pxHead = (LinkList_Item*)&(pxList->xHeadItem);
    pxList->pxIndex = (LinkList_Item*)&(pxList->xHeadItem);

    pxList->uxDataSize = uxDataSize;
    pxList->uxNumberOfItems = 0U;
}

// --- 在链表末尾插入新节点 ---
LinkList_Item* List_Insert(LinkList* const pxList)
{
    LinkList_Item* const pxHead = pxList->pxHead;
    
    // 1. 申请节点结构体内存
    LinkList_Item* pxNewListItem = (LinkList_Item*)mymalloc(SRAMIN, sizeof(LinkList_Item));
    if (pxNewListItem == NULL) return NULL;

    // 2. 申请载荷数据内存 (修正: 直接使用 uxDataSize，而非 sizeof(uxDataSize))
    pxNewListItem->pvData = mymalloc(SRAMIN, pxList->uxDataSize);
    if (pxNewListItem->pvData == NULL) {
        myfree(SRAMIN, pxNewListItem);
        return NULL;
    }

    // 3. 链表挂载逻辑（插入到 pxHead 之前，即链表尾部）
    pxNewListItem->pxPrev = pxHead->pxPrev;
    pxNewListItem->pxNext = pxHead;
    pxHead->pxPrev->pxNext = pxNewListItem;
    pxHead->pxPrev = pxNewListItem;

    pxNewListItem->pxContainer = pxList;
    (pxList->uxNumberOfItems)++;

    return pxNewListItem;
}

// --- 在链表开头插入新节点 ---
LinkList_Item* List_HeadInsert(LinkList* const pxList)
{
    LinkList_Item* const pxHead = pxList->pxHead;
    
    LinkList_Item* pxNewListItem = (LinkList_Item*)mymalloc(SRAMIN, sizeof(LinkList_Item));
    if (pxNewListItem == NULL) return NULL;

    pxNewListItem->pvData = mymalloc(SRAMIN, pxList->uxDataSize);
    if (pxNewListItem->pvData == NULL) {
        myfree(SRAMIN, pxNewListItem);
        return NULL;
    }

    // 插入到 pxHead 之后，即链表头部
    pxNewListItem->pxNext = pxHead->pxNext;
    pxNewListItem->pxPrev = pxHead;
    pxHead->pxNext->pxPrev = pxNewListItem;
    pxHead->pxNext = pxNewListItem;

    pxNewListItem->pxContainer = pxList;
    (pxList->uxNumberOfItems)++;

    return pxNewListItem;
}

// --- 移除指定节点 ---
bool List_Remove(LinkList_Item* const pxItemToRemove)
{
    if (pxItemToRemove == NULL || pxItemToRemove->pxContainer == NULL) return false;

    LinkList* pxList = pxItemToRemove->pxContainer;

    // 如果待删除节点刚好是遍历索引 pxIndex，需先移动索引，防止野指针
    if (pxList->pxIndex == pxItemToRemove) {
        pxList->pxIndex = pxItemToRemove->pxPrev;
    }

    // 释放载荷数据和节点结构体
    if (pxItemToRemove->pvData != NULL) {
        myfree(SRAMIN, pxItemToRemove->pvData);
    }

    // 调整前后节点指针
    pxItemToRemove->pxPrev->pxNext = pxItemToRemove->pxNext;
    pxItemToRemove->pxNext->pxPrev = pxItemToRemove->pxPrev;

    myfree(SRAMIN, pxItemToRemove);
    (pxList->uxNumberOfItems)--;
    
    return true;
}

// --- 设置当前的遍历位置 ---
uint8_t List_SetNowItem(LinkList* const pxList, LinkList_Item* pxNowItem)
{
    if (pxList == NULL || pxNowItem == NULL || pxNowItem->pxContainer != pxList) return 1;
    pxList->pxIndex = pxNowItem;
    return 0;
}

// --- 获取当前节点 (若在哨兵位则自动跳到首个有效节点) ---
LinkList_Item* List_GetNowItem(LinkList* const pxList)
{
    if (pxList == NULL || pxList->uxNumberOfItems == 0) return NULL;
    
    LinkList_Item* ListItem = pxList->pxIndex;
    if (ListItem == pxList->pxHead) {
        ListItem = ListItem->pxNext;
    }
    
    pxList->pxIndex = ListItem;
    return (ListItem == pxList->pxHead) ? NULL : ListItem;
}

// --- 获取下一个节点 (自动跳过哨兵) ---
LinkList_Item* List_GetNextItem(LinkList* const pxList)
{
    if (pxList == NULL || pxList->uxNumberOfItems == 0) return NULL;

    LinkList_Item* ListItem = pxList->pxIndex->pxNext;
    if (ListItem == pxList->pxHead) {
        ListItem = ListItem->pxNext;
    }

    pxList->pxIndex = ListItem;
    return (ListItem == pxList->pxHead) ? NULL : ListItem;
}

// --- 获取上一个节点 (自动跳过哨兵) ---
LinkList_Item* List_GetPrevItem(LinkList* const pxList)
{
    if (pxList == NULL || pxList->uxNumberOfItems == 0) return NULL;

    LinkList_Item* ListItem = pxList->pxIndex->pxPrev;
    if (ListItem == pxList->pxHead) {
        ListItem = ListItem->pxPrev;
    }

    pxList->pxIndex = ListItem;
    return (ListItem == pxList->pxHead) ? NULL : ListItem;
}

// --- 读写节点载荷数据 ---
bool List_ListItemWriteData(LinkList_Item* pxListItem, void* data)
{
    if (pxListItem == NULL || pxListItem->pvData == NULL || data == NULL) return false;
    memcpy(pxListItem->pvData, data, pxListItem->pxContainer->uxDataSize);
    return true;
}

void* List_ListItemGetData(LinkList_Item* pxListItem)
{
    return (pxListItem == NULL) ? NULL : pxListItem->pvData;
}

