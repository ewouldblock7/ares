/*
 * Condition.h
 *
 *  Created on: 2013年8月31日
 *      Author: seven
 */

#ifndef ARES_CONDITION_H_
#define ARES_CONDITION_H_
#include "ares/mutex.h"

namespace ares {

class Condition {
public:
    Condition(Mutex * mutex);
    ~Condition();

    void Wait();
    void Signal();
    void BroadCast();

private:
    pthread_cond_t cond_;
    Mutex * mutex_;
};

} /* namespace ares */
#endif /* ARES_CONDITION_H_ */
