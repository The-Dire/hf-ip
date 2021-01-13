#ifndef SKBUFF_H_
#define SKBUFF_H_

#include "netdev.h"
#include "route.h"
#include "list.h"
#include <pthread.h>
/*
* sk_buff是通用缓冲区,
* 每个发送或接收的数据包都使用这个数据结构进行处理。
*/

// 为所有网络数据的通用缓冲区结构
struct sk_buff {
    struct list_head list;
    struct rtentry *rt;
    struct netdev *dev;
    int refcnt;
    uint16_t protocol; // 来自驱动程序的包协议
    uint32_t len;   // 缓冲区长度
    uint32_t dlen; 
    uint32_t seq;   // 第一个seq序号
    uint32_t end_seq; // 最后一个seq序号
    uint8_t *end;   // 指向缓冲区尾部
    uint8_t *head;   // 指向skb缓冲区头
    uint8_t *data;  // 指向数据头部的指针
    uint8_t *payload;   // 负载
};
// sk_buff链表结构
struct sk_buff_head {
    struct list_head head;

    uint32_t qlen;
};
// skb_head取sk_buff首部地址的声明在ehernet.h中因为以太网帧结构的实现需要该函数
// 给skb结构分配内存
struct sk_buff *alloc_skb(unsigned int size);
void free_skb(struct sk_buff *skb); // 释放skb内存
uint8_t *skb_push(struct sk_buff *skb, unsigned int len);
uint8_t *skb_head(struct sk_buff *skb); 
void *skb_reserve(struct sk_buff *skb, unsigned int len);
void skb_reset_header(struct sk_buff *skb);
// 得到通用缓冲区队列大小
static inline uint32_t skb_queue_len(const struct sk_buff_head *list)
{
    return list->qlen;
}
// 通用缓冲区队列初始化
static inline void skb_queue_init(struct sk_buff_head *list)
{
    list_init(&list->head);
    list->qlen = 0;
}
// 新增sk_buff节点
static inline void skb_queue_add(struct sk_buff_head *list, struct sk_buff *new, struct sk_buff *next)
{
    list_add_tail(&new->list, &next->list);
    list->qlen += 1;
}
// 从尾部新增一个sk_buff节点
static inline void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *new)
{
    list_add_tail(&new->list, &list->head);
    list->qlen += 1;
}
// 删除一个sk_buff节点
static inline struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
    struct sk_buff *skb = list_first_entry(&list->head, struct sk_buff, list);
    list_del(&skb->list);
    list->qlen -= 1;

    return skb;
}
// 判断sk_buff队列是否为空
static inline int skb_queue_empty(const struct sk_buff_head *list)
{
    return skb_queue_len(list) < 1;
}
// 查询sk_buff队首元素
static inline struct sk_buff *skb_peek(struct sk_buff_head *list)
{
    if (skb_queue_empty(list)) return NULL;
        
    return list_first_entry(&list->head, struct sk_buff, list);
}
// 清空释放sk_buff队列
static inline void skb_queue_free(struct sk_buff_head *list)
{
    struct sk_buff *skb = NULL;
    
    while ((skb = skb_peek(list)) != NULL) {
        skb_dequeue(list);
        skb->refcnt--;
        free_skb(skb);
    }
}

#endif
