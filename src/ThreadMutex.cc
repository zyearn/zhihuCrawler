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
 * @file ThreadMutex.cc
 * @version 0.0.0
 * @author zouxin ( zouxin2008@gmail.com )
 * @date 11/3/2008 0.0.0 created, by zouxin
 */
#include "ThreadMutex.h"

/**
 * mutex class
 */
const pthread_mutex_t MUTEX_INIT=PTHREAD_MUTEX_INITIALIZER;
ThreadMutex::ThreadMutex()
{
	m_mutex=MUTEX_INIT;
}

ThreadMutex::~ThreadMutex()
{
	pthread_mutex_destroy(&m_mutex);
}

int ThreadMutex::lock()
{
	return pthread_mutex_lock(&m_mutex);
}

int ThreadMutex::unlock()
{
	return pthread_mutex_unlock(&m_mutex);
}

int ThreadMutex::trylock()
{
	return pthread_mutex_trylock(&m_mutex);
}
