# hf-TCP-IP-stack
创建一个用户态的TCP/IP协议栈。基于Linux内核的TCP/IP协议栈,Xiaochen Wang's TCP/IP stack 和 Level-IP这三个项目构建的。同时它也是一个中文教程，可以帮助自己(或者看到此教程的人)更好的理解计算机网络,更好的理解协议栈。

用户态协议栈运行的收发流程参考 [net_topylogy](doc/net_topylogy.md) 

具体到实现通过用户态协议栈传输数据,只需要把数据封好转换为skb(skb可以理解为数据流缓存)写入tap设备即可。(代码在netdev.c的netdev_transmit函数)。接收数据在(netdev.c的netdev_receive函数),该函数就是调用比如IP协议调用ip_rcv,然后ip_rcv里通过判断语句调用tcp_in(tcp包处理函数)等操作。

### 目录

- 1.1 [用C语言从零写一个TCP/IP用户态协议栈1：Ethernet 和 arp协议的实现](doc/01.md)
- 1.2 [用C语言从零写一个TCP/IP用户态协议栈2：IPV4&ICMPv4](doc/02.md)
- 1.3 [用C语言从零写一个TCP/IP用户态协议栈3：TCP基础知识和三次握手](doc/03.md)
- 1.4 [用C语言从零写一个TCP/IP用户态协议栈4：TCP数据流和套接字API](doc/04.md)
- 1.5 [用C语言从零写一个TCP/IP用户态协议栈5:TCP重传](doc/05.md)
