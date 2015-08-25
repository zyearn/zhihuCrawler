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
 * @file XCrawler.h
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#ifndef _CRAWLER_H_
#define _CRAWLER_H_
#include <string>
#include <vector>
#include "CMd5.h"
#include "XUrl.h"
#include "RawDataFile.h"

using namespace std;

struct md5comp
{
	bool operator()(const MD5 &d1, const MD5 &d2) const
	{
		for(int i=0;i<4;i++)
		{
			if(d1.data[i]<d2.data[i])
				return true;
			else if(d1.data[i]>d2.data[i])
				return false;
		}
		return false;//make sure the when A equal B	
	}
};

struct urlcomp
{
	bool operator()(const string &url1,const string &url2) const
	{
		XUrl u1(url1),u2(url2);
		return u1.getWeight()<u2.getWeight();
	}
};


class XCrawler
{
	public:
		XCrawler();
		~XCrawler();

		void start();
		void fetch();
	private:
		void init();
		void addUrl( string curUrl,const vector<string> &links,int urldeep);
		RawDataFile m_rawfile;
};

#endif
