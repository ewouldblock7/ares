/*
 * runnable.h

 *
 *  Created on: 2013-8-4
 *      Author: seven
 */

#ifndef RUNNABLE_H_
#define RUNNABLE_H_
#include <stdlib.h>
#include <pthread.h>
#include "util/log.h"

namespace ares {

class Runnable {
public:
	Runnable() : th_(0){};
	virtual ~Runnable(){};

	virtual void run() = 0;

	void start(){
		if(pthread_create(&th_, NULL, thread_work, this)){
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
	Runnable(const Runnable &);
	Runnable & operator=(const Runnable &);
	pthread_t th_;
};

} /* namespace ares */
#endif /* RUNNABLE_H_ */
