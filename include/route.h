#ifndef _ROUTE_H
#define _ROUTE_H

#include "list.h"

#define RT_LOOPBACK 0x01
#define RT_GATEWAY  0x02
#define RT_HOST     0x04
#define RT_REJECT   0x08
#define RT_UP       0x10
// 路由的数据结构
struct rtentry {
    struct list_head list;
    uint32_t dst;
    uint32_t gateway;   // 网关
    uint32_t netmask;   // 子网掩码
    uint8_t flags;
    uint32_t metric;
    struct netdev *dev; // 虚拟设备实体
};
// 初始化路由
void route_init();
struct rtentry *route_lookup(uint32_t daddr); // 路由查找
void free_routes(); // 释放路由器资源

#endif
