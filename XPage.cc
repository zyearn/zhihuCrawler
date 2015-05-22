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
 * @file XPage.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#include "XPage.h"
#include <stdexcept>
#include <cstdlib>
#include "debug.h"

/**
 * you can get the attributes of one web page from this class 
 */
const string ENCODING="Content-Encoding: ";
const string TYPE="Content-Type: ";//Content-Type: text/html; charset=GB2312
const string CHARSET="charset=";
const string LENGTH="Content-Length: ";
const string LOCATION="Location: ";

XPage::XPage():m_contentLength(-1)
{
}

XPage::XPage(const string &header,const string &body):m_contentLength(-1)
{
	parserHeader(header);
	parserBody(body);
}

XPage::~XPage()
{
}

string XPage::getBody() const 
{
	return m_body;
}

string XPage::getHeader() const
{
	return m_header;
}
//if successfully parsed, return 0
//otherwize return negative number
int XPage::parserHeader(const string &header)
{
	m_header=header;
#ifdef debug
	//cout<<header;
#endif

	try{
		m_statusCode=atoi(header.substr(9,3).c_str());

		string temp=header;
		string field;
		string::size_type pos,fieldpos;
		
		while((pos=temp.find("\r\n"))>0)
		{
#ifdef debug
			//cout<<temp.substr(0,pos)<<"---------"<<endl;
#endif
			field=temp.substr(0,pos);
			//////////////
			fieldpos=field.find(TYPE);
			if(fieldpos!=string::npos)
				m_contentType=field.substr(fieldpos+TYPE.length());
			else if( (fieldpos=field.find(ENCODING))!=string::npos)
				m_encoding=field.substr(fieldpos+ENCODING.length());
			else if( (fieldpos=field.find(LENGTH))!=string::npos)
				m_contentLength=atoi(field.substr(fieldpos+LENGTH.length()).c_str());
			else if( (fieldpos=field.find(LOCATION))!=string::npos)
				m_location=field.substr(fieldpos+LOCATION.length()).c_str();

			///////////////
			temp=temp.substr(pos+2);
			
		}//end while
	//some web server may output wrong header,so we have to catch the exception
	}catch(const out_of_range &excp)
	{
		cerr<<"the crawler catch out of range in parserHeader()" 
			"exception ,may be caused by the bad Web Server"<<endl;
		return -1;
	}
	
	return 0;

}

//if successfully parsed,return 0
//otherwize return negative number
int XPage::parserBody(const string &body)
{
	m_body.append(body);
}

int XPage::getStatusCode()
{
	return m_statusCode;
}

int XPage::getContentLength()
{
	return m_contentLength;
}

string XPage::getLocation()
{
	return m_location;
}

string XPage::getEncoding()
{
	return m_encoding;
}

string XPage::getContentType()
{
	return m_contentType;
}

string XPage::getCharset()
{
	string::size_type pos=m_contentType.find(CHARSET);
	if(pos!=string::npos)
		m_charset=m_contentType.substr(pos+CHARSET.length());
	return m_charset;
}

string XPage::getTransferEncoding()
{
	return m_transferEncoding;
}

string XPage::getUrl() const
{
	return m_url;
}

void XPage::setUrl(string url)
{
	m_url=url;
}


string XPage::getIp() const
{
	return m_ip;
}


void XPage::setIp(string ip)
{
	m_ip=ip;
}

//if no title ,return url
string XPage::getTitle()  
{
	string::size_type start=m_body.find("<title>");
	string::size_type end;
	if(start!=string::npos)
	{
		if((end=m_body.find("</title>"))!=string::npos)
		{
			m_title=m_body.substr(start+7,end-start-7);
			return m_title;
		}		
	}
	return getUrl();
}

/**
 * just find "href=", and try to find links 
 * note that this funtion provide an easy way to find links from web pages,
 * though it may get wrong links.
 * we will provid more accurate method to find links in future
 */
vector<string> XPage::getLinks()
{
	vector<string>links;
	if(m_header.empty())
		return links;
	string::size_type pos=0;
	string::size_type temp,tempt1,tempt2,temppos1,temppos2,temppos3,temppos4;
	while(  (pos=m_body.find("href=",pos))!=string::npos  )
	{
		if( (temppos1=m_body.find(" ",pos))==string::npos)
				temppos1=(unsigned int)-1;
		if( (temppos2=m_body.find(">",pos))==string::npos)
				temppos2=(unsigned int)-1;
		tempt1=temppos1<temppos2?temppos1:temppos2;

		if( (temppos3=m_body.find('\n',pos))==string::npos)
				temppos3=(unsigned int)-1;
		if( (temppos4=m_body.find(";",pos))==string::npos)
				temppos4=(unsigned int)-1;
		tempt2=temppos3<temppos4?temppos3:temppos4;

		temp=tempt1<tempt2?tempt1:tempt2;
	//	////////////////////////////////////////////////
		string rawlink=m_body.substr(pos+5,temp-pos-5);
		rawlinkfilter(rawlink);
		if(!rawlink.empty())
			links.push_back(rawlink);
		//cout<<m_body.substr(pos+5,temp-pos-5)<<endl;
		pos+=5;
	}
	return links;
}

/*
 * adjust a rawlink to normal link which can be crawled,
 * and discard obvious invalid links
 */
void XPage::rawlinkfilter( string &rawlink)
{
	
	if(rawlink.empty())
		return;
	//if(rawlink.find("\'")==0||rawlink.find("\"")==0)
    if(rawlink.at(0)=='\''||rawlink.at(0)=='\"')
		rawlink=rawlink.substr(1);
	//if(rawlink.find("\'")==rawlink.length()-1||
	//		rawlink.find("\"")==rawlink.length()-1)
	if(rawlink.empty())
		return;
	if(rawlink.at(rawlink.length()-1)=='\''||
			  rawlink.at(rawlink.length()-1)=='\"')
		rawlink=rawlink.substr(0,rawlink.length()-1);
	if((rawlink.length()>=6&&rawlink.substr(0,6)=="mailto")||
		(rawlink.length()>=3&&rawlink.substr(0,3)=="ftp")||
		(rawlink.length()>=10&&rawlink.substr(0,10)=="javascript")||
		(rawlink.length()>=1&&rawlink.at(0)=='#')||
		(rawlink.length()>=1&&rawlink.at(0)=='+'))
	{
		rawlink="";
		return;
	}
	
	if(rawlink.empty())
		return;
	if( isFilterLink(rawlink))//filter spam links
	{
		rawlink="";
		return;
	}

	//filter jpg,gif,mp3,wma,pdf,doc,swf,exe,rar,zip,rm files
	int templength=rawlink.length()-4;
	if( rawlink.rfind(".jpg")==templength||
			rawlink.rfind(".gif")==templength||
			rawlink.rfind(".mp3")==templength||
			rawlink.rfind(".wma")==templength||
			rawlink.rfind(".pdf")==templength||
			rawlink.rfind(".doc")==templength||
			rawlink.rfind(".swf")==templength||
			rawlink.rfind(".exe")==templength||
			rawlink.rfind(".rar")==templength||
			rawlink.rfind(".zip")==templength)
		rawlink="";

    string::size_type pos;
    if ((pos = rawlink.find("www.zhihu.com/people/")) == string::npos)
    {
        rawlink="";
        return;
    }
    if(rawlink.substr(pos+21).find("\\") != string::npos)
    {
        rawlink="";
        return;
    }
	//////////////////
	//debug
//	cout<<"-----"<<rawlink<<endl;
}

/* this code belong to TSE
 * Filter spam links
 * If it is, return ture; otherwise false
 */
bool XPage::isFilterLink(const string &plink)
{
	if( plink.size() > 256 ) return true;

	string link = plink, tmp;
	string::size_type idx = 0;

	// find two times following symbols, return false
	tmp = link;
	idx = tmp.find("?");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("?");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("-");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("+");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("&");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("&");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("//");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("//");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("http");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("http");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("misc");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("misc");
		if( idx != string::npos ) return true;
	}

	tmp = link;
	idx = tmp.find("ipb");
	if( idx != string::npos ){
		tmp = tmp.substr(idx+1);
		idx = tmp.find("ipb");
		if( idx != string::npos ) return true;
	}

	const char *filter_str[]={
	"cgi-bin",	"htbin",	"linder",	"srs5",		"uin-cgi", 
	"uhtbin",	"snapshot",	"=+",		"=-",		"script",
	"gate",		"search",	"clickfile",	"data/scop",	"names",
	"staff/",	"enter",	"user",		"mail",	"pst?",
	"find?",	"ccc?",		"fwd?",		"tcon?",	"&amp",
	"counter?",	"forum",	"cgisirsi",	"{",		"}",
	"proxy",	"login",	"00.pl?",	"sciserv.pl",	"sign.asp",
	"<",		">",		"review.asp?",	"result.asp?",	"keyword",
	"\"",		"'",		"php?s=",	"error",	"showdate",
	"niceprot.pl?",	"volue.asp?id",	".css",		".asp?month",	"prot.pl?",
	"msg.asp",	"register.asp", "database",	"reg.asp",	"qry?u",
	"p?msg",	"tj_all.asp?page", ".plot.",	"comment.php",	"nicezyme.pl?",
	"entr",		"compute-map?", "view-pdb?",	"list.cgi?",	"lists.cgi?",
	"details.pl?",	"aligner?",	"raw.pl?",	"interface.pl?","memcp.php?",
	"member.php?",	"post.php?",	"thread.php",	"bbs/",		"/bbs"
	};
	int filter_str_num = 75;

	
	for(int i=0; i<filter_str_num; i++){
		if( link.find(filter_str[i]) != string::npos)
		return true;
	}	

	return false;
}

