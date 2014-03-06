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
 * @file main.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#include <iostream>
#include <unistd.h>
#include "CMd5.h"
#include "XCrawler.h"
#include "XUrl.h"
#include "config.h"
#include <sys/time.h>
#include <time.h>

using namespace std;

//some statitics variable
int pageNum=0;
struct timeval starttime;
struct timeval endtime;
extern void* startWebserver(void*);
//
int main(int argc , char *argv[])
{

	pthread_t serverpidt;
	pthread_create(&serverpidt,NULL,startWebserver,NULL);
	gettimeofday(&starttime,NULL);

	XCrawler crawler;
	crawler.start();
	//XUrl url("");
	//cout<<url.getHost()<<endl;
	return 0;
}
