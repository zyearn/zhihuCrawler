#include "Parse.h"

const int COUNT_THREAD = 100;

Parse::Parse() {

}

Parse::~Parse() {

}

int Parse::doSearch(char *pContext, int iLen, ofstream &out) {
    
    log_info("in the doSearch");
    string body = string(pContext, iLen);
    string::size_type pos, ques_pos, votecount_pos;
    string countString;

    int cur_count;
    while ((pos = body.find("zm-item-vote-info")) != string::npos)
    {
        countString = body.substr(pos, 100);
        votecount_pos = countString.find("data-votecount");

        cur_count = atoi(countString.c_str() + votecount_pos + strlen("data-votecount") + 2);
        log_info("find count %d", cur_count);

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
    log_info("leave doSearch");

    return 0;
}
