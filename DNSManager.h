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
 * @file DNSManager.h
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#ifndef _DNSMANAGER_H_
#define _DNSMANAGER_H_
#include <string>
#include <set>
#include <vector>
#include <map>

using namespace std;

class DNSManager
{
	public:
		DNSManager();
		~DNSManager();

		string getIP(const string &host);
	private:
		bool isValidHostChar(char ch);
		bool isValidHost(const char *host);	
		bool isDomesticIP(string ip);
		bool isDomainValid(const string &host);


		 map<string,string>dnsCache;
		set<string>domainfilter;
};
#endif
