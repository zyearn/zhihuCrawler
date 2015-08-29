#ifndef _MUTEXLOCK_H
#define _MUTEXLOCK_H

#include <pthread.h>

class MutexLock {
public:
    MutexLock() {
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock() {
        pthread_mutex_destroy(&mutex_);
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *getPthreadMutex() {
        return &mutex_;
    }

private:
    
    pthread_mutex_t mutex_;

};

#endif
