#include "ares/mutex.h"
#include "util/log.h"

namespace ares{

Mutex::Mutex(){
	if(pthread_mutex_init(&mutex_, NULL)){
		ARES_FATAL("pthread_mutex_init error")
	}
}

Mutex::~Mutex(){
	if(pthread_mutex_destroy(&mutex_)){
		ARES_FATAL("pthread_mutex_destroy error")
	}
}

void Mutex::Lock(){
	if(pthread_mutex_lock(&mutex_)){
		ARES_FATAL("pthread_mutex_lock error")
	}
}

void Mutex::Unlock(){
	if(pthread_mutex_unlock(&mutex_)){
		ARES_FATAL("pthread_mutex_unlock error")
	}
}

}
