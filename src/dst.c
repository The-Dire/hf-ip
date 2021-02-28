#include "syshead.h"
#include "dst.h"
#include "ip.h"
#include "arp.h"

int dst_neigh_output(struct sk_buff *skb)
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct netdev *netdev = skb->dev;
    struct rtentry *rt = skb->rt;
    uint32_t daddr = ntohl(iphdr->daddr);
    uint32_t saddr = ntohl(iphdr->saddr);

    uint8_t *dmac;

    if (rt->flags & RT_GATEWAY) {
        daddr = rt->gateway;
    }
    // 目的mac地址获取
    dmac = arp_get_hwaddr(daddr);
    // 目的mac地址存在
    if (dmac) {
        return netdev_transmit(skb, dmac, ETH_P_IP);// 网卡中传入数据,该函数实际是把变为skbuff然后加封装写入到tun设备中
    } else {
        arp_request(saddr, daddr, netdev); // 没有源地址说明需要arp请求对端给出其mac地址,发送arp请求

        /* 通知上层通信未发送，请稍后重试 */
        return -1;
    }
}
