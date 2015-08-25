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
 * @file XUrl.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#include "XUrl.h"

/**
 * XUrl represents a url,
 * currently, we don't support url that port is not 80
 * e.g. http://www.**.com::8080
 *
 * note that every url has a weight which is used in priority queue,
 * 10 is the biggest weight
 */
XUrl::XUrl(string url)
:m_url(url),m_weight(10)
{
	string::size_type pos=url.find("http://");
	if(pos==string::npos)
		m_url="http://"+url;
	
	pos=m_url.find('/',7);
	if(pos==string::npos)//such as www.g.cn
		m_host=m_url;
	else
	{
		m_host=m_url.substr(0,pos);
		m_path=m_url.substr(pos);
	}

	//calculate  weight
	string tempStr(m_url.substr(7));
	for(int i=0;i<tempStr.length();i++)
		if(tempStr[i]=='/'||tempStr[i]=='?'||tempStr[i]=='&')
			m_weight--;
	if(tempStr.find("search")!=string::npos)
		m_weight-=2;
	if(tempStr.find("proxy")!=string::npos)
		m_weight-=2;
	if(tempStr.find("gate")!=string::npos)
		m_weight-=2;
}

XUrl::~XUrl()
{
}

string XUrl::getUrl()//return with "http://"
{
	return m_url;
}

string XUrl::getHost()//return without "http://"
{
	return m_host.substr(7);
}

// http://www.g.cn/test-->return /test
// http://www.g.cn/    -->return empty string
// http://www.g.cn     -->return empty string
string XUrl::getPath()
{
	if(m_path.size()==1)//just one '/'
		return "";
	return m_path;
}

void XUrl::setWeight(int weight)
{
	m_weight=(weight>10)?10:weight;
}

int XUrl::getWeight()
{
	//return m_weight;
	return 10;
}

