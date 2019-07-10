# SEGGER|FIND protocol |快速检索局域网所有设备案例|源码实现

## 我如何知道它

第一次知道这个工具是在安富莱的公众号上，推送的文章标题是[【原创开源应用第6期】基于SEGGER的FIND小软件，快速检索局域网所有设备案例，非常实用](http://forum.armfly.com/forum.php?mod=viewthread&tid=89911&highlight=FIND)。

这篇文章对它的说明是:

```
1、借助SEGGER的FIND小软件，用户可以快速检索局域网内的所有设备，仅需在设备上插入一小段的UDP代码即可。
2、实现原理是FIND小软件向网络内发UDP广播消息，接收到消息的设备，返回一条FIND格式的信息即可。通过这种方式可以快速检索所有设备的运行状态，IP，MAC等信息，便于管理。
3、关于协议格式的学习，看此贴：https://www.segger.com/products/connectivity/embosip/technology/find-protocol/
```

好奇之余打开官网，[官网对FIND的介绍](https://www.segger.com/products/connectivity/embosip/technology/find-protocol/)是:

```
FIND是一种免费使用的嵌入式设备协议，可以轻松快速地在 LAN 中定位它们。FIND 在几分之一秒内找到支持发现的所有嵌入式设备。由于 ROM 少于300个字节且没有占用任何 RAM，因此可以在任何具有任何TCP/IP堆栈的设备上轻松实现。您可以自由地使用FIND进行任何操作，甚至是商业和闭源软件。
```

更详细的介绍请打开这两个链接，这里不再展开说。

## 为什么这么做

当时我看完这篇文章之后花了几分钟时间移植到了我自己用的开发板上，使用的网络协议栈是 NicheStack。在试用之后感觉真的如介绍所说，小巧、实用。由于上位机软件 FIND Discover 是看不到源码的，不禁好奇上位机是如何实现的。

好在官网上给出了 Client  和  Host  的实现原理，大概就是上位机需要监听UDP端口50022，然后每隔一段时间发送一次查询信息，然后等待回应，摘取有效回应。

然后我试着在 WIN32 环境下实现了主机和客户机的代码。其实，实现的原理很简单，没有很多弯弯绕绕，感兴趣的同学可以抓包对比下，官方的实现和我的实现。下面直接给运行结果。

## UDPDiscover 运行现象

![UDPDiscover](https://github.com/songwenshuai/SEGGER_FIND/blob/master/Picture/UDPDiscover.png)

## UDPAnswer 运行现象

![UDPAnswer ](https://github.com/songwenshuai/SEGGER_FIND/blob/master/Picture/UDPAnswer.png)

## 代码在哪里

请参阅 我的 [Github 代码页面](https://github.com/songwenshuai/SEGGER_FIND)。

# 如何联系我

- 邮箱：[songwenshuai@sina.com](mailto:songwenshuai@sina.com)
- 主页：[songwenshuai](<https://github.com/songwenshuai>)
- 仓库：[Github](<https://github.com/songwenshuai>)

- 专栏：[zhuanlan](<https://zhuanlan.zhihu.com/songwenshuai>)