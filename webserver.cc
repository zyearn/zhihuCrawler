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
 * @file webserver.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#include "config.h"
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
/**
 * This code is used to monitor the status of the crawler
 * open your web brower,and visit http://127.0.0.1:8080
 * you will see the statistics
 */
void writeHeader(int);
void writeBody(int);

/*start listening port*/
void *startWebserver(void *arg)
{
  int listensock,connsock;
  struct sockaddr_in serveraddr;
  bzero(&serveraddr,sizeof(serveraddr));
  listensock=socket(AF_INET,SOCK_STREAM,0);
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
  serveraddr.sin_port=htons(8080);
  if (bind(listensock,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) < 0)
  {
      fprintf(stderr, "bind error\n");
      exit(0);
  }
  printf("begin to listen on port 8080... ");
  listen(listensock,3);
  while(1)
  {
  	connsock=accept(listensock,NULL,NULL);
    if (connsock < 0)
    {
        fprintf(stderr, "Web Server, Accept error: %s\n", strerror(errno));
        exit(0);
    }
    printf("accept!\n");
	writeHeader(connsock);
	//writeBody(connsock);
  	//send(connsock,buff,sizeof(buff),0);
  	close(connsock);
  }

}

/*write html header*/
void writeHeader(int sock)
{
	struct timeval nowtv;
	char cnowtv[100];
	gettimeofday(&nowtv,NULL);
	sprintf(cnowtv,"%d",nowtv.tv_sec-starttime.tv_sec);

	char cpageNum[100];
	sprintf(cpageNum,"%d",pageNum);

	const string body="<html>\n<head><title>xyzse real time stat</title></head><body><p>"+string(cnowtv)+" seconds</p>"+string(cpageNum)+"</body></html>";
	//send(sock,body.c_str(),body.length(),0);

    char num[100];
    sprintf(num, "%d", body.length());
	const string header="HTTP/1.0 200 OK\r\nServer: xyzse\r\nContent-Type: text/html\r\nContent-Length: " + string(num) + "\r\n\r\n";
	send(sock,header.c_str(),header.length(),0);
	send(sock,body.c_str(),body.length(),0);
}

/*write html body*/
void writeBody(int sock)
{
	struct timeval nowtv;
	char cnowtv[100];
	gettimeofday(&nowtv,NULL);
	sprintf(cnowtv,"%d",nowtv.tv_sec-starttime.tv_sec);

	char cpageNum[100];
	sprintf(cpageNum,"%d",pageNum);

	const string body="<html><p>"+string(cnowtv)+" seconds<p>"+string(cpageNum)+"</html>";
	send(sock,body.c_str(),body.length(),0);
}
