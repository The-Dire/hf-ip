## Tun及Tap接口教程

Tun/tap接口是Linux(可能还有其他类unix操作系统)提供的一种功能，它可以进行用户空间网络连接，也就是说，允许用户空间程序查看原始网络流量(在以太网或IP级别)，并使用它做任何它们想做的事情。本文试图解释tun/tap接口在Linux下是如何工作的，并使用一些示例代码演示它们的用法。

## Tun及Tap设备如何工作的呢？

Tun/tap接口是软件专用接口，这意味着它们只存在于内核中，而且与常规网络接口不同，它们没有物理硬件组件(因此没有物理“线”连接到它们)。可以将tun/tap接口看作是一个常规的网络接口，当内核决定在“连线”上发送数据时，它将数据发送到附加到该接口的某个用户空间程序(使用一个特定的过程)。当程序附加到tun/tap接口时，它会获得一个特殊的文件描述符，从中读取接口要发送的数据。以类似的方式，程序可以写入这个特殊的描述符，数据(我们将看到，它必须被正确格式化)将作为tun/tap接口的输入出现。对于内核来说，看起来tun/tap接口正在“从网络”接收数据。

tap接口和tun接口的区别在于，tap接口输出(并且必须给定)完整的以太网帧，而tun接口输出(并且必须给定)原始IP数据包(内核没有添加以太网报头)。接口是tun接口还是tap接口，在创建接口时使用标志指定。

接口可以是暂时的，这意味着它是由同一个程序创建、使用和销毁的;当程序终止时，即使它没有显式地销毁接口，接口也会停止存在。另一个选择(我喜欢的)是使接口持久;在本例中，使用专用实用程序(如tunctl或openvpn—mktun)创建它，然后普通程序可以附加到它;当它们这样做时，它们必须使用最初创建接口时使用的相同类型(tun或tap)进行连接，否则它们将无法连接。我们会看到这是怎么做的

一旦tun/tap接口就位，它就可以像其他接口一样使用，这意味着可以分配IP地址，可以分析其流量，可以创建防火墙规则，可以建立指向它的路由，等等。

有了这些知识，就可以尝试看看如何使用tun/tap接口，以及可以用它做什么。

## 创建接口

tun/tap可以创建一个全新的接口(必须在root权限下),也可以指定已有接口使用(可在普通用户下)。所以分两段来介绍。

创建全新接口和(重新)附加到持久接口的代码本质上是相同的;区别在于前者必须由root用户运行(更准确地说，是由具有CAP_NET_ADMIN权限的用户)，而后者可以由普通用户运行，如果满足某些条件。让我们从创建一个新接口开始。

首先，无论做什么，设备/dev/net/tun必须打开读/写权限。该设备也称为克隆设备，因为它被用作创建任何tun/tap虚拟接口的起点。该操作(与任何open()调用一样)返回一个文件描述符。但这还不足以开始使用它与接口通信。

创建接口的下一步是发出一个特殊的ioctl()系统调用，它的参数是在上一步中获得的描述符、TUNSETIFF常量和一个指向包含描述虚拟接口的参数的数据结构的指针(一般来说,获得了虚拟网卡设备的名称和所需的操作模式tun还是tap)。作为一种变体，虚拟接口的名称可以不指定，在这种情况下，内核将通过尝试分配这种类型的“下一个”设备来选择一个名称(例如，如果tap2已经存在，内核将尝试分配tap3，等等)。所有这些都必须由root用户(或具有CAP_NET_ADMIN能力的用户)完成--都是需要该权限故不在提示。

如果ioctl()函数执行成功，虚拟接口就会创建，现在我们拥有的文件描述符就会与之关联，并可用于通信。

在这一点上，两件事可能会发生。程序可以立即开始使用接口(可能之前至少配置了一个IP地址)，完成之后，终止并销毁接口。另一种选择是发出两个其他特殊的ioctl()调用，以使接口持久，并终止它，使其他程序可以附加到它。这就是像tunctl或openvpn—mktun这样的程序所做的。这些程序通常还可以选择将虚拟接口的所有权设置为非根用户和/或组，因此以non-运行的程序

用于创建虚拟接口的基本代码显示在内核源代码树中的文件Documentation/networking/tuntap.txt中。稍微修改一下，我们可以写一个创建虚拟网卡接口的函数:

```C
#include <linux /if.h>
#include <linux /if_tun.h>

int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  /* 函数的参数:
   *
   * char *dev: 接口的名称 (或者是 '\0'). 如果传递了'\0'，
   * 必须有足够的空间保存接口名称
   * int flags: IFF_TUN   - TUN device (no Ethernet headers)
   *					    IFF_TAP   - TAP device
   *                        IFF_NO_PI - 不提供数据包信息
   */

   /* 打开克隆设备 */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }
   /*
    * Linux支持一些标准的ioctl来配置网络设备。
	* 可以在任何套接字的文件描述符上使用它们
	* 。大多数通过ifreq结构。具体参考:https://man7.org/linux/man-pages/man7/netdevice.7.html
   */
   /* 获得结构ifr，类型为“struct ifreq” */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, 也许是 IFF_NO_PI */

   if (*dev) {
     /* 如果指定了设备名称，将其放入结构中;否则，内核将尝试分配指定类型的“下一个”设备 */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* 尝试创建设备 */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* 
  *  如果操作成功，则将接口的名称写回变量“dev”，
  *  以便调用者可以知道它。
  *  注意，调用者必须在*dev中保留空间(参见下面的调用代码)
  */
  strcpy(dev, ifr.ifr_name);

  /* 这是调用者将用于与虚拟接口通信的特殊文件描述符 */
  return fd;
}
```
tun_alloc()有两个参数:

- `char *dev`包含接口的名称(例如，tap0、tun2等)。任何名称都可以使用，不过最好选择一个表明它是哪种接口的名称。在实践中，通常使用像tunX或tapX这样的名称。如果`*dev`是'\0'，内核将尝试创建请求类型的“第一个”可用接口(例如，tap0，但如果已经存在，则创建tap1，等等)。

- `int flags`包含了告诉内核我们想要的接口类型的标志(tun或tap)。基本上，它可以取IFF_TUN值来表示TUN设备(包中没有以太网头)，或IFF_TAP值来表示TAP设备(包中有以太网头)。另外，另一个标志IFF_NO_PI可以使用基本值来修改。IFF_NO_PI告诉内核不提供数据包信息。IFF_NO_PI的目的是告诉内核包将是“纯”IP包，不增加字节。否则(如果IFF_NO_PI未被设置)，4个额外的字节被添加到包的开始(2个标志字节和2个协议字节)。IFF_NO_PI不需要匹配接口创建和重连接时间。还要注意，当使用Wireshark在接口上捕获流量时，这4个字节不会显示出来。

一个程序可以使用以下代码来创建一个设备:

```C
  char tun_name[IFNAMSIZ];
  char tap_name[IFNAMSIZ];
  char *a_name;

  ...

  strcpy(tun_name, "tun1");
  tunfd = tun_alloc(tun_name, IFF_TUN);  /* tun interface */

  strcpy(tap_name, "tap44");
  tapfd = tun_alloc(tap_name, IFF_TAP);  /* tap interface */

  a_name = malloc(IFNAMSIZ);
  a_name[0]='\0';
  tapfd = tun_alloc(a_name, IFF_TAP);    /* '\0'让内核选择一个名称 */
```

如前所述，程序可以按照自己取的名字使用接口，也可以将其设置为持久性(并可选地将所有权分配给特定的用户/组)。如果是前者，那就没什么可说的了。但如果是后者，会发生什么呢?

另外还可以通过ioctl()函数可以修改tun/tap设备的属性(有两种属性)，它们通常一起使用。第一个系统调用可以设置(或删除)接口上的持久状态。第二种方法允许将接口的所有权分配给普通(非根)用户。这两个特性都在程序tunctl (UML实用程序的一部分)和openvpn—mktun(可能还有其他程序)中实现。因为tunctl代码更简单，所以让我们来检查一下它，记住它只创建tap接口，因为这是linux使用的用户模式(为了清晰起见，代码稍加编辑和简化):

```C
...
  /* 如果用户想要删除，需设置现有的接口为
      “delete”属性(即非持久);
    如果不设为"delete"属性,用户将创建一个新的接口 */
  if(delete) {
    /* 删除持久状态 */
    if(ioctl(tap_fd, TUNSETPERSIST, 0) < 0){
      perror("disabling TUNSETPERSIST");
      exit(1);
    }
    printf("Set '%s' nonpersistent\n", ifr.ifr_name);
  }
  else {
    /* 在TUNSETGROUP之前模拟行为 */
    if(owner == -1 && group == -1) {
      owner = geteuid();
    }

    if(owner != -1) {
      if(ioctl(tap_fd, TUNSETOWNER, owner) < 0){
        perror("TUNSETOWNER");
        exit(1);
      }
    }
    if(group != -1) {
      if(ioctl(tap_fd, TUNSETGROUP, group) < 0){
        perror("TUNSETGROUP");
        exit(1);
      }
    }

    if(ioctl(tap_fd, TUNSETPERSIST, 1) < 0){
      perror("enabling TUNSETPERSIST");
      exit(1);
    }

    if(brief)
      printf("%s\n", ifr.ifr_name);
    else {
      printf("Set '%s' persistent and owned by", ifr.ifr_name);
      if(owner != -1)
          printf(" uid %d", owner);
      if(group != -1)
          printf(" gid %d", group);
      printf("\n");
    }
  }
  ...
```

这些ioctl()函数仍然必须由root运行。但现在拥有的是特定用户拥有的持久接口，因此作为该用户运行的进程可以成功地连接到该接口。

如上所述，将(重新)附加到现有的tun/tap接口的代码与用于创建该接口的代码相同;换句话说，tun_alloc()可以再次使用。在这样做的时候，要想成功，必须做到三件事:

- 接口必须已经存在，并且为试图连接的同一用户所拥有(可能是持久的)

- 用户必须对/dev/net/tun具有读/写权限(用户为root权限)

- 提供的标志必须与用于创建接口的标志匹配(例如，如果它是用IFF_TUN创建的，那么在重新附加时必须使用相同的标志)。

这是可行的，因为内核允许TUNSETIFF ioctl()成功，如果发出它的用户指定了一个已经存在的接口的名称，并且他是接口的所有者。在这种情况下，不需要创建新的接口，因此普通用户可以成功地执行操作。

- 如果不存在或没有指定接口名，则意味着用户正在请求分配新接口。内核因此使用给定的名称创建接口(如果给定空名称，则选择下一个可用名称)。只有在由根用户执行时才有效。

- 如果指定了现有接口的名称，则意味着用户希望连接到以前分配的接口。这可以由普通用户完成，前提是:用户在克隆设备上拥有适当的权限，并且是接口的所有者(在创建时设置)，并且指定的模式(tun或tap)与创建时设置的模式匹配。

可以在内核源代码的driver/net/tun.c文件中查看实现上述步骤的代码;重要的函数有`tun_attach()`、`tun_net_init()`、`tun_set_iff()`、`tun_chr_ioctl()`;最后这个函数还实现了各种可用的`ioctl()`，包括TUNSETIFF、TUNSETPERSIST、TUNSETOWNER、TUNSETGROUP等。

这是一个可能的使用场景

- 虚拟接口由root用户创建、持久、分配和配置(例如，通过在引导时使用tunctl或等效的initscripts)

- 然后，普通用户可以根据自己的需要多次附加和分离虚拟接口。

- 虚拟接口由root销毁，例如在关闭时运行的脚本，可能使用tunctl -d或等效程序


#### 实现tunnel

在理解了上述操作后。应该知道了怎么使用tun/tap。下面看一个利用tun/tap实现简单tunnel的例子。不需要重新实现TCP/IP;相反，可以编写一个程序来把原始数据来回地传递给运行相同程序的远程主机，它会以反射的方式做同样的事情。假设上面的程序，除了附加到tun/tap接口之外，还建立了到远程主机的网络连接，其中有一个类似的程序(也连接到本地tun/tap接口)在服务器模式下运行。(实际上这两个程序是相同的，谁是服务器，谁是客户端是由命令行开关决定的)。一旦两个程序开始运行，流量可以朝任何方向流动，因为代码的主体将在两个站点上做相同的事情。这里的网络连接是使用TCP实现的，但任何其他方式都可以使用(如UDP，甚至ICMP!)

代码的主流程如下:

```C
/* net_fd是网络文件描述符(对对等点)，tap_fd是
	连接到tun/tap接口的描述符 */

/* 使用select()一次处理两个描述符 */
maxfd = (tap_fd > net_fd)?tap_fd:net_fd;

while(1) {
int ret;
fd_set rd_set;

FD_ZERO(&rd_set);
FD_SET(tap_fd, &rd_set); FD_SET(net_fd, &rd_set);

ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

if (ret < 0 && errno == EINTR) {
  continue;
}

if (ret < 0) {
  perror("select()");
  exit(1);
}

if(FD_ISSET(tap_fd, &rd_set)) {
  /* 来自tun/tap的数据:读取它并将其写入网络 */

  nread = cread(tap_fd, buffer, BUFSIZE);

  /* 写入 包的长度 + 包的内容 */
  plength = htons(nread);
  nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength));
  nwrite = cwrite(net_fd, buffer, nread);
}

if(FD_ISSET(net_fd, &rd_set)) {
  /* 来自网络的数据:读取它，并将其写入到tun/tap接口。
   * 需要先读取包的长度，然后读取数据包 */

  /* 读取包长度 */
  nread = read_n(net_fd, (char *)&plength, sizeof(plength));

  /* 读取包的内容 */
  nread = read_n(net_fd, buffer, ntohs(plength));

  /* 现在buffer里包含一个完整的包或帧，把它写入到tun/tap接口 */
  nwrite = cwrite(tap_fd, buffer, nread);
}
}
```

