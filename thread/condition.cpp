/*
 * Condition.cpp
 *
 *  Created on: 2013年8月31日
 *      Author: seven
 */

#include "ares/condition.h"

namespace ares {

Condition::Condition(Mutex * mutex) :
        mutex_(mutex) {
    // TODO Auto-generated constructor stub
    pthread_cond_init(&cond_, NULL);
}

Condition::~Condition() {
    // TODO Auto-generated destructor stub
    pthread_cond_destroy(&cond_);
}

void Condition::Wait() {
    pthread_cond_wait(&cond_, mutex_->getMutex());
}
void Condition::Signal() {
    pthread_cond_signal(&cond_);
}
void Condition::BroadCast() {
    pthread_cond_broadcast(&cond_);
}

} /* namespace ares */
