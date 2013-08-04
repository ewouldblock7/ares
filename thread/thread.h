/*
 * Thread.h
 *
 *  Created on: 2013-8-4
 *      Author: seven
 */

#ifndef THREAD_H_
#define THREAD_H_
#include <stdlib.h>
#include <pthread.h>
#include "thread/runnable.h"
#include "util/log.h"

namespace ares {

class Thread {
public:
	Thread(Runnable * runnable) : runnable_(runnable), th_(0){

	}
	virtual ~Thread(){};

	void start(){
		if(pthread_create(&th_, NULL, thread_work, runnable_)){
			ARES_WARNING("start thread fail");
		}
	}

	static void * thread_work(void * arg) {
		Runnable * runnable = static_cast<Runnable *>(arg);
		runnable->run();
		return NULL;
	}

	void join(){
		if(pthread_join(th_, NULL)){
			ARES_WARNING("join thread fail");
		}
	}

private:
	Runnable * runnable_;
	pthread_t th_;
};

} /* namespace ares */
#endif /* THREAD_H_ */
