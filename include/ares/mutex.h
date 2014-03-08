/*
 * Mutex.h

 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#ifndef ARES_MUTEX_H_
#define ARES_MUTEX_H_
#include <pthread.h>

namespace ares {

class Mutex {
public:
    Mutex();
    ~Mutex();

    void Lock();

    void Unlock();

    pthread_mutex_t * getMutex() {
        return &mutex_;
    }

private:
    Mutex(const Mutex &);
    Mutex & operator=(const Mutex &);
    pthread_mutex_t mutex_;
};

class MutexGuard {
public:
    MutexGuard(Mutex * mutex) :
            mutex_(mutex) {
        mutex_->Lock();
    }

    ~MutexGuard() {
        mutex_->Unlock();
    }

private:
    MutexGuard(const MutexGuard &);
    MutexGuard & operator=(const MutexGuard &);
    Mutex * mutex_;
};

} /* namespace ares */
#endif /* ARES_MUTEX_H_ */
