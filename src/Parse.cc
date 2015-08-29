#include "Parse.h"

const int COUNT_THREAD = 100;

Parse::Parse() {

}

Parse::~Parse() {

}

void Parse::doSearch(char *pContext, int iLen, ofstream &out) {
    
    string body = string(pContext, iLen);
    string::size_type pos, ques_pos;

    int cur_count;
    while ((pos = body.find("class=\"count\"")) != string::npos)
    {
        cur_count = atoi(body.c_str() + pos + 14);
        //out << "cur_count = " << cur_count << std::endl;

        body[pos] = '\0';
        string tmpbody(body.c_str());
        
        body = body.substr(pos + 1);
        if (cur_count < COUNT_THREAD)
        {
            //out << cur_count << " too small!\n";
            continue;
        }
        
        if ((ques_pos = tmpbody.rfind("/question/")) == string::npos)
        {
            //out << "question not found!!! strange thing\n";
            continue;
        }

        string question_tail = tmpbody.substr(ques_pos);
        if ((ques_pos = question_tail.find("\"")) == string::npos)
        {
            //out << "\" not found!! strange thing\n";
            continue;
        }

        string question = question_tail.substr(0, ques_pos);
        out << "count = " << cur_count << ", url = www.zhihu.com" << question << std::endl;
        out.flush();
    }   
    //std::cout << "out the doSearch!!!" << std::endl;
}
