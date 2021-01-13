#include "syshead.h"
#include "skbuff.h"
#include "list.h"
// 分配sk_buff所需的空间,实例化sk_buff
struct sk_buff *alloc_skb(unsigned int size)
{
    struct sk_buff *skb = malloc(sizeof(struct sk_buff));

    memset(skb, 0, sizeof(struct sk_buff));
    skb->data = malloc(size);
    memset(skb->data, 0, size);

    skb->refcnt = 0;
    skb->head = skb->data;
    skb->end = skb->data + size;

    list_init(&skb->list);

    return skb;
}
// 释放sk_buff
void free_skb(struct sk_buff *skb)
{
    if (skb->refcnt < 1) {
        free(skb->head);
        free(skb);
    }
}
// 给sk_buff链表结构预留更多的空间
void *skb_reserve(struct sk_buff *skb, unsigned int len)
{
    skb->data += len;

    return skb->data;
}
//  skb添加数据
uint8_t *skb_push(struct sk_buff *skb, unsigned int len)
{
    skb->data -= len;
    skb->len += len;

    return skb->data;
}
// 取sk_buff首部地址
uint8_t *skb_head(struct sk_buff *skb)
{
    return skb->head;
}
// 重置sk_buff的首部
void skb_reset_header(struct sk_buff *skb)
{
    skb->data = skb->end - skb->dlen;
    skb->len = skb->dlen;
}
