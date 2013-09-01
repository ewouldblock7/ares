/*
 * runnable.h

 *
 *  Created on: 2013-8-4
 *      Author: seven
 */

#ifndef ARES_RUNNABLE_H_
#define ARES_RUNNABLE_H_
#include <stdlib.h>
#include <pthread.h>
#include "util/log.h"

namespace ares {

class Runnable {
public:
	Runnable() : th_(0), running_(false){};
	virtual ~Runnable(){};

	virtual void run() = 0;

	void start();

	static void * thread_work(void * arg) {
		Runnable * runnable = static_cast<Runnable *>(arg);
		runnable->run();
		return NULL;
	}

	void stop() {running_ = false;}

	void join();


protected:
	pthread_t th_;
	bool running_;

private:
	Runnable(const Runnable &);
	Runnable & operator=(const Runnable &);
};

} /* namespace ares */
#endif /* ARES_RUNNABLE_H_ */
