#include "DNSManager.h"
using namespace std;

int main() {
    DNSManager dns;

    string result;
    result = dns.getIP("www.baidu.com");
    result = dns.getIP("www.zhihu.com");
    result = dns.getIP("www.sina.com");
    result = dns.getIP("www.qq.com");

    cout << "result = " << result << endl;

    result = dns.getIP("www.qq.com");
    cout << "result = " << result << endl;
    return 0;
}
