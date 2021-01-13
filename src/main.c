#include "syshead.h"
#include "basic.h"
#include "cli.h"
#include "tuntap_if.h"
#include "utils.h"
#include "ipc.h"
#include "timer.h"
#include "route.h"
#include "ethernet.h"
#include "arp.h"
#include "tcp.h"
#include "netdev.h"
#include "ip.h"

#define MAX_CMD_LENGTH 6

typedef void (*sighandler_t)(int);

#define THREAD_CORE 0
#define THREAD_TIMERS 1
#define THREAD_IPC 2
#define THREAD_SIGNAL 3
static pthread_t threads[4];

int running = 1;
sigset_t mask;
// 创建线程
static void create_thread(pthread_t id, void *(*func) (void *))
{
    if (pthread_create(&threads[id], NULL,
                       func, NULL) != 0) {
        print_err("Could not create core thread\n");
    }
}
// 关闭拉起线程
static void *stop_stack_handler(void *arg)
{
    int err, signo;

    for (;;) {
        err = sigwait(&mask, &signo);
        if (err != 0) {
            print_err("Sigwait failed: %d\n", err);
        }

        switch (signo) {
        case SIGINT:
        case SIGQUIT:
            running = 0;
            pthread_cancel(threads[THREAD_IPC]);
            pthread_cancel(threads[THREAD_CORE]);
            pthread_cancel(threads[THREAD_TIMERS]);
            return 0;
        default:
            printf("Unexpected signal %d\n", signo);
        }
    }
}
// 初始化信号监听
static void init_signals()
{
    int err;
    
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
        print_err("SIG_BLOCK error\n");
        exit(1);
    }
}
// 初始化所需的个功能
static void init_stack()
{
    tun_init();
    netdev_init();
    route_init();
    arp_init();
    tcp_init();
}
// 开启线程
static void run_threads()
{
    create_thread(THREAD_CORE, netdev_rx_loop);
    create_thread(THREAD_TIMERS, timers_start);
    create_thread(THREAD_IPC, start_ipc_listener);
    create_thread(THREAD_SIGNAL, stop_stack_handler);
}
// 等待线程执行完成
static void wait_for_threads()
{
    for (int i = 0; i < 3; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            print_err("Error when joining threads\n");
            exit(1);
        }
    }
}
// 关闭进程释放资源
void free_stack()
{
    abort_sockets();
    free_arp();
    free_routes();
    free_netdev();
    free_tun();
}
// 安全选项初始化
void init_security()
{
    if (prctl(PR_CAPBSET_DROP, CAP_NET_ADMIN) == -1) {
        perror("Error on network admin capability drop");
        exit(1);
    }

    if (prctl(PR_CAPBSET_DROP, CAP_SETPCAP) == -1) {
        perror("Error on capability set drop");
        exit(1);
    }
}

int main(int argc, char** argv)
{
    // 解析指令
    parse_cli(argc, argv);
    // 初始化信号机制
    init_signals();
    // 初始化协议栈需要的:tun设备初始化,网络设备初始化,路由设备初始化,arp协议实现初始化(伪的),tcp协议实现的初始化(伪的)。实际上arp和tcp协议的运行是在netdev.c的netdev_receive函数中
    init_stack();
    // 安全选项
    init_security();
    // 创建四个线程:1.当做虚拟网卡接收流量 2.定时器 3.进程间通信的组件 4.监听信号用来停止hfip进程的线程
    run_threads();
    // 主进程等待线程执行(把所有线程设置为pthread_join属性)
    wait_for_threads();
    // 释放所有占用资源
    free_stack();
}