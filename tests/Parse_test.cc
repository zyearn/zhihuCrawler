#include "Parse.h"
#include <iostream>
using namespace std;

int main() {
    cout << Parse::RemoveEscape("http::\\/\\/www.zhihu.com");

    return 0;
}
