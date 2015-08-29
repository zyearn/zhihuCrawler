#include "Url.h"

#define HTTPPROT    "http://"

Url::Url(string &url): m_url(url) {
    parse(url);
}

Url::~Url() {
}

void Url::parse(const string &sUrl) {
    m_url = sUrl;

	string::size_type pos = url.find(HTTPPROT);
	if (pos == string::npos) {
		m_url = HTTPPROT + url;
    }
	
	pos=m_url.find('/', sizeo(HTTPROT));
	if (pos == string::npos) {
        //such as www.g.cn
		m_host = m_url; 
        m_path = "/";
    } else {
		m_host = m_url.substr(0,pos);
		m_path = m_url.substr(pos);
	}
}

string Url::getUrl() {
	return m_url;
}

string Url::getHost()
{
	return m_host.substr(sizeof(HTTPPROT));
}

string Url::getPath()
{
	return m_path;
}
