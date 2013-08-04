/*
 * Mutex.h

 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#ifndef MUTEX_H_
#define MUTEX_H_
#include <pthread.h>

namespace ares {

class Mutex {
public:
	Mutex(){
		pthread_mutex_init(&mutex_, NULL);
	}
	~Mutex(){
		pthread_mutex_destroy(&mutex_);
	}

	void Lock(){
		pthread_mutex_lock(&mutex_);
	}

	void Unlock(){
		pthread_mutex_unlock(&mutex_);
	}

private:
	Mutex(const Mutex &);
	Mutex & operator=(const Mutex &);
	pthread_mutex_t mutex_;
};


class MutexGuard{
public:
	MutexGuard(Mutex * mutex) : mutex_(mutex){
		mutex_->Lock();
	}

	~MutexGuard(){
		mutex_->Unlock();
	}

private:
	MutexGuard(const MutexGuard &);
	MutexGuard & operator=(const MutexGuard &);
	Mutex * mutex_;
};

} /* namespace ares */
#endif /* MUTEX_H_ */
