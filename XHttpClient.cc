/*
 * Copyright (C) 2008 xyzse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file XHttpClient.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#include "XHttpClient.h"
#include "XCrawler.h"
#include "config.h"
#include "ThreadMutex.h"
#include "DNSManager.h"
#include "XPage.h"
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
//#include "debug.h"

/**
 * XHttpclient create connection and download web page ,
 * it use nonblock , the timeout parameter can be modified
 * in config.h
 */

extern DNSManager dnsMana;

XHttpClient::XHttpClient()
{
}

XHttpClient::~XHttpClient()
{
}

/** download and fullfill the page,
 * if everything is ok,return 0
 * else return error code
 *  -1-->empty ip in createConnection
 *  -2-->socket create error in createConnection
 *  -3-->fcntl error 
 *  -4-->select error when connecting
 *  -5-->select timout when connecting
 *  -6-->status code>=400
 *  -7-->content too long
 *  -8-->unwanted type
 *  -9-->read error
 *  -10-->direct connect and nonblock connect are all error or timeout
 *  -11-->select read error
 *  -12-->select read timeout
 *  -13-->write error
 *  -14-->catch parserHeader exception
 */
int XHttpClient::download(const string &url,XPage &page)
{
	int sockfd=createConnection(url,page);
	if(sockfd<0)
		return sockfd;

    long tid = (long)(pthread_self()) % THREAD_NUM;
	cout<<"thread " << tid << ": connect successfully: "<<url<<endl;
	page.setUrl(url);
	//create client header
	string location;
	XUrl xurl(url);
	if(xurl.getPath().empty()) //path is empty,we can assume request main page
		location="/";
	else
		location=xurl.getPath();
	//const string reqHeader="GET "+location+" HTTP/1.0\r\nUser-Agent: Linux/1.0\r\n\r\n";	
    const string cookie = "";   // your cookie here
	const string reqHeader="GET "+location+" HTTP/1.1\r\nHost: "+xurl.getHost()+"\r\nConnection: close\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36\r\nReferer: http://www.zhihu.com/\r\nAccept-Language: zh-CN,zh;q=0.8\r\nCookie: " + cookie + "\r\n\r\n";
    
    //std::cout << "reqHeader = " << reqHeader << std::endl;

	int writeRet=write(sockfd,reqHeader.c_str(),reqHeader.length());
	if(writeRet==-1)
	{
		cerr<<"write error in XHttpClient"<<endl;
		perror("write");
		close(sockfd);
		return -13;
	}
	
	//////////////////////////read header and body
	int readFlag=fcntl(sockfd,F_GETFL,0);
	if(readFlag<0)
	{
		cerr<<"wrong in fcntl get in download"<<endl;
		close(sockfd);
		return -3;
	}
	readFlag|=O_NONBLOCK;
	if(fcntl(sockfd,F_SETFL,0)<0)
	{
		cerr<<"wrong in fcntl write in download"<<endl;
		close(sockfd);
		return -3;
	}
	struct timeval waittv;
	waittv.tv_sec=DEFAULT_TIMEOUT;
	waittv.tv_usec=0;
	fd_set nonbset, readyset;
	FD_ZERO(&nonbset);
	FD_SET(sockfd,&nonbset);
	
	int selectRet=-1,readRet=-1;
	int bytesRead=0;
	//char readBuffer[1024];//!!!!be careful,may cause "stack smashing"!!!!!!!
//	memset(readBuffer,0,sizeof(readBuffer));


	//////read header
    //std::cout << "XHTTPCLIENT: ready to read header\n";
	int newlines=0;
	//char *tempBuffer=readBuffer;
	char tempBuffer;
	//memset(tempBuffer,0,sizeof(tempBuffer));
	string pageHeader;
	pageHeader.reserve(1024);
	while(newlines<2)
	{
        readyset = nonbset;
		selectRet=select(sockfd+1,&readyset,NULL,NULL,&waittv);
		if(selectRet<0)
		{
			cerr<<"select error"<<endl;
			close(sockfd);
			return -11;
		}else if(selectRet==0)
		{
			cerr<<"select timeout"<<endl;
			//do something,maybe register this host
			close(sockfd);
			return -12;
		}
		if( (readRet=read(sockfd,&tempBuffer,1))==-1)
		{
			cerr<<"read header error"<<endl;
			close(sockfd);
			return -9;
		}
		if(tempBuffer=='\r')
		{
			//readBuffer++;
			//continue;
		}else if(tempBuffer=='\n')
			newlines++;
		else
			newlines=0;
		//tempBuffer++;
		pageHeader+=tempBuffer;
	}
	//string pageHeader(readBuffer);//now pageHeader is header
	//cout<<pageHeader<<endl;

	//////////parse header
	if(page.parserHeader(pageHeader)<0)
	{
		close(sockfd);
		return -14;//catch exception
	}
	if(page.getStatusCode()>=400||page.getStatusCode()<200)
	{
		cout<<"status code err"<<page.getStatusCode()<<endl;
		close(sockfd);
		return -6;
	}
	int contentLength=page.getContentLength();
	if(contentLength>MAX_WEBPAGE_BYTES)
	{
		close(sockfd);
		return -7;
	}
	if(page.getContentType().empty()==false&&
			page.getContentType().find("text/html")==string::npos&&
			page.getContentType().find("text/plain")==string::npos&&
			page.getContentType().find("text/xml")==string::npos&&
			page.getContentType().find("text/html")==string::npos)
	{
		cout<<"unwanted type :"<<page.getContentType()<<endl;
		close(sockfd);
		return -8;
	}

#ifdef debug
	//cout<<page.getHeader()<<endl;
#endif
    //std::cout << "XHTTPCLIENT: ready to read body\n";
	//////////read body
	string pageBody;
	char readBuffer[4096];//!!!!be careful,may cause "stack smashing"!!!!!!!
	if(contentLength>0)
		pageBody.reserve(page.getContentLength());

	selectRet=select(sockfd+1,&nonbset,NULL,NULL,&waittv);
    //std::cout << "1" << std::endl;
	if(selectRet<0)
	{
		cerr<<"select error"<<endl;
		close(sockfd);
		return -11;
	}else if(selectRet==0)
	{
		cerr<<"select timeout"<<endl;
		close(sockfd);
		//do something,maybe register this host
		return -12;
	}
    int count = 0;	
	while( (readRet=read(sockfd,readBuffer,sizeof(readBuffer)))>0 )
	{
        //std::cout << count++ << std::endl;
		pageBody+=string(readBuffer,0,readRet);
	}	
    //std::cout << "2" << std::endl;
	if(readRet<0)
	{
		cerr<<"read error"<<endl;
		close(sockfd);
		//may be you can do something,
		return -9;
	}

    //std::cout << "2" << std::endl;
	//parse body,should judge the return value!!!
	page.parserBody(pageBody);
	//////////////// now pageStream is the full content,including header and body 
	close(sockfd);
	cout<<"thread " << tid << ": download successfully: "<<url<<endl;
    //std::cout << "out of download\n";
	return 0;
}




/*if successfuly connect to the host,will return the socketfd,otherwize
 *will return negative number
 *should modify into nonblock connect
 */
int XHttpClient::createConnection(const string &url, XPage &page)
{
	//DNSManager dnsMana;
#ifdef debug
			struct timeval starttv={0,0};
			memset(&starttv,0,sizeof(starttv));
			gettimeofday(&starttv,NULL);
#endif
	string ip=dnsMana.getIP(url);
#ifdef debug
			struct timeval endtv={0,0};
			memset(&endtv,0,sizeof(endtv));
			gettimeofday(&endtv,NULL);
			cout<<"dns time cost "<<endtv.tv_sec-starttv.tv_sec<<endl;
#endif

	if(ip.empty())
		return -1;
	page.setIp(ip);

	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(80);
	inet_pton(AF_INET,ip.c_str(),&addr.sin_addr);

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		perror("socket");
		return -2;
	}
	int optval=1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)&optval,sizeof(optval));
	struct timeval connectv;
	connectv.tv_sec=CONNECT_TIMEOUT;
	connectv.tv_usec=0;
	//set connect timeout
	setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&connectv,sizeof(struct timeval));
	if(connect(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))==0)
	{
		return sockfd;
	}
	////if the socket is nonblocking,so  can not connect immediately, set nonblock connect
	if(errno==EINPROGRESS)
	{
		int readFlag=fcntl(sockfd,F_GETFL,0);
		if(readFlag<0)
		{
			cerr<<"wrong in fcntl get"<<endl;
			close(sockfd);
			return -3;
		}
		readFlag|=O_NONBLOCK;
		if(fcntl(sockfd,F_SETFL,0)<0)
		{
			cerr<<"wrong in fcntl write"<<endl;
			close(sockfd);
			return -3;
		}
		fd_set nonbset;
		FD_ZERO(&nonbset);
		FD_SET(sockfd,&nonbset);

		struct timeval tv;
		tv.tv_sec=DEFAULT_TIMEOUT;
		tv.tv_usec=0;

		int selectRet=select(sockfd+1,&nonbset,NULL,NULL,&tv);
		
		readFlag&=~O_NONBLOCK;
		fcntl(sockfd,F_SETFL,readFlag);

		if(selectRet==-1)
		{
			close(sockfd);
			return -4;
		}else if(selectRet==0)
		{
			close(sockfd);
			return -5;
		}
		//nonblocking connect success
		return sockfd;
	}
	
	//both direct connect and nonblock connect are all failed or timeout 
	close(sockfd);
	return -10;
	
}
