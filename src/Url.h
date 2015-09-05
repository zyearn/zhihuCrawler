#ifndef _URL_H
#define _URL_H

#include <string>
#include <cctype>
#include <iomanip>
#include <sstream>
#include "dbg.h"

#define HTTPPROT    "http://"

using std::string;

class Url {
public:
    Url();
    Url(const string &url);
    ~Url();
    
    void parse(const string &sUrl);

    /*
    * return with "http://"
    */
    string getUrl();

    /*
    * return without "http://"
    */
    string getHost();

    string getPath();

    static string encode(const string &value);

private:
    string m_url;
    string m_host;
    string m_path;
    //int m_weight;
};

#endif
