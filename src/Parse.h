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

    static int SearchAnswer(char *pContext, int iLen, ofstream &out);
    static int SearchFollowers(char *pContext, int iLen, vector<string> &vFollow);
};

#endif
