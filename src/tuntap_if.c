#include "syshead.h"
#include "utils.h"
#include "basic.h"

static int tun_fd;
static char* dev;

char *tapaddr = "10.0.0.5";
char *taproute = "10.0.0.0/24";
// 设置路由
static int set_if_route(char *dev, char *cidr)
{
    return run_cmd("ip route add dev %s %s", dev, cidr);
}
// 设置ip 地址
static int set_if_address(char *dev, char *cidr)
{
    return run_cmd("ip address add dev %s local %s", dev, cidr);
}
// 开启网卡
static int set_if_up(char *dev)
{
    return run_cmd("ip link set dev %s up", dev);
}

/* 
 * Taken from Kernel Documentation/networking/tuntap.txt(https://www.kernel.org/doc/Documentation/networking/tuntap.txt)
 * 虚拟网卡资源分配
 */
static int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tap", O_RDWR)) < 0 ) {
        perror("Cannot open TUN/TAP dev\n"
                    "Make sure one exists with " 
                    "'$ mknod /dev/net/tap c 10 200'");
        exit(1);
    }

    CLEAR(ifr);

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - 不提供数据包
     */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if( *dev ) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
        perror("ERR: Could not ioctl tun");
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}
// 读tun设备接收到的数据
int tun_read(char *buf, int len)
{
    return read(tun_fd, buf, len);
}
// 往tun设备里写数据
int tun_write(char *buf, int len)
{
    return write(tun_fd, buf, len);
}
// 初始化tun设备
void tun_init()
{
    dev = calloc(10, 1);
    tun_fd = tun_alloc(dev);

    if (set_if_up(dev) != 0) { // 拉起网卡
        print_err("ERROR when setting up if\n");
    }

    if (set_if_route(dev, taproute) != 0) { // 设置路由
        print_err("ERROR when setting route for if\n");
    }

    if (set_if_address(dev, tapaddr) != 0) { // 设置ip地址
        print_err("ERROR when setting addr for if\n");
    }
}
// 释放tun设备
void free_tun()
{
    free(dev);
}
