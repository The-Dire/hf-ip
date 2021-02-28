#include "syshead.h"
#include "skbuff.h"
#include "utils.h"
#include "ip.h"
#include "dst.h"
#include "route.h"
// ip头校验
void ip_send_check(struct iphdr *ihdr)
{
    uint32_t csum = checksum(ihdr, ihdr->ihl * 4, 0);
    ihdr->csum = csum;
}
// ip数据包发送函数:icmp和tcp都是基于此的
int ip_output(struct sock *sk, struct sk_buff *skb)
{
    struct rtentry *rt;
    struct iphdr *ihdr = ip_hdr(skb);

    rt = route_lookup(sk->daddr); // 路由查找

    if (!rt) {
        // TODO: 目的ip无法到达的处理
        print_err("IP output route lookup fail\n");
        return -1;
    }

    skb->dev = rt->dev;
    skb->rt = rt;
    // 放入到skbuff缓冲区中
    skb_push(skb, IP_HDR_LEN);

    ihdr->version = IPV4;
    ihdr->ihl = 0x05;
    ihdr->tos = 0;
    ihdr->len = skb->len;
    ihdr->id = ihdr->id;
    ihdr->frag_offset = 0x4000;
    ihdr->ttl = 64;
    ihdr->proto = skb->protocol;
    ihdr->saddr = skb->dev->addr;
    ihdr->daddr = sk->daddr;
    ihdr->csum = 0;

    ip_dbg("out", ihdr);

    ihdr->len = htons(ihdr->len);
    ihdr->id = htons(ihdr->id);
    ihdr->daddr = htonl(ihdr->daddr);
    ihdr->saddr = htonl(ihdr->saddr);
    ihdr->csum = htons(ihdr->csum);
    ihdr->frag_offset = htons(ihdr->frag_offset);

    ip_send_check(ihdr);

    return dst_neigh_output(skb);
}
