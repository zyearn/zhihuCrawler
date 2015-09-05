#include "XCrawler.h"

/**
 * The most important class.
 * This class start thread, maintain priority queue ,fetch web page,
 * add links to priority queue etc.
 * Note that currently we have not implemented all the futures provided here
 * 
 */

queue<string> unvisitedUrl;
set<string> visitedUrl;
DNSManager dnsMana;
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

    while (getline(init_file, strLine)) {
        visitedUrl.insert(strLine);
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

    int iFd;
    string sUrl;
    int i;
    int size;

    while (true) {

        do {
            if (curConns < MAXCONNS) {
                iRet = fetch_url(sUrl);
                if (iRet < 0) {
                    log_err("fetch_url");
                    break;
                }

                iRet = make_connection(&iFd);
                if (iRet < 0) {
                    log_err("and_make_connection");
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
                pState->iLast = 0;

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

        for (i=0; i<n; i++) {
            pState = (CrawlerState *)events[i].data.ptr;
            int iHeaderSize = MAXLINE;
            vector<string> vFollows;

            switch (pState->iState) {
                case 0:
                    iRet = get_response(pState);
                    if (iRet < 0) {
                        log_err("get_response");
                        continue;
                    }
                    
                    // parse html
                    iRet = is_valid_html(pState->htmlBody, pState->iLast);
                    if (iRet != 0) {
                        break;
                    }

                    Parse::SearchAnswer(pState->htmlBody, pState->iLast, fResultOut);

                    Parse::GetFollowCount(pState->htmlBody, pState->iLast, &(pState->iFolloweeCount), &(pState->iFollowerCount));

                    Parse::GetHashId(pState->htmlBody, pState->iLast, pState->hashId, &(pState->iHashIdSize));

                    Parse::GetXsrf(pState->htmlBody, pState->iLast, pState->xsrf, &(pState->iXsrfSize));

                    // how many request needed
                    pState->iFolloweeCount += (USERSPERREQ - 1);
                    pState->iFolloweeCount /= USERSPERREQ;
                    pState->iFolloweeCur = 0;

                    pState->iFollowerCount += (USERSPERREQ - 1);
                    pState->iFollowerCount /= USERSPERREQ;
                    pState->iFollowerCur = 0;

                    log_info("ee = %d, er = %d", pState->iFolloweeCount, pState->iFollowerCount);

                    pState->iState++;
                    pState->iLast = 0;
                    sUrl = string(pState->base, pState->iLen);

                    // add followers link to queue
                    iRet = prepare_get_followers_request(reqBuf, &iHeaderSize, sUrl, pState->iFollowerCur * USERSPERREQ, pState);
                    if (iRet < 0) {
                        log_err("prepare_get_followers_request");
                        continue;
                    }
                    
                    iRet = write(pState->iFd, reqBuf, iHeaderSize);
                    if (iRet < 0) {
                        log_err("write");
                        continue;
                    }

                    pState->iFollowerCur++;

                    break;

                case 1:
                    iRet = get_response(pState);
                    if (iRet < 0) {
                        log_err("get_response");

                        if (iRet == EEOF) {
                            iRet = make_connection(&(pState->iFd));
                            if (iRet < 0) {
                                log_err("and_make_connection");
                                break;
                            }

                            log_info("make connection suc!");
                            struct epoll_event event;
                            event.data.ptr = (void *)pState;
                            event.events = EPOLLIN | EPOLLET;

                            iRet = epoll_ctl(epfd, EPOLL_CTL_ADD, pState->iFd, &event);
                            check(iRet == 0, "epoll_add");

                            curConns++;
                        } else {
                            break;
                        }
                    } else {

                        iRet = is_valid_html(pState->htmlBody, pState->iLast);
                        if (iRet != 0) {
                            break;
                        }

                    }

                    Parse::SearchFollowers(pState->htmlBody, pState->iLast, vFollows);
                    if (pState->iFollowerCur != pState->iFollowerCount) {
                        // more followers! need get followers again
                        log_info("more followers! need get followers again cur = %d, target = %d", pState->iFollowerCur, pState->iFollowerCount);
                        sUrl = string(pState->base, pState->iLen);

                        iHeaderSize = HTMLSIZE;
                        iRet = prepare_get_followers_request(reqBuf, &iHeaderSize, sUrl, pState->iFollowerCur * USERSPERREQ, pState);
                        if (iRet < 0) {
                            log_err("prepare_get_followers_request");
                            continue;
                        }
                        
                        iRet = write(pState->iFd, reqBuf, iHeaderSize);
                        if (iRet < 0) {
                            log_err("write");
                            continue;
                        }

                        pState->iFollowerCur++;
                        pState->iLast = 0;
                        log_info("send get follower succ!");
                    } else {
                        log_info("complete!! get all followers of %.*s", pState->iLen, pState->base);
                    }

                    /*
                    push_urls(vFollows);

                    // add followees link to queue
                    pState->iState++;
                    pState->iLast = 0;
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

                    */
                    break;

                case 2:
                    iRet = get_response(pState);
                    if (iRet < 0) {
                        log_err("get_response");
                        continue;
                    }

                    iRet = is_valid_html(pState->htmlBody, pState->iLast);
                    if (iRet != 0) {
                        break;
                    }

                    Parse::SearchFollowers(pState->htmlBody, pState->iLast, vFollows);

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
                    pState->iLast = 0;
                    memcpy(pState->base, sUrl.c_str(), sUrl.size());
                    pState->iLen = sUrl.size();

                    break;

                default:
                    break;
            } 
        }

        //usleep(100000);

    } //end while
}

int XCrawler::is_valid_html(char *pHtml, int iSize) {
    char *pCLpos;
    pHtml[iSize] = '\0';
    if ((pCLpos = strstr(pHtml, "Content-Length: ")) == NULL) {
        return -1;
    }

    int iContentLen = atoi(pCLpos + strlen("Content-Length: "));
    printf("Content-Length: %d\n", iContentLen);


    char *pCRLF = strstr(pHtml, "\r\n\r\n");
    if (pCRLF == NULL) {
        return -1;
    }

    int iTrueLen = pCRLF - pHtml + strlen("\r\n\r\n") + iContentLen;

    if (iSize < iTrueLen) {
        return -1;
    }

    if (iSize > iTrueLen) {
        log_err("iSize = %d, iTrueLen = %d", iSize, iTrueLen);
    }
    return 0;
}

int XCrawler::get_response(CrawlerState *pState) {
    int iFd = pState->iFd;
    int iLast = pState->iLast;
    int iRet = 0;
    int nRead = 0;
    int first = 1;
    while (1) {
        nRead = read(iFd, pState->htmlBody + iLast, HTMLSIZE - iLast);

        if (nRead == 0) {
            // EOF
            if (first) log_info("first loop read EOF");
            else  log_info("not first loop rend EOF");
            log_err("EOF");
            close(iFd);
            curConns--;
            return EEOF;
        }

        if (nRead < 0) {
            if (errno != EAGAIN) {
                log_err("read err, and errno = %d", errno);
                goto err;
            }

            break;
        }
        
        iLast += nRead;
        first = 0;
    }

    pState->iLast = iLast;

    return iRet;

err:
    close(iFd);
    //free(pState);
    curConns--;
    return -1;
}

int XCrawler::fetch_url(string &sUrl) {
    int iRet = 0;
    int i;

    while (1) {
    
        if(unvisitedUrl.empty()) {
            sleep(1);
            printf("thread %ld: no data to process\n", (long)(pthread_self()) % THREAD_NUM);
            return -1;
        }

        sUrl = unvisitedUrl.front();
        unvisitedUrl.pop();


        break;
    }

    log_info("fetch success! url = %s", sUrl.c_str());
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

int XCrawler::prepare_get_followers_request(char *pReq, int *pSize, string &sUrl, int iCur, CrawlerState *pState) {
    ostringstream oss; 
    Url url(sUrl);
    int iRet = 0;
    string hashId(pState->hashId, pState->iHashIdSize);
    string _xsrf(pState->xsrf, pState->iXsrfSize);

    oss << "{\"offset\":" << iCur << ",\"order_by\":\"created\",\"hash_id\":\"" << hashId << "\"}";
    string sParams = oss.str();
    char postBody[MAXLINE];

    iRet = snprintf(postBody, MAXLINE, 
            "method=next&params=%s&_xsrf=%s",
            Url::encode(sParams).c_str(), _xsrf.c_str()
            );

    if (iRet < 0) {
        log_err("snprintf postBody");
        return iRet;
    }
    
    int iContentLen = iRet;
    postBody[iContentLen] = '\0';

    //iRet = snprintf(pReq, *pSize,
    //        "GET %s/followers HTTP/1.1\r\n"
    //        "Host: www.zhihu.com\r\n"
    //        "Connection: keep-alive\r\n"
    //        "Cache-Control: max-age=0\r\n"
    //        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    //        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n"
    //        "Referer: %s\r\n"
    //        "Accept-Language: zh-CN,zh;q=0.8\r\n"
    //        "Cookie: %s\r\n"
    //        "\r\n", url.getPath().c_str(), sUrl.c_str(), cookie.c_str());
    iRet = snprintf(pReq, *pSize,
            "POST /node/ProfileFollowersListV2 HTTP/1.1\r\n"
            "Host: www.zhihu.com\r\n"
            "Connection: keep-alive\r\n"
            "Content-Length: %d\r\n"
            "Accept: */*\r\n"
            "Origin: http://www.zhihu.com\r\n"
            "X-Requested-With: XMLHttpRequest\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n"
            "Referer: %s/followers\r\n"
            "Accept-Language: zh-CN,zh;q=0.8\r\n"
            "Cookie: %s\r\n"
            "\r\n"
            "%s",
            iContentLen, sUrl.c_str(), cookie.c_str(), postBody
            );
    if (iRet < 0) {
        log_err("snprintf");
        return iRet;
    }

    printf("post followers:\n%.*s", iRet, pReq);

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

        if (visitedUrl.find(*it) != visitedUrl.end()) {
            continue;
        }

        visitedUrl.insert(*it);
        unvisitedUrl.push(*it);
    }
}
