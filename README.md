# myServer
## 概述
参考**MQTT**实现了轻量级自定义应用层协议TCP服务器
* 使用 **线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(模拟Proactor)** 的并发模型
* 实现连接、订阅、发布、心跳报文功能
* 添加**同步/异步日志系统**，记录服务器运行状态
* 经压力测试可以实现**上万的并发连接**数据交换 ----//待进行  

## 环境
ubuntu18.04  

# 文件名
/lock：锁  
/log：日志系统  
/myProtocol：自定义应用层协议——简化版MQTT  
/threadpool：线程池  

# 自定义协议 - 服务器
参考MQTT实现了简化版MQTT协议
## 连接 CONNECT 
服务器接收连接报文，并在map中注册其主题。  
报文格式：1位控制报文类型 + 2位剩余长度 + 1位TOPIC长度 + TOPIC  
## 订阅 SUBSCRIBE
服务器接收订阅报文，找到其订阅主题，服务器会转发其订阅的主题给客户端。  
报文格式：1位控制报文类型 + 2位剩余长度 + 1位SUB_TOPIC长度 + SUB_TOPIC   
## 发布 PUBLISH
服务器接收发布报文，并向其此主题的订阅者转发其报文。  
报文格式：1位控制报文类型 + 2位剩余长度 + 1位TOPIC长度 + TOPIC + 消息  
## 心跳 PING
长连接，保持通信活性，每隔10s向客户端发送心跳报文，同时接收客户端心跳报文，超出15s则关闭连接。  
报文格式：1位控制报文类型 + 2位剩余长度 + 1位TOPIC长度 + TOPIC  

# 实现demo功能
服务器：个人PC_linux  
客户端：1、STM32H7_MCU_Freertos; 2、STM32MP157_MPU_Linux  
## 功能：
* STM32H7_MCU_Freertos发布温湿度信息，订阅STM32MP157_MPU_Linux信息来控制LED灯的颜色和亮灭。  
* STM32MP157_MPU_Linux发布灯控制信息来控制其订阅者LED灯的颜色和亮灭。

#个人学习项目

- 服务器github仓库地址： https://github.com/White-mustache/myServer  
- 客户端STM32H7_MCU_Freertos github仓库地址： https://github.com/White-mustache/myClient_STM32H7_MCU  
- 客户端STM32MP157_MPU_Linux github仓库地址： https://github.com/White-mustache/myClient_STM32MP157_MPU  