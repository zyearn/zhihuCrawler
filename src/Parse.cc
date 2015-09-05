#include "Parse.h"

const int COUNT_THREAD = 100;

Parse::Parse() {

}

Parse::~Parse() {

}

ofstream fBodyOut("./bodyOut");

int Parse::SearchAnswer(char *pContent, int iLen, ofstream &out) {

    string body = string(pContent, iLen);
    fBodyOut << "Bodyout: " << body << endl;
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

    return 0;
}

int Parse::SearchFollowers(char *pContent, int iLen, vector<string> &vFollow) {
    cout << "in the SearchFollowers, contentLen = " << iLen << endl;
    string body = string(pContent, iLen);
    fBodyOut << "Bodyout: " << body << endl;
    string::size_type pos, start, end;

    string follower;

    while ((pos = body.find("zm-list-content-title")) != string::npos) {

        // ajax的回复里面都是escape字符，所以有了下面的string
        start = body.find("http:\\/\\/www.zhihu.com\\/people\\/", pos);
        if (start == string::npos) {
            return 0;
        }

        int end = body.find("\"", start);
        if (end == string::npos) {
            return 0;
        }

        follower = RemoveEscape(body.substr(start, end - start));
        log_info("find follower!! %s", follower.c_str());

        body = body.substr(end + 1);
        vFollow.push_back(follower);
    }

    cout << "leave SearchFollowers" << endl;
    return 0;
}

string Parse::RemoveEscape(const string &str) {
    string sRes;
    string::value_type c;

    for (int i=0; i<str.size(); i++) {
        if (str[i] == '\\' && i != str.size()-1) {
            i++;
            sRes.push_back(str[i]);
        } else if (str[i] == '\\' && i == str.size()-1) {
            continue;
        } else {
            sRes.push_back(str[i]);
        }
    }

    return sRes;
}

int Parse::GetFollowCount(char *pContent, int iLen, int *pFollowee, int *pFollower) {
    string body = string(pContent, iLen);
    string::size_type pos, followeeSt, followerSt;

    if ((pos = body.find("zm-profile-side-following")) != string::npos) {
        followeeSt = body.find("<strong>", pos); 
        *pFollowee = atoi(body.c_str() + followeeSt + strlen("<strong>"));

        followerSt = body.find("<strong>", followeeSt + 1);
        *pFollower = atoi(body.c_str() + followerSt + strlen("<strong>"));
    
    } else {
        log_err("fatal err: can not find followee/er");
        return -1;
    }

    return 0;
}

int Parse::GetHashId(char *pContent, int iLen, char *pHash, int *pHashSize) {
    string body(pContent, iLen);
    string::size_type pos = body.find("zm-profile-header-op-btns");
    
    if (pos != string::npos) {
        string::size_type hash_start = body.find("data-id", pos) + 9;
        string::size_type hash_end = body.find("\"", hash_start);
        
        string result = body.substr(hash_start, hash_end-hash_start);
        memcpy(pHash, result.c_str(), result.size());
        *pHashSize = result.size();
    } else {
        log_err("can not find hash id");
        return -1;
    }

    return 0;
}

int Parse::GetXsrf(char *pContent, int iLen, char *pXsrf, int *pXsrfSize) {
    string body(pContent, iLen);
    string::size_type pos = body.find("_xsrf");

    if (pos != string::npos) {
        string::size_type xsrf_start = body.find("value", pos) + 7;
        string::size_type xsrf_end = body.find("\"", xsrf_start);
        string result = body.substr(xsrf_start, xsrf_end - xsrf_start);
        memcpy(pXsrf, result.c_str(), result.size());
        *pXsrfSize = result.size();
    } else {
        log_err("can not find hash id");
        return -1;
    }

    return 0;
}
