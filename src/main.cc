#include <iostream>
#include <unistd.h>
#include "CMd5.h"
#include "XCrawler.h"
#include "XUrl.h"
#include "config.h"
#include "webserver.h"
#include <sys/time.h>
#include <time.h>

int pageNum=0;

int main(int argc , char *argv[])
{
    WebServer server;
    server.run();

	XCrawler crawler;
	crawler.start();

	return 0;
}
