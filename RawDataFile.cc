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
 * @file RawDataFile.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 * 
 * @modified on 3/6/2014, by zjs
 */
#include <iostream>
#include <fstream>
#include <string>
#include "RawDataFile.h"
#include "ThreadMutex.h"

/**
 * write the web page into file
 */
ThreadMutex mutex_rawfile;
const int COUNT_THREAD = 100;

RawDataFile::RawDataFile()
{
}

RawDataFile::RawDataFile(string path)
{
    out.open(path.c_str(),ios::out|ios::binary);
	if(!out)
	{
		cerr<<"can not open file "<<path<<endl;
		exit(-1);
	}
}

RawDataFile::~RawDataFile()
{
	out.close();
}

void RawDataFile::write( XPage &page)
{
	//cout<<page.getBody()<<endl;
	mutex_rawfile.lock();
	out<<"version: 1.0"<<"\n";
	out<<"url: "<<page.getUrl()<<"\n";
	out<<"ip: "<<page.getTitle()<<"\n";

	out<<"\n";

	out<<page.getHeader();
	out<<page.getBody();

	out<<"\n";
	
	mutex_rawfile.unlock();
}

void RawDataFile::doSearch(XPage &page)
{
    //std::cout << "in the doSearch!!!" << std::endl;
    mutex_rawfile.lock();
    string body = page.getBody();
    //std::cout << "gethead = \n" << page.getHeader() << std::endl; 
    //ofstream fout("./body");
    //fout << "getbody = \n" << body << std::endl;
    //fout.close();
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
    mutex_rawfile.unlock();
    //std::cout << "out the doSearch!!!" << std::endl;
}
