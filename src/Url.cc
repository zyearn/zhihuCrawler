#include "Url.h"

Url::Url(const string &url): m_url(url) {
    parse(url);
}

Url::Url() {

}

Url::~Url() {
}

void Url::parse(const string &sUrl) {
    m_url = sUrl;

	string::size_type pos = sUrl.find(HTTPPROT);
	if (pos == string::npos) {
		m_url = HTTPPROT + sUrl;
    }
	
	pos=m_url.find('/', strlen(HTTPPROT));
	if (pos == string::npos) {
        //such as www.g.cn
		m_host = m_url; 
        m_path = "/";
    } else {
		m_host = m_url.substr(0,pos);
		m_path = m_url.substr(pos);
	}

    //log_info("in UrL::parse, m_host = %s", m_host.c_str());
    //log_info("in UrL::parse, m_path = %s", m_path.c_str());
}

string Url::getUrl() {
	return m_url;
}

string Url::getHost()
{
	return m_host.substr(strlen(HTTPPROT));
}

string Url::getPath()
{
	return m_path;
}
