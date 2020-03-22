一、安装依赖软件包
1、MySQL
    sudo apt-get install mysql-server
    sudo apt-get install mysql-client
    sudo apt-get install libmysqlclient-dev

一、编译并运行
    make
    sudo ./xmoon

二、检查 xmoon 是否已经成功运行
    ps -eo pid,ppid,sid,tty,pgrp,comm,stat,cmd | grep -E 'bash|PID|xmoon'
    如果出现1个 master 进程、4个 worker 进程，则表示 xmoon 已经成功运行。


