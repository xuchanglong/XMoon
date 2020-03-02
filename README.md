XMoon
===========================
****
	
|Author|徐昌隆|
|---|---
|E-mail|xclsoftware@163.com


****
### 零、前言
　　基于 linux epoll 技术的轻量级的 C++ 通讯框架。
### 一、使用说明
>xmoon 根目录，运行 make 指令，会在根目录生成 xmoon 可执行文件，直接运行即可。
### 二、目录介绍
   * **_include**：存放所有的头文件代码。易于管理和方便 makefile 的编写。
   * **app**：存放主程序代码文件。
   * **logic**：存放业务逻辑代码文件。
   * **misc**：存放其他代码文件，包括连接池、CRC32校验。
   * **net**：存放基于 socket 的网络库代码文件，整个项目的核心。
   * **proc**：存放进程相关的代码文件。
   * **signal**：存放信号相关的代码文件。
   * **test**：存放相关功能单元测试代码文件。
### 三、整体框架图
    ![Image text](https://github.com/xuchanglong/XMoon/blob/master/%E8%BD%AF%E4%BB%B6%E6%9E%B6%E6%9E%84%E5%9B%BE.jpg)
### 四、项目概述
   * 非常完整的多进程多线程高并发服务器项目。
   * 按照包头 + 包体格式，完美地解决了数据粘包的问题。
   * 根据收到的数据包来执行不同的业务逻辑。
   * 把业务处理产生的结果数据包正确地返回给客户端。
### 五、技术特点
   * 采用一个 master 进程，多个 worker 进程的框架.
   * epoll 高并发通讯技术，用的是水平触发模式【LT】。
   * 采用线程池技术处理业务逻辑，极大的提高数据的吞吐率。
   * 采用含有嵌入式指针的内存池技术，提高程序运行效率、节约内存并防止大量内存碎片的产生。
   * 采用连接池技术，并使用延迟回收技术，防止未知异常的发生。
   * 采用线程之间的同步技术包括互斥量、信号量等等
   * 采用 C++11 及以上标准。
   * Google C++ 编程风格。

（ 完 ）