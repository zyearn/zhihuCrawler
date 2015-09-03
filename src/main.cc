#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "XCrawler.h"
#include "Url.h"
#include "config.h"
#include "webserver.h"

int pageNum = 0;

int main(int argc, char *argv[])
{
    WebServer server;
    server.run();

	XCrawler crawler;
	crawler.start();

	return 0;
}
