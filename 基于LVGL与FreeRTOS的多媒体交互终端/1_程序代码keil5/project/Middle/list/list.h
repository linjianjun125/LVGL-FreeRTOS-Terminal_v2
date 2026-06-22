#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
 * struct list_node - 双向链表节点零件
 * @next: 指向后继节点
 * @prev: 指向前驱节点
 */
typedef struct list_node {
    struct list_node *next;
    struct list_node *prev;
} list_node_t;

/**
 * struct list - 链表容器/管理头
 * @root: 哨兵节点
 * @count: 节点总数
 * @member_offset: 成员偏移量
 * @name: 链表名称
 */
typedef struct list {
    list_node_t root;
    int count;
    size_t member_offset;
    char name[32];
} list_t;

/* --- 转换与初始化宏 --- */

/**
 * LIST_NODE_GET_OBJ - 根据节点指针获取业务对象地址
 * @node: 节点指针 (list_node_t *)
 * @list_ptr: 链表容器指针 (list_t *)
 */
#define LIST_NODE_GET_OBJ(node, list_ptr) \
    (((node) == NULL || (node) == &((list_ptr)->root)) ? \
    NULL : (void *)((char *)(node) - (list_ptr)->member_offset))

/**
 * LIST_DECLARE - 静态声明并初始化链表变量
 * @list_name: 变量标识符
 * @name: 链表的逻辑名称
 * @type: 业务结构体类型
 * @member: 结构体中 list_node_t 成员的名字
 */
#define LIST_DECLARE(list_name, name, type, member) \
    list_t list_name = { \
        .root = { .next = &list_name.root, .prev = &list_name.root }, \
        .count = 0, \
        .member_offset = offsetof(type, member), \
        .name = name \
    }

/**
 * LIST_INIT - 运行时初始化链表容器
 * @list_ptr: 指向 list_t 的指针
 * @name: 链表的逻辑名称
 * @type: 业务结构体类型
 * @member: 结构体中 list_node_t 成员的名字
 */
#define LIST_INIT(list_ptr, name, type, member) do { \
    (list_ptr)->root.next = &(list_ptr)->root; \
    (list_ptr)->root.prev = &(list_ptr)->root; \
    (list_ptr)->count = 0; \
    (list_ptr)->member_offset = offsetof(type, member); \
    if (name) { \
        strncpy((list_ptr)->name, name, 31); \
        (list_ptr)->name[31] = '\0'; \
    } else { \
        (list_ptr)->name[0] = '\0'; \
    } \
} while(0)

/* --- 操作函数 --- */

/**
 * list_is_valid_node - 校验节点在链表中的合法性
 * @list: 指向 list_t 的指针
 * @node: 待校验的节点指针
 */
static inline int list_is_valid_node(list_t *list, list_node_t *node) {
    if (!node || node == &list->root) return 0;
    if (node->next == NULL || node->prev == NULL ||
        node->next->prev != node || node->prev->next != node) {
        return 0;
    }
    return 1;
}

/**
 * list_add_head - 将节点插入到链表头部
 * @list: 链表容器指针
 * @new_node: 待插入的新节点指针
 */
static inline void list_add_head(list_t *list, list_node_t *new_node) {
    list_node_t *next = list->root.next;
    new_node->next = next;
    new_node->prev = &list->root;
    next->prev = new_node;
    list->root.next = new_node;
    list->count++;
}

/**
 * list_add_tail - 将节点插入到链表末尾
 * @list: 链表容器指针
 * @new_node: 待插入的新节点指针
 */
static inline void list_add_tail(list_t *list, list_node_t *new_node) {
    list_node_t *prev = list->root.prev;
    new_node->next = &list->root;
    new_node->prev = prev;
    prev->next = new_node;
    list->root.prev = new_node;
    list->count++;
}

/**
 * list_delete_node - 从链表中移除指定节点
 * @list: 链表容器指针
 * @node: 待移除的节点指针
 */
static inline void list_delete_node(list_t *list, list_node_t *node) {
    if (list->count == 0 || !node || node == &list->root) return;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = NULL;
    node->prev = NULL;
    list->count--;
}

/**
 * list_get_first_node - 获取链表首个节点
 * @list: 链表容器指针
 */
static inline list_node_t* list_get_first_node(list_t *list) {
    return (list->count > 0) ? list->root.next : NULL;
}

/**
 * list_get_last_node - 获取链表最后一个节点
 * @list: 链表容器指针
 */
static inline list_node_t* list_get_last_node(list_t *list) {
    return (list->count > 0) ? list->root.prev : NULL;
}

/**
 * list_get_next_node - 循环获取当前节点的下一个业务节点
 * @list: 链表容器指针
 * @node: 当前参考节点指针
 */
static inline list_node_t* list_get_next_node(list_t *list, list_node_t *node) {
    if (!list_is_valid_node(list, node)) return NULL;
    list_node_t *next = node->next;
    if (next == &list->root) next = next->next;
    return (next == &list->root) ? NULL : next; 
}

/**
 * list_get_prev_node - 循环获取当前节点的上一个业务节点
 * @list: 链表容器指针
 * @node: 当前参考节点指针
 */
static inline list_node_t* list_get_prev_node(list_t *list, list_node_t *node) {
    if (!list_is_valid_node(list, node)) return NULL;
    list_node_t *prev = node->prev;
    if (prev == &list->root) prev = prev->prev;
    return (prev == &list->root) ? NULL : prev; 
}

/* --- 遍历宏 --- */

/**
 * LIST_FOR_EACH_NODE - 遍历链表中的所有节点
 * @pos: 迭代变量 (list_node_t *)
 * @list_ptr: 指向 list_t 的指针
 */
#define LIST_FOR_EACH_NODE(pos, list_ptr) \
    for (pos = (list_ptr)->root.next; pos != &(list_ptr)->root; pos = pos->next)

/**
 * LIST_FOR_EACH_NODE_SAFE - 安全遍历，允许在循环内删除节点
 * @pos: 迭代变量
 * @n: 临时存储下一节点的备份指针
 * @list_ptr: 指向 list_t 的指针
 */
#define LIST_FOR_EACH_NODE_SAFE(pos, n, list_ptr) \
    for (pos = (list_ptr)->root.next, n = (pos)->next; \
         pos != &(list_ptr)->root; \
         pos = n, n = (pos)->next)

#endif /* _LIST_H */

