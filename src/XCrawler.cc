/*
 * Copyright (C) 2008 xyzse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file XCrawler.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 *
 * @modified on 3/6/2014, by zjs
 */

#include "XCrawler.h"
#include "XUrl.h"
#include "config.h"
#include "ThreadMutex.h"
#include "XHttpClient.h"
#include "DNSManager.h"
#include "XPage.h"
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <iterator>
#include <queue>
#include <signal.h>
#include <cstdlib>
#include "debug.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/**
 * The most important class.
 * This class start thread, maintain priority queue ,fetch web page,
 * add links to priority queue etc.
 * Note that currently we have not implemented all the futures provided here
 * 
 */

//
priority_queue<string,vector<string>,urlcomp>unvisitedUrl[MAX_CRAWLER_DEEP];

set<MD5,md5comp>visitedUrlMd5;

set<MD5,md5comp>visitedPageMd5;

set<MD5,md5comp>unreachableHostMd5;

//
ThreadMutex mutex_unvisitedUrl[MAX_CRAWLER_DEEP]; 
ThreadMutex mutex_visitedUrl;
ThreadMutex mutex_unreachable_host;
ThreadMutex mutex_processlink;
ThreadMutex mutex_pagenum;
//
DNSManager dnsMana;
//
ofstream fout("./processlink");

XCrawler::XCrawler():m_rawfile(RAW_DATA_FILE)
{
    init();
}

XCrawler::~XCrawler()
{

}


static void sig_stop_handler(int sig)
{
    cout<<"catch term signal"<<endl;
    //do something to save current status
}

void XCrawler::init()
{
    signal(SIGTERM,sig_stop_handler);

    try{
        string strLine;

        ifstream init_file;
        init_file.open(SEEDS_FILE.c_str(),ios::binary);
        if(!init_file)
            throw exception();

        while(getline(init_file,strLine))
            unvisitedUrl[0].push(strLine);
        init_file.close();

        init_file.open(VISITED_URL_MD5_FILE.c_str(),ios::binary);
        if(!init_file)
            throw exception();

        while(getline(init_file,strLine))
        {
            CMD5 tempCmd5(strLine.c_str());
            visitedUrlMd5.insert(tempCmd5.getResult());
        }
        init_file.close();

        init_file.open(VISITED_PAGE_MD5_FILE.c_str(),ios::binary);
        if(!init_file)
            throw exception();

        while(getline(init_file,strLine))
        {
            CMD5 tempCmd5(strLine.c_str());
            visitedPageMd5.insert(tempCmd5.getResult());
        }
        init_file.close();

        init_file.open(UNREACHABLE_HOST_MD5_FILE.c_str(),ios::binary);
        if(!init_file)
            throw exception();
        while(getline(init_file,strLine))
        {
            CMD5 tempCmd5(strLine.c_str());
            unreachableHostMd5.insert(tempCmd5.getResult());
        }
        init_file.close();

        init_file.open(UNVISITED_URL_FILE.c_str(),ios::binary);
        if(!init_file)
            throw exception();
        //do something
        while(getline(init_file,strLine))
        {
            //unvisitedUrl[0].push(strLine);
        }
        //
        init_file.close();

        init_file.open(IP_CN_FILE.c_str(),ios::binary);
        if(!init_file)
            throw exception();
        //do something
        //
        init_file.close();


    }
    catch(exception)
    {
        cerr<<"error:can not open file"<<endl;
        exit(1);
    }	

}

void *run(void *arg)
{
    ((XCrawler *)arg)->fetch();
    return NULL;
}

void XCrawler::start()
{
    pthread_t tids[THREAD_NUM];
    for(int i=0;i<THREAD_NUM;i++)
        if(pthread_create(&tids[i],NULL,run,this)) 
        {
            perror("pthread_create");
            exit(2);
        }

    for(int i=0;i<THREAD_NUM;i++)
        pthread_join(tids[i],NULL);
}

void XCrawler::fetch()
{
    string url("");
    //XHttpClient http;

    while(1)
    {
        int i;
        for(i=0;i<MAX_CRAWLER_DEEP;i++)
        {
            mutex_unvisitedUrl[i].lock();

            if(unvisitedUrl[i].empty())
            {
                mutex_unvisitedUrl[i].unlock();
                continue;
            }
            url=unvisitedUrl[i].top();
            unvisitedUrl[i].pop();

            mutex_unvisitedUrl[i].unlock();
            break;
        }

        CMD5 urldigest;
        urldigest.GenerateMD5((unsigned char*)url.c_str(), url.length());
        mutex_visitedUrl.lock();
        if( visitedUrlMd5.find(urldigest.getResult()) != visitedUrlMd5.end())
        {
            mutex_visitedUrl.unlock();
            continue;
        }
        mutex_visitedUrl.unlock();

        if (i < MAX_CRAWLER_DEEP)
        {
            mutex_processlink.lock();
            fout << "curdeep = " << i << ", url = " << url << std::endl;
            fout.flush();
            mutex_processlink.unlock();
        }
        //if (i < MAX_CRAWLER_DEEP)
        //    std::cout << "url=" << url << std::endl;    

        XPage page;
        vector<string>links;
        if(i<MAX_CRAWLER_DEEP)//i+1 is the url deepth
        {

            //ofstream foutt("./iMAX", ios_base::app);
            //foutt << "i<MAX, url = " << url << std::endl;

            CMD5 url_md5;
            url_md5.GenerateMD5((unsigned char*)url.c_str(),url.length());
            mutex_visitedUrl.lock();
            visitedUrlMd5.insert(url_md5.getResult());
            mutex_visitedUrl.unlock();
            //foutt<<"add MD5: "<<url_md5.ToString()<<endl <<endl;
            //foutt.close();
            //pageNum++;
#ifdef debug
            //	struct timeval starttv={0,0};
            //	memset(&starttv,0,sizeof(starttv));
            //	gettimeofday(&starttv,NULL);
            //cout<<pthread_self()%THREAD_NUM<<"is going to download"<<url<<endl;
#endif
            XHttpClient http;
            int	retd=http.download(url+"/answers?order_by=vote_num",page);
#ifdef debug
            //	struct timeval endtv={0,0};
            //	memset(&endtv,0,sizeof(endtv));
            //	gettimeofday(&endtv,NULL);
            //	cout<<"time cost "<<endtv.tv_sec-starttv.tv_sec<<endl;

#endif

            if(retd==0)
            {
                //cout<<page.getHeader();//test
                //cout<<page.getBody()<<endl;//test

                //the MAX_DEEP's page need not to get links

                m_rawfile.doSearch(page);
                mutex_pagenum.lock();
                pageNum++;
                mutex_pagenum.unlock();

                if(i<MAX_CRAWLER_DEEP-1)
                {
                    XPage fer_page;
                    XHttpClient fer_http;
                    if (fer_http.download(url + "/followers", fer_page) != 0)
                    {
                        cerr << "download follower error\n" << std::endl;
                    }
                    else
                        links = fer_page.getLinks();

                    if(!links.empty())
                        addUrl(url,links,i);
                    links.clear();

                    XPage fee_page;
                    XHttpClient fee_http;
                    if (fee_http.download(url + "/followees", fee_page) != 0)
                    {
                        cerr << "download followee error\n" << std::endl;
                    }
                    else
                        links = fee_page.getLinks();

                    if(!links.empty())
                        addUrl(url,links,i);
                    links.clear();
                }
                //////////redirect
                if(page.getStatusCode()==301||page.getStatusCode()==302)//redirect to location
                {
                    cout<<"found redirection:: "<<page.getLocation()<<endl;
                }

                /////////put the url into visitedUurl 
                /*	CMD5 url_md5;
                    url_md5.GenerateMD5((unsigned char*)url.c_str(),url.length());
                    mutex_visitedUrl.lock();
                    visitedUrlMd5.insert(url_md5.getResult());
                    mutex_visitedUrl.unlock();
                //cout<<"add MD5: "<<url_md5.ToString()<<endl;
                pageNum++;*/
                //pageNum++;
                //
                //m_rawfile.write(page);
            }else
            {
                //cout<<retd<<endl;//test
                if(retd==-6)
                    cout<<"bad status code from "<<url<<" "<<page.getStatusCode()<<endl;

                CMD5 unreachhost_md5;
                XUrl badurl(url);
                string badhost=badurl.getHost();
                unreachhost_md5.GenerateMD5((unsigned char*)badhost.c_str(),badhost.length());
                mutex_unreachable_host.lock();
                unreachableHostMd5.insert(unreachhost_md5.getResult());
                mutex_unreachable_host.unlock();

                mutex_visitedUrl.lock();
                ofstream fout("./cantDownload", ios_base::app);
                fout << "cant download: " << url << ", push again"<< std::endl;
                fout.close();
                set<MD5,md5comp>::iterator it = visitedUrlMd5.find(url_md5.getResult());
                visitedUrlMd5.erase(it);                
                mutex_visitedUrl.unlock();

                mutex_unvisitedUrl[0].lock();
                unvisitedUrl[0].push(url);
                mutex_unvisitedUrl[0].unlock();
            }


        }//end if


#ifdef debug
        //cout<<"queue0 size: "<<unvisitedUrl[0].size()<<", ";
        //cout<<"queue1 size: "<<unvisitedUrl[1].size()<<endl;
        /*cout<<unvisitedUrl[2].size()<<endl;
          const int number=1;
          while(!unvisitedUrl[number].empty())
          {
          cout<<unvisitedUrl[number].top()<<endl;
          unvisitedUrl[number].pop();
          }*/
#endif

        if(i==MAX_CRAWLER_DEEP)
        {
            sleep(1);
            printf("thread %ld: no data to process\n", (long)(pthread_self()) % THREAD_NUM);
        }
        usleep(100000);
    }//end while
}

//urldeep is curUrl's deep ,begin with 0
void XCrawler::addUrl( string curUrl,const vector<string> &links,int urldeep)
{
    vector<string>::const_iterator iter_link;
    string templink;
    for( iter_link=links.begin();iter_link!=links.end();iter_link++)
    {
        templink=*iter_link;

        if(templink.empty()||templink.length()>256)
            continue;
        /*
           if( (templink.length()>= 4 && templink.substr(0,4)!="http") ||
           templink.length()<4)//if true,mean it's reference link
           {
           if(templink.at(0)=='.')//this version we temporily do not deal with this situation
           continue;
           if( (curUrl.rfind("/")!=curUrl.length()-1 && templink.substr(0,1)=="/")||
           (curUrl.rfind("/")==curUrl.length()-1 && templink.substr(0,1)!="/")) 
           templink=curUrl+templink;
           else if(curUrl.rfind("/")!=curUrl.length()-1 && templink.substr(0,1)!="/"  )
           templink=curUrl+"/"+templink;
           else
           {
           templink=curUrl+templink.substr(1);
           }

           }	
         */

#ifdef debug
        //cout<<templink<<endl;
#endif

        //
        CMD5 urldigest;
        urldigest.GenerateMD5((unsigned char*)templink.c_str(),templink.length());
        mutex_visitedUrl.lock();
        //ofstream fout("./hasvisit", ios_base::app);
        if( visitedUrlMd5.find(urldigest.getResult()) != visitedUrlMd5.end())
        {
            //fout<<"!!!!!!!has visited :"<<templink<<endl;
            mutex_visitedUrl.unlock();
            continue;
        }
        //fout << "sobad, " << templink << std::endl;
        //fout << "md5 = " << urldigest.ToString() << std::endl << std::endl;
        //fout.close();
        mutex_visitedUrl.unlock();

        //
        XUrl url(templink);
        string host=url.getHost();
        CMD5 hostdigest;
        hostdigest.GenerateMD5((unsigned char *)host.c_str(),host.length());
        mutex_unreachable_host.lock();
        if( unreachableHostMd5.find(hostdigest.getResult())
                !=unreachableHostMd5.end())
        {
            mutex_unreachable_host.unlock();
            continue;
        }
        mutex_unreachable_host.unlock();

        //
        /*if(url.getPath().empty())//means it's a website's root url
          {
        //root url should put into the first priority queue
        mutex_unvisitedUrl[0].lock();
        unvisitedUrl[0].push(templink);
        mutex_unvisitedUrl[0].unlock();
        }else*/
        {
            //the other url should put into the urldeep+1 priority queue
            mutex_unvisitedUrl[urldeep+1].lock();
            unvisitedUrl[urldeep+1].push(templink);
            mutex_unvisitedUrl[urldeep+1].unlock();
        }

        /*
           mutex_unvisitedUrl[1].lock();
           unvisitedUrl[1].push(templink);
           mutex_unvisitedUrl[1].unlock();*/

    }//end for
}
