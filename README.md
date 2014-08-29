# 知乎爬虫

## 介绍

知乎爬虫是个小型爬虫工具，用来爬下知乎最高票的答案们，在linux下用C++开发，其中的用到的较多库工具由 [zouxin](http://blog.csdn.net/zouxinfox/article/details/2221224) 在写他的小型搜索引擎的时候提供。

## 使用

编辑./startfile/seeds.txt, 爬虫将从这个文件指定的用户URL开始爬。

    make
    ./zhihuCrawler

即开始爬，可以访问http://localhost:8080来查看爬虫的状态。

## 输出

爬下的数据都存储在./datafile/rawData.raw下。
调用

    ./sort.sh

可以查看根据票数排序后的结果。
