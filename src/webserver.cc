#include "webserver.h"

WebServer::WebServer() {
    int iRet = 0;
    iRet = gettimeofday(&startTime, NULL);
    if (iRet < 0) {
        fprintf(stderr, "error: gettimeofday\n");
        //TODO: throw an exception
        return;
    }
}

WebServer::~WebServer() {

}

void WebServer::run() {
    pthread_t serverId;
    int iRet = pthread_create(&serverId, NULL, WebServer::startWebServer, this);
    if (iRet < 0) {
        fprintf(stderr, "error: pthread_create\n");
    }

    return;
}

void *WebServer::startWebServer(void *arg) {
    WebServer *server = (WebServer *)arg;
    int iRet = 0;
    int listenfd, connfd;
    int optval = 1;
    struct sockaddr_in serverAddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        fprintf(stderr, "create socket error\n");
        return NULL;
    }
    
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0) {
        fprintf(stderr, "error: setsockopt\n");
        return NULL;
    }

    bzero(&serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    iRet = bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (iRet < 0) {
        fprintf(stderr, "bind error\n");
        exit(0);
    }

    printf("begin to listen on port 8080...\n");
    
    iRet = listen(listenfd, LISTENQ);
    if (iRet < 0) {
        fprintf(stderr, "error: listen\n");
        return NULL;
    }

    while(1)
    {
        connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            fprintf(stderr, "Web Server, Accept error: %s\n", strerror(errno));
            return NULL;
        }
        printf("accept!\n");

        server->sendHtml(connfd);
        //writeBody(connsock);
        //send(connsock,buff,sizeof(buff),0);
        close(connfd);
    }

    return NULL;
}

int WebServer::sendHtml(int sock)
{
	struct timeval nowtv;
	char cnowtv[100];
	gettimeofday(&nowtv,NULL);
	sprintf(cnowtv,"%ld",nowtv.tv_sec-startTime.tv_sec);

	char cpageNum[100];
	sprintf(cpageNum,"%d",pageNum);

	const string body="<html>\n<head><title>zhihuCrawler real time stat</title></head><body><p>"+string(cnowtv)+" seconds</p>"+string(cpageNum)+"</body></html>";
	//send(sock,body.c_str(),body.length(),0);

    char num[100];
    sprintf(num, "%lu", body.length());
	const string header="HTTP/1.0 200 OK\r\nServer: ZhihuCrawler\r\nContent-Type: text/html\r\nContent-Length: " + string(num) + "\r\n\r\n";
	if (send(sock, header.c_str(),header.length(),0) < 0 ||
	        send(sock,body.c_str(),body.length(),0) < 0) 
    {
        fprintf(stderr, "error: send\n");
        return -1;
    }

    return 0;
}
