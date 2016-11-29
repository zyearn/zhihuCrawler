#include "DNSManager.h"

DNSManager::DNSManager() {

}

DNSManager::~DNSManager() {

}

bool DNSManager::isValidHostChar(char ch)
{
    return (isalpha(ch) || isdigit(ch)
        || ch == '-' || ch == '.' || ch == ':' || ch == '_');
}

bool DNSManager::isValidHost(const char *host)
{
    if (!host) {
        log_err("host == NULL");
        return false;
    }

    if (strlen(host) < strlen(HTTPPROT)) { 
        log_err("len of host is too small");
        return false;
    }

    char ch;
    int len = strlen(host);
    for (unsigned int i=0; i<len; i++){
        ch = *(host++);
        if (!isValidHostChar(ch)) {
            log_err("%c is not a valid char in %s", ch, host);
            return false;
        }
    }

    return true;
}

string DNSManager::getIP(const string &sUrl)
{
    Url url(sUrl);
    const char *purl = url.getHost().c_str();
    //log_info("purl = %s", purl);
    
    if (!isValidHost(purl)) {
        log_err("%s is not a valid host", purl);
        return "";
    }

    in_addr_t inaddr = inet_addr(purl);
    
    if (inaddr != INADDR_NONE) {
        //host is ip
        log_info("%s is an ip", purl);
        if (isDomesticIP(url.getHost())) {
            return url.getHost();
        }
    }

    if (!isDomainValid(url.getHost())) {
        log_info("%s is not valid", purl);
        return "";
    }

    map<string, string>::iterator iter = m_dnsCache.find(url.getHost());
    if (iter != m_dnsCache.end()) {
        //find it in cache
        string ip = iter->second;
        inaddr = inet_addr(ip.c_str());
        if (inaddr != INADDR_NONE) {
            //valid ip
            return ip;
        }
    } 
    
    //not find from cache, try to find from DNS
    struct hostent *host = gethostbyname(purl);

    if (host == NULL) {
        log_err("gethostbyname");
        return "";
    }

    char **pp = host->h_addr_list;
    if (*pp == NULL) {
        return "";
    }

    const char *result = inet_ntoa(*(struct in_addr*)(*pp));
    if (result == NULL) {
        return "";
    }

    string ips(result);
    if (!isDomesticIP(ips))
        return "";

    //insert into cache
    m_dnsCache.insert(map<string,string>::value_type(url.getHost(), ips));

    printf("dns insert %s:%s\n", url.getHost().c_str(), ips.c_str());
    return ips;
}

bool DNSManager::isDomesticIP(string ip) {
    return true;
}

bool DNSManager::isDomainValid(const string &host)
{
    string::size_type pos=host.rfind('.');
    
    if (pos != string::npos) {
        string temp = host.substr(pos);
        if (temp == ".cn" || temp==".com" ||
                temp == ".org" || temp == ".net") {

            return true;
        }
    }
    return false;
}
