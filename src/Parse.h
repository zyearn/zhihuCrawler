#ifndef _PARSE_H
#define _PARSE_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "dbg.h"

using namespace std;

class Parse
{
public:
    Parse();
    ~Parse();

    static int doSearch(char *pContext, int iLen, ofstream &out);
};

#endif
