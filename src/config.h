#include <string>
#include <queue>
using std::string;
using std::queue;

/**
 * you can configure the crawler here
 */
extern int MAX_CRAWLER_DEEP;
extern int THREAD_NUM;

extern int DEFAULT_TIMEOUT;
extern int CONNECT_TIMEOUT;
extern int MAX_WEBPAGE_BYTES;

//define the files path
extern string SEEDS_FILE;
extern string VISITED_URL_MD5_FILE;
extern string UNREACHABLE_HOST_MD5_FILE;

extern string RAW_DATA_FILE;

extern int pageNum;
extern queue<string> unvisitedUrl;
extern string cookie;
