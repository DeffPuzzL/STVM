# STVM 高效缓存数据库

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
新建stvm工作目录
**mkdir tvmdb**
* TVMDBD=/home/stvm/tvmdb 指向stvm工作目录
* TVMCFG=/home/stvm/tvmdb/.tvm.run 启动参数


使用方法:
如果不使用多机集群，无需配置域文件，直接 stvm -w 缺省启动单机模式。

stvm -s 停止系统

STVM也提供一个类型sqlpuls类型简单工具， 输入:
stvm SQL 进入SQL界面，该工具主要用来运维调试使用。
stvm DOM 进入域的维护

关于域的配置，请参见:
    stvm.conf

配置域完成后，可以stvm -c stvm.conf编译到$TVMCFG下
然后在stvm -w启动即可
目前本版本为测试版本，建议以单机模式64位下运行，将性能发挥在极致。

**作者:Savens Liu**

**邮箱:deffpuzzl@qq.com**

## 3、更新
====***更新日期:20180312***====

>* 1、支持动态创建表语句

>* 2、新增对表结构导出功能

用法:
表创建语法示例: tbl_acct_base.def

在stvm SQL中，利用 create $file来创建表


**下本版本：**
* 1、新增多机资源共享方式。
* 2、JAVA接口开发



**不定时更新**
