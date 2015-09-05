#include "Url.h"
#include <iostream>
using namespace std;

int main() {
    string sParams = "{\"offset\":20,\"order_by\":\"created\",\"hash_id\":\"c3eec602ab5320fd4f5f9d3dcff053aa\"}";

    cout << Url::encode(sParams) << endl;
    return 0;
}
