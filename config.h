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
 * @file config.h
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <string>
using namespace std;

/**
 * you can configure the crawler here
 */
const int MAX_CRAWLER_DEEP=20;
const int THREAD_NUM=150;

const int DEFAULT_TIMEOUT=5;//read timeout
const int CONNECT_TIMEOUT=5;
const int MAX_WEBPAGE_BYTES=500*1024;//500KB


//define the files path
const string SEEDS_FILE("./startfile/seeds.txt");
const string VISITED_URL_MD5_FILE("./startfile/visitedUrlMd5.txt");
const string VISITED_PAGE_MD5_FILE("./startfile/visitedPageMd5.txt");
const string UNREACHABLE_HOST_MD5_FILE("./startfile/unreachableHost.txt");
const string UNVISITED_URL_FILE("./startfile/unvisitedUrl.txt");
const string IP_CN_FILE("./startfile/ipCn.txt");

const string RAW_DATA_FILE("./datafile/rawData.raw");



/*
 * attention:should not modified
 */
extern int pageNum;
extern struct timeval starttime;
extern struct timeval endtime;
#endif
