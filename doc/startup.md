## 写在开始之前

hf-ip是一个TCP/IP协议栈的实现，在Linux主机上作为单个守护进程运行。可通过配置Linux主机使之通过hf-ip转发数据包来实现网络连接。我使用的是centos7环境。

这里需要说明：hf-ip不是用于生产环境的网络协议栈，也没有支持所有的网络协议栈。所以有可能会有安全方面的隐患这个项目仅是实验性质的，因此不要长运行hf-ip。

用户态tcp/ip协议栈有什么优点呢？可以减少用户态内核态频繁切换带来的开销。当然这个项目主要是为了好玩。

## 编译
使用`make`进行编译
需要先安装依赖`sudo yum install libcap-devel`

执行

```sh
make all
```


这将构建hf-ip本身，同时构建libc包装程序并提供示例应用程序。

在构建时，可能会遇到sudo setcap ...的情况，操作系统向你询问超级用户权限。这是因为hf-ip需要CAP_NET_ADMIN功能才能进行自我设置。设置完成后，它将放弃该功能。

当前，hf-ip还通过ip工具配置Tap接口。因此，需要给它权限：

```sh
which ip
#运行以上指令会出现该程序的路径，一般在 /usr/sbin/ip
sudo setcap cap_net_admin=ep /usr/sbin/ip
#这里是赋予普通用户组也可使用/usr/bin/ip程序
sysctl -w net.ipv6.conf.all.disable_ipv6=1
sysctl -w net.ipv6.conf.default.disable_ipv6=1
#关闭ipv6,该协议栈不支持ipv6
sysctl -w net.ipv6.conf.all.disable_ipv6=1
sysctl -w net.ipv6.conf.default.disable_ipv6=1
```

## hf-ip运行需要的设置

hf-ip使用Linux TAP设备与外界通信。简而言之，在主机Linux的网络堆栈中初始化了tap设备，然后hf-ip可以读取链路层(L2层)的数据帧：

```sh
sudo mknod /dev/net/tap c 10 200
sudo chmod 0666 /dev/net/tap
```


本质上，hf-ip在Tap设备的子网内作为主机运行。因此，为了与其他主机通信,需要将tap设备设置为转发模式：

在我都linux设备上的示例,其中wlp2s0是我的传出接口,而tap0是运行hf-ip程序的tap设备。

```sh
sysctl -w net.ipv4.ip_forward=1 # 在linux中能使用/usr/sbin/ip进行转发
iptables -I INPUT --source 10.0.0.0/24 -j ACCEPT
iptables -t nat -I POSTROUTING --out-interface wlp2s0 -j MASQUERADE
iptables -I FORWARD --in-interface wlp2s0 --out-interface tap0 -j ACCEPT
iptables -I FORWARD --in-interface tap0 --out-interface wlp2s0 -j ACCEPT
```


现在，来自hf-ip（在本例中为10.0.0.4/24）的数据包应由linux主机接口进行NAT，并正确地遍历FORWARD转发链到达主机的输出网关。

有关iptables的信息可以在这个网址下查看  http://www.netfilter.org/documentation/HOWTO/packet-filtering-HOWTO-9.html

## hf-ip使用方法

构建hf-ip并设置主机以hf-ip协议栈转发数据包后，您可以尝试与Internet网通信：

```sh
./hf-ip
```
先做一下测试:
我给tap设备在程序中固定的ip是10.0.0.5
```sh
ping 10.0.0.5
# 能够得到返回包
```
注意：10.0.0.5是tap0 ip地址，而不是我们的网络堆栈ip地址。
根据tap0，我们的网络堆栈（veth）是“外部网络”，(veth为virtual ethernet)
因此我们的程序只是外部网络的仿真器，
我们必须为./hf-ip虚拟netif veth分配一个伪IP地址（10.0.0.1）！

用户空间TCP/IP协议栈程序应该就启动。 现在，测试与提供的应用程序的通信：

```sh
cd tools
./hf-ip.sh ../apps/curl/curl baidu.com 80
```
hf-ip.sh是一个shell脚本，它允许libhfip.so优先于libc套接字API调用。

重要的是，这个脚本可用于任何现有的动态链接应用程序。尝试一下使用curl去访问：
```sh
[root@localhost hf-ip]# curl --version
curl 7.29.0 (x86_64-redhat-linux-gnu) libcurl/7.29.0 NSS/3.44 zlib/1.2.7 libidn/1.28 libssh2/1.8.0
Protocols: dict file ftp ftps gopher http https imap imaps ldap ldaps pop3 pop3s rtsp scp sftp smtp smtps telnet tftp 
Features: AsynchDNS GSS-Negotiate IDN IPv6 Largefile NTLM NTLM_WB SSL libz unix-sockets 

[root@localhost hf-ip]# curl baidu.com
<html>
<meta http-equiv="refresh" content="0;url=http://www.baidu.com/">
</html>
```

让curl使用hf-ip协议栈，得到如下结果:
```sh
[root@localhost tools]$ ./hf-ip curl baidu.com
<html>
<meta http-equiv="refresh" content="0;url=http://www.baidu.com/">
</html>
```
结果是完全一样的。但是，curl调用了libc套接字API，但这些调用被重定向到hf-ip。

尝试浏览Web，使用hf-ip进行数据包传输：

```sh
[root@localhost tools]$ firefox --version
Mozilla Firefox 52.8.0
[root@localhost tools]$ ./hf-ip firefox baidu.com
```

以上。