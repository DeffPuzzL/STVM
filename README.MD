# STVM 高效缓存数据库

STVM（truck of Virtual memory table）是一个开源的使用ANSI C语言编写、支持本地API调用和网络调用，全表数据基于IPC共享内存方式存储，基于C语言struck结构定义记录行，RB-Tree和hash作为主要算法的内存数据库，是一款介于SQL和NOSQL之间的一款高速缓存数据库。

*   支持SQL基础语法(insert、update、delete、select、group order、count，first)功能。
*   支持序列
*   拥有百万级别队列(gcc版本 >= 4.12)
*   支持唯一索引、查询索引和组合索引
*   支持多字动态查询
*   内置记录点击量(热点数据)
*   集群、主-子同步。
*   事务功能（假性事务）
*   基于IPC，进程异常退出，不会导致数据丢失，除非系统宕机。
*   支持网络API同步、异步和本地API直接调用
*   内置数据版本，维护数据一致安全
*   数据导入、导出接口

***使用须知***
*  表空间大小在创建时已经确定, 一旦创建不能随意修改大小。
*  多机集群无法使用事务。
*  数据无法持久化(宕机情况)。
*  索引长度限制（默认64字节），修改需编译项目。
*  启动后不能修改表字段，但可以修改字段别名

建议使用场景：***数据库缓存、数据缓存、事件驱动***

不定时更新 https://my.oschina.net/deffpuzzl/blog/1627626  

## 1、编译
**编译项目**

cd src目录 **./make**

在../lib下生成
*    libstvm.a
*    libstvm.so
*    libstvm.so.1.2 -> ../lib/libstvm.so

在../bin下生成
*    stvm
*    detvm

## 2、启动
新建stvm工作目录 **mkdir tvmdb**
* **TVMDBD=/home/stvm/tvmdb** 指向stvm工作目录
* **TVMCFG=$(TVMDBD)/.tvm.run** 启动参数

使用方法:
如果不使用多机集群，无需配置域文件，缺省本地启动。 

启动系统  
> stvm -w

停止系统  
> stvm -s 

STVM也提供一个类型sqlpuls类型简单工具。
进入SQL界面, 该工具主要用来运维调试使用
> stvm SQL  

域的维护
> stvm DOM

关于域的配置，请参见:
> vi stvm.conf

配置域完成后，可以配置编译到$TVMCFG下
> stvm -c stvm.conf

然后在stvm -w启动即可

建议以单机模式64位下运行，将性能发挥在极致。

**作者：Savens Liu**

**邮箱：deffpuzzl@qq.com**

## 3、更新
====***更新日期:20180312***====

>* 1、支持动态创建表语句

>* 2、新增对表结构导出功能

用法:
表创建语法示例: tbl_acct_base.def

在stvm SQL中，利用 create $file来创建表

====***更新日期:20180314***====

>* 1、新增表数据结构化导出、导入功能

>* 2、新增热点数据点击量api

====***更新日期:20180430***====

>* 1、修复create脚本sizeof算法问题。
>* 2、新增异步操作(网络插入、删除、更新）异步接口支持。

====***更新日期:20180515***====

>* 1、新增脚本msql来批量执行M-SQL语句。
>* 2、对表字段新增别名，暂用一些场景字段映射。
>* 3、新增comment关键字，在创建脚本文件或M-SQL中本地使用
>* 4、修复char字符拷贝和M-SQL界面非人性化操作。

====***更新日期:20180531***====

>* 1、新增replace功能(有则更新，无则新增), 包括同、异步接口
>* 2、在M-SQL支持查询索引 show index from tablename
>* 3、新增用户M-SQL用户体验(TAB补全、查询展示showmode、showsize)

====***更新日期:20180610***====

>* 1、新增queue网络同步异步接口
>* 1、新增将表中长时间无用数据导出备份接口

**下本版本：**
* 1、新增多机资源共享方式。
* 2、JAVA接口开发


**不定时更新**




---
#### *常见错误集合* ####
```
Q、failed to boot TVM, Invalid parameter or sem has disappeared
A：echo "500 X  X X" > /proc/sys/kernel/sem
```
