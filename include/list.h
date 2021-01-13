#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>
// 链表结构
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD(name) \
    struct list_head name = { &(name), &(name) }
// 初始化链表
static inline void list_init(struct list_head *head)
{
    head->prev = head->next = head;
}
// 头部新插入一个结点
static inline void list_add(struct list_head *new, struct list_head *head)
{
    head->next->prev = new;
    new->next = head->next;
    new->prev = head;
    head->next = new;
}
// 尾部新插入一个结点
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    head->prev->next = new;
    new->prev = head->prev;
    new->next = head;
    head->prev = new;
}
// 删除链表elem的元素
static inline void list_del(struct list_head *elem)
{
    struct list_head *prev = elem->prev;
    struct list_head *next = elem->next;

    prev->next = next;
    next->prev = prev;
}
// 访问链表的元素
#define list_entry(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))
// 获取链表第一个元素
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)
// 遍历链表
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)
// 安全遍历
#define list_for_each_safe(pos, p, head)    \
    for (pos = (head)->next, p = pos->next; \
         pos != (head);                     \
         pos = p, p = pos->next)
// 判断链表是否为空
static inline int list_empty(struct list_head *head)
{
    return head->next == head;
}

#endif
