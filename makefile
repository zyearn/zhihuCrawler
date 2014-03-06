objects= DNSManager.o CMd5.o RawDataFile.o ThreadMutex.o \
		 XCrawler.o XPage.o XUrl.o XHttpClient.o webserver.o main.o

CC= g++

#CFLAGS= -g
CFLAGS=
xyzCrawler:$(objects)
	$(CC) $(CFLAGS) -o xyzCrawler  $(objects) -lpthread
	
CMd5.o: CMd5.cc CMd5.h
	$(CC) $(CFLAGS) -c CMd5.cc
DNSManager.o: DNSManager.cc DNSManager.h XUrl.h ThreadMutex.h debug.h
	$(CC) $(CFLAGS) -c DNSManager.cc
main.o: main.cc CMd5.h XCrawler.h XUrl.h RawDataFile.h XPage.h config.h
	$(CC) $(CFLAGS) -c main.cc
RawDataFile.o: RawDataFile.cc RawDataFile.h XPage.h ThreadMutex.h
	$(CC) $(CFLAGS) -c RawDataFile.cc
ThreadMutex.o: ThreadMutex.cc ThreadMutex.h
	$(CC) $(CFLAGS) -c ThreadMutex.cc
webserver.o: webserver.cc config.h
	$(CC) $(CFLAGS) -c webserver.cc
XCrawler.o: XCrawler.cc XCrawler.h CMd5.h XUrl.h RawDataFile.h XPage.h \
  config.h ThreadMutex.h XHttpClient.h DNSManager.h debug.h
	$(CC) $(CFLAGS) -c XCrawler.cc
XHttpClient.o: XHttpClient.cc XHttpClient.h XPage.h XCrawler.h CMd5.h \
  XUrl.h RawDataFile.h config.h ThreadMutex.h DNSManager.h
	$(CC) $(CFLAGS) -c XHttpClient.cc
XPage.o: XPage.cc XPage.h debug.h
	$(CC) $(CFLAGS) -c XPage.cc
XUrl.o: XUrl.cc XUrl.h
	$(CC) $(CFLAGS) -c XUrl.cc

.PHONY:clean
clean:	
	-rm xyzCrawler $(objects)
