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
 * @file DNSManager.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */

#include "DNSManager.h"
#include "XUrl.h"
#include "ThreadMutex.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<iostream>
#include "debug.h"

/**
 * DNSManager is in charge of maintain the DNS cache,
 * check if url is valid, and get ip from url
 */

ThreadMutex cacheMutex;
DNSManager::DNSManager()
{
}

DNSManager::~DNSManager()
{
}

bool DNSManager::isValidHostChar(char ch)
{
	return( isalpha(ch) || isdigit(ch)
		|| ch=='-' || ch=='.' || ch==':' || ch=='_');
}

bool DNSManager::isValidHost(const char *host)
{
	if( !host ){
		return false;
	}

	if( strlen(host) < 6 ){ 
		return false;
	}

	char ch;
	for(unsigned int i=0; i<strlen(host); i++){
		ch = *(host++);
		if( !isValidHostChar(ch) ){
			return false;
		}
	}

	return true;
}

/* description:
 * 	very import function, get the ip of url
 * 	the url will be checked , invalid url will be discarded
 *
 * attention:
 * if can not lookup the url from DNS,or the IP is out of the range of IP,
 * the function will return a empty string
 */
string DNSManager::getIP(const string &url)
{
	XUrl xurl(url);
	if(url.empty())
		return "";
	
	if(!isValidHost(xurl.getHost().c_str()))
		return "";


	in_addr_t inaddr=inet_addr(xurl.getHost().c_str());
	
	if(inaddr!=INADDR_NONE)//host is ip
	{
		if(isDomesticIP(xurl.getHost()))
			return xurl.getHost();
	}else
	{
		if(!isDomainValid(xurl.getHost()))
			return "";
		
		cacheMutex.lock();
		map<string,string>::iterator iter=dnsCache.find(xurl.getHost());
		if(iter!=dnsCache.end())//find it in cache
		{
			string ip=(*iter).second;
			inaddr=inet_addr(ip.c_str());
			if(inaddr!=INADDR_NONE)//yes,valid ip
			{
				cacheMutex.unlock();
				return ip;
			}
		} 
		cacheMutex.unlock();
		
		
	}		
#ifdef debug
		struct timeval starttv={0,0};
		memset(&starttv,0,sizeof(starttv));
		gettimeofday(&starttv,NULL);
#endif

	//not find from cache, so try to find from DNS
	struct hostent *host=gethostbyname(xurl.getHost().c_str());
#ifdef debug
		struct timeval endtv={0,0};
		memset(&endtv,0,sizeof(endtv));
		gettimeofday(&endtv,NULL);
		cout<<"dns time cost "<<endtv.tv_sec-starttv.tv_sec<<endl;

#endif

	if(host==NULL)
		return "";
	char **pp=host->h_addr_list;
	if(*pp==NULL)
		return "";
	const char *result=inet_ntoa(*(struct in_addr*)(*pp));
	if(result==NULL)		
		return "";
	string ips(result);
	if(!isDomesticIP(ips))
		return "";
	//insert into cache
	cacheMutex.lock();
	dnsCache.insert(map<string,string>::value_type(xurl.getHost(),string(result)));
	cacheMutex.unlock();
	//
#ifdef debug
	cout<<"DNS cache has "<<dnsCache.size()<<" items"<<endl;
#endif
	return ips;
		
}

/*
 * check if the ip is in the range of what we want
 * e.g. China or one university
 * we will add this feature in future
 */
bool DNSManager::isDomesticIP(string ip)
{
	return true;
}

/**
 * we just visit .cn .com .org .net
 */
bool DNSManager::isDomainValid(const string &host)
{
	string::size_type pos=host.rfind('.');
	if(pos!=string::npos)
	{
		string temp=host.substr(pos);
		if(temp==".cn"||temp==".com"||
				temp==".org"||temp==".net")
			return true;
	}
	return false;
}
