#include "syshead.h"
#include "utils.h"
#include "skbuff.h"
#include "netdev.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
#include "tuntap_if.h"
#include "basic.h"

struct netdev *loop;
struct netdev *netdev;
extern int running;
// 给网卡设备分配空间,实例化网卡
static struct netdev *netdev_alloc(char *addr, char *hwaddr, uint32_t mtu)
{
    struct netdev *dev = malloc(sizeof(struct netdev));

    dev->addr = ip_parse(addr);

    sscanf(hwaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dev->hwaddr[0],
                                                    &dev->hwaddr[1],
                                                    &dev->hwaddr[2],
                                                    &dev->hwaddr[3],
                                                    &dev->hwaddr[4],
                                                    &dev->hwaddr[5]);

    dev->addr_len = 6;
    dev->mtu = mtu;

    return dev;
}
// 初始化网卡设备
void netdev_init(char *addr, char *hwaddr)
{
    loop = netdev_alloc("127.0.0.1", "00:00:00:00:00:00", 1500);
    netdev = netdev_alloc("10.0.0.4", "00:0c:29:6d:50:25", 1500);
}
// 网卡设备传输数据函数
int netdev_transmit(struct sk_buff *skb, uint8_t *dst_hw, uint16_t ethertype)
{
    struct netdev *dev;
    struct eth_hdr *hdr;
    int ret = 0;

    dev = skb->dev; 

    skb_push(skb, ETH_HDR_LEN); // 把ethernet包放入到skb结构中

    hdr = (struct eth_hdr *)skb->data;

    memcpy(hdr->dmac, dst_hw, dev->addr_len);
    memcpy(hdr->smac, dev->hwaddr, dev->addr_len);

    hdr->ethertype = htons(ethertype);
    eth_dbg("out", hdr);

    ret = tun_write((char *)skb->data, skb->len); // skb结构写入tun设备

    return ret;
}
// 网卡接收数据函数
static int netdev_receive(struct sk_buff *skb)
{
    struct eth_hdr *hdr = eth_hdr(skb);

    eth_dbg("in", hdr);

    switch (hdr->ethertype) {
        case ETH_P_ARP:
            arp_rcv(skb); // 判断为arp协议,走arp协议处理流程
            break;
        case ETH_P_IP:
            ip_rcv(skb); // ip协议走ip协议流程
            break;
        case ETH_P_IPV6:
        default:
            printf("Unsupported ethertype %x\n", hdr->ethertype);
            free_skb(skb);
            break;
    }

    return 0;
}
// 网卡循环接收网络流量
void *netdev_rx_loop()
{
    while (running) {
        struct sk_buff *skb = alloc_skb(BUFLEN); // 给网络流量分配skb结构
        
        if (tun_read((char *)skb->data, BUFLEN) < 0) {  // 读取tun中得到的数据存放到skb->data里
            perror("ERR: Read from tun_fd");
            free_skb(skb);  // 释放skb流量
            return NULL;
        }

        netdev_receive(skb); // 进入网卡流量接收处理流程
    }

    return NULL;
}
// 获取虚拟网卡设备
struct netdev* netdev_get(uint32_t sip)
{
    if (netdev->addr == sip) {
        return netdev;
    } else {
        return NULL;
    }
}

void free_netdev()
{
    free(loop);
    free(netdev);
}
