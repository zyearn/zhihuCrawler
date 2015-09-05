#ifndef _PARSE_H
#define _PARSE_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>

#include "dbg.h"

using namespace std;

class Parse
{
public:
    Parse();
    ~Parse();

    static int SearchAnswer(char *pContent, int iLen, ofstream &out);
    static int SearchFollowers(char *pContent, int iLen, vector<string> &vFollow);
    static int GetFollowCount(char *pContent, int iLen, int *pFollowee, int *pFollower);
    static int GetHashId(char *pContent, int iLen, char *pHash, int *pHashSize);
    static int GetXsrf(char *pContent, int iLen, char *pXsrf, int *pXsrfSize);

    //helpers:
    static string RemoveEscape(const string &str);
};

#endif
