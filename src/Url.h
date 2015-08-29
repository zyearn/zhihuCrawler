#ifndef _URL_H
#define _URL_H

#include <string>
#include "dbg.h"

#define HTTPPROT    "http://"

using std::string;

class Url
{
public:
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

private:
    string m_url;
    string m_host;
    string m_path;
    //int m_weight;
};

#endif
