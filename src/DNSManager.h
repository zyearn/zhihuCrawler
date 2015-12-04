#ifndef _DNSMANAGER_H_
#define _DNSMANAGER_H_

#include <cstring>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Url.h"
#include "dbg.h"

using namespace std;

/*
 * DNSManager is in charge of maintaining the DNS cache,
 * check if url is valid, and get ip from url
 */

class DNSManager {
public:
    DNSManager();
    ~DNSManager();

    /*
     * get the ip of url
     * url will be checked, and invalid url will be discarded
     */
    string getIP(const string &host);
private:
    bool isValidHostChar(char ch);
    bool isValidHost(const char *host); 

    /*
     * check if the ip is in the range of what we want
     * e.g. China or one university
     */
    bool isDomesticIP(string ip);

    /*
     * we just visit .cn .com .org .net
     */
    bool isDomainValid(const string &host);

private:
    map<string, string> m_dnsCache;
};

#endif
