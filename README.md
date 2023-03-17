# Cyber-Lighthouse
## Day0（2023.3.11）----有个创意赛，所以做事情的时间不是很多
~~1.确定了后端框架：Crow--一个基于C++编写的轻量级、高性能、跨平台的Web框架  
官方文档：https://crowcpp.org/  
2.在Ubuntu18.04上完成了Crow的环境部署~~

在task1和2疯狂横跳，完善理解ing

1.重新确定使用boost.asio这个socket库去完成该任务

2.在Ubuntu18.04上完成了库的部署

3.了解TCP和UDP的原理

4.尝试开始写server和client部分（追加，看到了一点钟，有点迷糊了，但是大概的理了一下框架思路，明天重新开始写）

## Day1(2023.3.12)
#### 这里是凌晨两点钟，接上面凌晨一点
到此为止，我先规划了WebClient的封装，然后动手写了个client的大概，但是还没有写进主函数，具体细节有待完善
#### 这里是凌晨四点钟了
我的client也终于写完并且debug到了最后，可是不知道为什么我的make时好像库没有连接上，也没什么时间再看了，就先这样子叭，睡了，good night
![image](https://user-images.githubusercontent.com/113229789/224509458-702e3610-575c-40ce-bad0-a5db222d5a23.png)
#### 这里是中午12点
上面的连接问题我也一个个的解决了，很累但是很开心捏。But一个新的问题--connection refused，还在debug中。
#### day01结束了
完成了task1基础任务，并且进阶任务除了没有支持GET/POST以及文件以外，都做完力（过程是曲折的，螺旋上升滴）

其次在完成的基础上我对我的服务端和客户端都做了一个异常处理（使用try-catch）,并且在服务端设置一个threshold来抑制高并发的情况，同时为并发提供一秒的缓冲
![image](https://user-images.githubusercontent.com/113229789/224555012-cac9dea5-0b75-4ac9-97d9-746af5433ceb.png)
![image](https://user-images.githubusercontent.com/113229789/224555023-fee007bc-fb79-4865-814d-7a19ef7c697e.png)

## day2(2023.3.13)
今日早八到晚十一的课，好在Task2比较轻松  
完成了Task2中所有的基础要求，主要是学会了dig的基础命令去查看DNS的查询和响应报文，然后使用wiresshark进行抓包后，在过滤器中用dns.flags.response == 0滤去响应，保留查询报文  
下面是我的md文档记录截屏
![image](https://user-images.githubusercontent.com/113229789/224764828-d9c0c7a2-e615-4c00-8710-fda02fde5c78.png)
![image](https://user-images.githubusercontent.com/113229789/224764928-02d2fe29-b0cd-4f0f-8343-5fc272793d49.png)
![image](https://user-images.githubusercontent.com/113229789/224764997-41f6e45d-e0b3-41a9-9e48-d4585e806b94.png)
还有好多好多就不放了，明天又是早八人，还是满课的，枯，goodnight

## day3(2023.3.14)--新增DNS_parser以及它的cpp文件
又是满课的一天（还有个大物实验），所以今天只能浅浅的写一个DNS报文解析器，它可以格式化A、NS、CNAME三种记录，并且能够存储所有类型的记录  
但是美中不足的是我没有测试它（时间实在不太够，也就没写文件读取的操作，截至目前）  
好了，差不多就这样力，goodnight（晚安，早八微积分人

## day4(2023.3.15)
和李根学长交流了很久，发现自己对于task3的误区，最终把我的解析器重新修改，但是对于压缩指针的处理还是出现了问题  
爆肝到凌晨两点（在这里点名李根学长也还没睡捏，bushi），还是没有解决这个问题，害，白天再看

## day5（2023.3.16）
再一次和李根学长交流，从头到尾的理了一下task3希望我们干什么，下午一点解决了压缩指针偏移出现的问题  
开始了报文生成器，这一次遇到了极大的阻力，（再次点名李根学长没有考虑cpp需要手动处理字节序的问题，害，努力爆肝），在晚上九点左右我成功写完了报文生成器  
然后我的报文解析和生成器相互测试成功  
晚上十点十五，我完成了对于自己整个项目目前代码的整理，test下的文件夹是完成了task1-3所有基本任务以及一半多的境界任务（也许是一半？？反正让自己的功能更加的完备，也有一定的异常处理）  
接着准备开始我的task4之旅
