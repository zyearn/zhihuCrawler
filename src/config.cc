#include <string>
using namespace std;

/**
 * you can configure the crawler here
 */
int MAX_CRAWLER_DEEP=20;
int THREAD_NUM=150;

int CONNECT_TIMEOUT=5;
int DEFAULT_TIMEOUT=5;//read timeout
int MAX_WEBPAGE_BYTES=500*1024;//500KB


//define the files path
string SEEDS_FILE("./startfile/seeds.txt");
string VISITED_URL_MD5_FILE("./startfile/visitedUrlMd5.txt");
string UNREACHABLE_HOST_MD5_FILE("./startfile/unreachableHost.txt");

string RAW_DATA_FILE("./datafile/rawData.raw");

// put your cookie here
string cookie;
