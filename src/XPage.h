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
 * @file XPage.h
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#ifndef _XPAGE_H_
#define _XPAGE_H_

#include <string>
#include <vector>

using namespace std;

class XPage
{
public:
	XPage();
	XPage(const string &header,const string &body);
	~XPage();

	//parser
	int parserHeader(const string &header);
	int parserBody(const string &body);
	//get header
	int getStatusCode();
	int getContentLength();
	string getLocation();
	string getEncoding();
	string getContentType();
	string getCharset();
	string getTransferEncoding();

	//get body
	string getBody() const;
	
	//get links
	vector<string> getLinks();
	
	//get header
	string getHeader() const;

	//url
	string getUrl() const;
	void setUrl(string url);

	//ip
	string getIp() const;
	void setIp(string ip);

	//get title
	 string getTitle();
private:
    //url
    string m_url;

    //ip
    string m_ip;

    //header
    int m_headerLen;

    int m_statusCode;
    int m_contentLength;
    string m_location;
    string m_encoding;
    string m_contentType;
    string m_charset;
    string m_transferEncoding;
    bool m_connectionState;

    //body
    string m_body;

    //header 
    string m_header;

    //title
     string m_title;
private:
    void rawlinkfilter( string &rawlink);
    bool isFilterLink(const string &plink);

};
#endif
