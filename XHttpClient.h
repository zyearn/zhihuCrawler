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
 * @file XHttpClient.h
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#ifndef _XHTTPCLIENT_H_
#define _XHTTPCLIENT_H_
#include "XPage.h"
#include <string>

using namespace std;

class XHttpClient
{
	public:
		XHttpClient();
		~XHttpClient();
		

		int download(const string &url,XPage &page);
	private:
		int createConnection(const string &url, XPage &page);
};
#endif
