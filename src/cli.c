#include "syshead.h"
#include "utils.h"
#include "cli.h"

int debug = 0;
// 操作提示函数
static void usage(char *app)
{
    print_err("Usage: %s\n", app);
    print_err("\n");
    print_err("Linux TCP/IP stack implemented with TUN/TAP devices.\n");
    print_err("Requires the CAP_NET_ADMIN capability. See capabilities(7).\n");
    print_err("See https://www.kernel.org/doc/Documentation/networking/tuntap.txt\n");
    print_err("\n");
    print_err("Options:\n");
    print_err("  -d Debug logging and tracing\n");
    print_err("  -h Print usage\n");
    print_err("\n");
    exit(1);
}

extern int optind;
// 命令选项解析
static int parse_opts(int *argc, char*** argv)
{
    int opt;

    while ((opt = getopt(*argc, *argv, "hd")) != -1) {
        switch (opt) {
        case 'd':
            debug = 1;
            break;
        case 'h':
        default:
            usage(*argv[0]);
        }
    }

    *argc -= optind;
    *argv += optind; 

    return optind;
}
// main.c里调用的,命令交互式解析器实例
void parse_cli(int argc, char **argv)
{
    parse_opts(&argc, &argv);
}
