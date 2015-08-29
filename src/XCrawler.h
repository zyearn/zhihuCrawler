#ifndef _CRAWLER_H_
#define _CRAWLER_H_

#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <queue>
#include <string>
#include <iterator>
#include <signal.h>
#include <cstdlib>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <unistd.h>
#include <fcntl.h>

#include "CMd5.h"
#include "Url.h"
#include "Parse.h"
#include "config.h"
#include "ThreadMutex.h"
#include "DNSManager.h"
#include "dbg.h"

#define MAXEVENTS   1024
#define MAXCONNS    500
#define MAXLINE     2048
#define SMALLLINE   64
#define HTMLSIZE    524288

#define EPOLLTIMEOUT    1000    //1s

using namespace std;

struct md5comp
{
	bool operator()(const MD5 &d1, const MD5 &d2) const
	{
		for(int i=0;i<4;i++)
		{
			if(d1.data[i]<d2.data[i])
				return true;
			else if(d1.data[i]>d2.data[i])
				return false;
		}
		return false;//make sure the when A equal B	
	}
};

class XCrawler
{
public:
    XCrawler();
    ~XCrawler();

    void start();
    void fetch();
private:
    void init();
    void init_epoll();
    void addUrl(string curUrl,const vector<string> &links,int urldeep);

    int fetch_url_and_make_connection(int *pFd, string &sUrl);
    int prepare_get_answer_request(char *pReq, int *pSize, string &sUrl);
    int get_response(int iFd, char *pHtmlBody, int *pHtmlSize);
    int make_socket_non_blocking(int fd);

private:
    int epfd;
    struct epoll_event *events;
    int curConns;

    struct CrawlerState {
        int iFd;
        int iState;
        char base[SMALLLINE];
        int iLen;
    };
};

#endif
