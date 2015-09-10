# 知乎爬虫

## 介绍

ZhihuCrawler是用C++编写的高效、基于事件驱动的知乎爬虫，目的是抓取最高赞回答、最高关注问题等数据。运行环境为支持epoll的平台。

## 使用

先找到浏览器访问知乎的cookie，将它复制到src/confic.cc下的cookie变量里。

编辑./startfile/seeds.txt, 将从这个文件指定的用户URL开始爬。

    make
    ./zhihuCrawler

可以访问http://localhost:8080来查看爬虫的状态。

## 输出

爬下的数据都存储在./datafile/rawData.raw下。
使用

    ./sort.sh

可以查看根据票数排序后的结果。

## TODO

* 增加ajax获取用户的全部关注人和关注者

* 降低模块间耦合度

* 用代理IP处理429错误/IP被封

## 更多

更多详情请访问 http://lifeofzjs.com/blog/2015/09/09/how-to-write-a-event-based-crawler-using-c/

// 用C/C++写爬虫真是做大死
