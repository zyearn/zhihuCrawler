#include "XCrawler.h"

/**
 * The most important class.
 * This class start thread, maintain priority queue ,fetch web page,
 * add links to priority queue etc.
 * Note that currently we have not implemented all the futures provided here
 * 
 */

queue<string> unvisitedUrl;

set<MD5,md5comp> visitedUrlMd5;

set<MD5,md5comp> unreachableHostMd5;

ThreadMutex mutex_unvisitedUrl[20]; 
ThreadMutex mutex_visitedUrl;
ThreadMutex mutex_unreachable_host;
ThreadMutex mutex_processlink;
ThreadMutex mutex_pagenum;

DNSManager dnsMana;

ofstream fout("./processlink");
ofstream fResultOut(RAW_DATA_FILE.c_str());

XCrawler::XCrawler():
    curConns(0) {

    init();
    init_epoll();
}

XCrawler::~XCrawler() {

}

void XCrawler::init() {
    string strLine;

    ifstream init_file;
    init_file.open(SEEDS_FILE.c_str(), ios::binary);
    if (!init_file) {
        exit(0);
    }
    while (getline(init_file, strLine)) {
        unvisitedUrl.push(strLine);
    }
    init_file.close();

    init_file.open(VISITED_URL_MD5_FILE.c_str(), ios::binary);
    if (!init_file) {
        exit(0);
    }

    while (getline(init_file,strLine)) {
        CMD5 tempCmd5(strLine.c_str());
        visitedUrlMd5.insert(tempCmd5.getResult());
    }
    init_file.close();

    init_file.open(UNREACHABLE_HOST_MD5_FILE.c_str(),ios::binary);
    if(!init_file) {
        exit(0);
    }

    while(getline(init_file,strLine)) {
        CMD5 tempCmd5(strLine.c_str());
        unreachableHostMd5.insert(tempCmd5.getResult());
    }
    init_file.close();
}

void XCrawler::init_epoll() {
    int fd = epoll_create1(0);
    check(fd > 0, "epoll_create");

    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);

    epfd = fd;
}

static void *run(void *arg)
{
    ((XCrawler *)arg)->fetch();
    return NULL;
}

void XCrawler::start()
{
    pthread_t tid;
    int iRet = 0;

    iRet = pthread_create(&tid, NULL, run, this);
    if (iRet < 0) {
        perror("pthread_create");
        exit(0);
    }

    pthread_join(tid, NULL);
}

void XCrawler::fetch()
{
    int iRet = 0;
    string url;
    char reqBuf[MAXLINE];
    char htmlBody[HTMLSIZE];

    int iFd;
    string sUrl;
    int i;
    int size;

    while (true) {

        do {
            if (curConns < MAXCONNS) {
                iRet = make_connection(&iFd);
                if (iRet < 0) {
                    log_err("and_make_connection");
                    break;
                }

                iRet = fetch_url(sUrl);
                if (iRet < 0) {
                    log_err("fetch_url");
                    break;
                }
                
                size = MAXLINE;
                iRet = prepare_get_answer_request(reqBuf, &size, sUrl);
                if (iRet < 0) {
                    log_err("prepare_get_answer_request");
                    break;
                }

                iRet = write(iFd, reqBuf, size);
                if (iRet < 0) {
                    log_err("write");
                    break;
                }

                CrawlerState *pState= (CrawlerState *)malloc(sizeof(CrawlerState));
                pState->iFd = iFd;
                pState->iState = 0;
                memcpy(pState->base, sUrl.c_str(), sUrl.size());
                pState->iLen = sUrl.size();

                struct epoll_event event;
                event.data.ptr = (void *)pState;
                event.events = EPOLLIN | EPOLLET;

                iRet = epoll_ctl(epfd, EPOLL_CTL_ADD, iFd, &event);
                check(iRet == 0, "epoll_add");

                curConns++;
            }
        } while(0);
        
        int n = epoll_wait(epfd, events, MAXEVENTS, EPOLLTIMEOUT);
        check(n >= 0, "epoll_wait");

        CrawlerState *pState;
        int readCount;
        int last;

        for (i=0; i<n; i++) {
            pState = (CrawlerState *)events[i].data.ptr;
            readCount = last = 0;
            int iHtmlSize = HTMLSIZE;
            int iHeaderSize = MAXLINE;
            vector<string> vFollows;

            switch (pState->iState) {
                case 0:
                    iRet = get_response(pState->iFd, htmlBody, &iHtmlSize);
                    if (iRet < 0) {
                        log_err("get_response");
                        continue;
                    }

                    Parse::SearchAnswer(htmlBody, iHtmlSize, fResultOut);

                    // add followers link to queue
                    pState->iState++;
                    sUrl = string(pState->base, pState->iLen);

                    iRet = prepare_get_followers_request(reqBuf, &iHeaderSize, sUrl);
                    if (iRet < 0) {
                        log_err("prepare_get_followers_request");
                        continue;
                    }
                    
                    iRet = write(pState->iFd, reqBuf, iHeaderSize);
                    if (iRet < 0) {
                        log_err("write");
                        continue;
                    }

                    break;

                case 1:
                    iRet = get_response(pState->iFd, htmlBody, &iHtmlSize);
                    if (iRet < 0) {
                        log_err("get_response");
                        continue;
                    }

                    Parse::SearchFollowers(htmlBody, iHtmlSize, vFollows);

                    push_urls(vFollows);

                    // add followees link to queue
                    pState->iState++;
                    sUrl = string(pState->base, pState->iLen);

                    iRet = prepare_get_followees_request(reqBuf, &iHeaderSize, sUrl);
                    if (iRet < 0) {
                        log_err("prepare_get_followees_request");
                        continue;
                    }
                    
                    iRet = write(pState->iFd, reqBuf, iHeaderSize);
                    if (iRet < 0) {
                        log_err("write");
                        continue;
                    }

                    break;

                case 2:
                    iRet = get_response(pState->iFd, htmlBody, &iHtmlSize);
                    if (iRet < 0) {
                        log_err("get_response");
                        continue;
                    }

                    Parse::SearchFollowers(htmlBody, iHtmlSize, vFollows);

                    push_urls(vFollows);

                    iRet = fetch_url(sUrl);
                    if (iRet < 0) {
                        log_err("fetch_url");
                        break;
                    }

                    size = MAXLINE;
                    iRet = prepare_get_answer_request(reqBuf, &size, sUrl);
                    if (iRet < 0) {
                        log_err("prepare_get_answer_request");
                        break;
                    }

                    iRet = write(iFd, reqBuf, size);
                    if (iRet < 0) {
                        log_err("write");
                        break;
                    }

                    pState->iState = 0;
                    memcpy(pState->base, sUrl.c_str(), sUrl.size());
                    pState->iLen = sUrl.size();

                    break;

                default:
                    break;
            } 
        }

    } //end while
}

int XCrawler::get_response(int iFd, char *pHtmlBody, int *pHtmlSize) {
    int iRet = 0;
    int last = 0, nRead = 0;
    while (1) {
        nRead = read(iFd, pHtmlBody+last, *pHtmlSize-last);

        if (nRead == 0) {
            // EOF
            close(iFd);
            curConns--;
            *pHtmlSize = last;
            log_err("EOF");

            return -1;
        }

        if (nRead < 0) {
            if (errno != EAGAIN) {
                log_err("read err, and errno = %d", errno);
                iRet = -1;
                close(iFd);
                curConns--;
            }

            break;
        }
        
        last += nRead;
    }

    *pHtmlSize = last;
    return iRet;
}

int XCrawler::fetch_url(string &sUrl) {
    int iRet = 0;
    Url url;
    int i;

    while (1) {
    
        if(unvisitedUrl.empty()) {
            sleep(1);
            printf("thread %ld: no data to process\n", (long)(pthread_self()) % THREAD_NUM);
            return -1;
        }

        sUrl = unvisitedUrl.front();
        unvisitedUrl.pop();

        CMD5 urlDigest;
        urlDigest.GenerateMD5((unsigned char*)sUrl.c_str(), sUrl.length());
        if (visitedUrlMd5.find(urlDigest.getResult()) != visitedUrlMd5.end()) {
            continue;
        }

        visitedUrlMd5.insert(urlDigest.getResult());
        url.parse(sUrl);

        break;
    }

    log_info("fetch success! url = %s", url.getUrl().c_str());
    return 0;
}

int XCrawler::make_connection(int *pFd) {
    int iRet = 0;

    // make connection
    int iConnfd = socket(AF_INET, SOCK_STREAM, 0);
    if (iConnfd < 0) {
        log_err("socket");
        return -1;
    }

    struct hostent *server;
    struct sockaddr_in servAddr;

    //string ip = dnsMana.getIP(url.getHost());
    string ip = "60.28.215.98";
    
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(80);
    servAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    iRet = connect(iConnfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (iRet < 0) {
        log_err("connect");
        return iRet;
    }

    iRet = make_socket_non_blocking(iConnfd);
    if (iRet < 0) {
        log_err("make_socket_non_blocking");
        return -1;
    }

    *pFd = iConnfd;

    return iRet;
}

int XCrawler::prepare_get_answer_request(char *pReq, int *pSize, string &sUrl) {
    Url url(sUrl);
    int iRet = 0;
    iRet = snprintf(pReq, *pSize,
            "GET %s/answers?order_by=vote_num HTTP/1.1\r\n"
            "Host: www.zhihu.com\r\n"
            "Connection: keep-alive\r\n"
            "Cache-Control: max-age=0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n"
            "Accept-Language: zh-CN,zh;q=0.8\r\n"
            "Cookie: %s\r\n"
            "\r\n", url.getPath().c_str(), cookie.c_str());

    if (iRet < 0) {
        log_err("snprintf");
        return iRet;
    }

    *pSize = iRet;
    return 0;
}

int XCrawler::prepare_get_followers_request(char *pReq, int *pSize, string &sUrl) {
    Url url(sUrl);
    int iRet = 0;
    iRet = snprintf(pReq, *pSize,
            "GET %s/followers HTTP/1.1\r\n"
            "Host: www.zhihu.com\r\n"
            "Connection: keep-alive\r\n"
            "Cache-Control: max-age=0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n"
            "Accept-Language: zh-CN,zh;q=0.8\r\n"
            "Cookie: %s\r\n"
            "\r\n", url.getPath().c_str(), cookie.c_str());

    if (iRet < 0) {
        log_err("snprintf");
        return iRet;
    }

    *pSize = iRet;
    return 0;
}

int XCrawler::prepare_get_followees_request(char *pReq, int *pSize, string &sUrl) {
    Url url(sUrl);
    int iRet = 0;
    iRet = snprintf(pReq, *pSize,
            "GET %s/followees HTTP/1.1\r\n"
            "Host: www.zhihu.com\r\n"
            "Connection: keep-alive\r\n"
            "Cache-Control: max-age=0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n"
            "Accept-Language: zh-CN,zh;q=0.8\r\n"
            "Cookie: %s\r\n"
            "\r\n", url.getPath().c_str(), cookie.c_str());

    if (iRet < 0) {
        log_err("snprintf");
        return iRet;
    }

    *pSize = iRet;
    return 0;
}
int XCrawler::make_socket_non_blocking(int fd) {
    int flags, s;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        log_err("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if (s == -1) {
        log_err("fcntl");
        return -1;
    }

    return 0;
}

void XCrawler::push_urls(vector<string> &vFollows) {
    vector<string>::iterator it;
    for (it = vFollows.begin(); it != vFollows.end(); it++) {
        unvisitedUrl.push(*it);
    }
}
