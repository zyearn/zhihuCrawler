#ifndef _CRAWLER_H_
#define _CRAWLER_H_

#include <iostream>
#include <fstream>
#include <sstream>
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
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "Url.h"
#include "Parse.h"
#include "config.h"
#include "DNSManager.h"
#include "dbg.h"

#define MAXEVENTS   1024
#define MAXCONNS    2
#define MAXLINE     2048
#define SMALLLINE   64
#define LINESIZE    256
#define HTMLSIZE    524288

#define EPOLLTIMEOUT    1000    //1s

// how many users returned by requesting followee/er ajax
#define USERSPERREQ      20

#define EEOF        -10

using namespace std;

class XCrawler
{
public:
    XCrawler();
    ~XCrawler();

    void start();
    void fetch();

private:
    int epfd;
    struct epoll_event *events;
    int curConns;

    struct CrawlerState {
        int iFd;
        int iState;
        char base[SMALLLINE];
        int iLen;
        char htmlBody[HTMLSIZE];
        int iLast;
        
        // need iFolloweeCount times request
        int iFolloweeCount;
        int iFolloweeCur;

        int iFollowerCount;
        int iFollowerCur;

        // hash_id and _xrsf for ajax
        char hashId[LINESIZE];
        int iHashIdSize;
        char xsrf[LINESIZE];
        int iXsrfSize;
    };

private:
    void init();
    void init_epoll();

    //int fetch_url_and_make_connection(int *pFd, string &sUrl);
    int fetch_url(string &sUrl);
    int make_connection(int *pFd);
    int prepare_get_answer_request(char *pReq, int *pSize, string &sUrl);
    int prepare_get_followers_request(char *pReq, int *pSize, string &sUrl, int iCur, CrawlerState *pState);
    int prepare_get_followees_request(char *pReq, int *pSize, string &sUrl);
    int get_response(CrawlerState *pState);
    int make_socket_non_blocking(int fd);
    void push_urls(vector<string> &vFollows);
    int is_valid_html(char *pHtml, int iSize);

};

#endif
