/*
 * runnable.cpp
 *
 *  Created on: 2013年8月31日
 *      Author: seven
 */
#include "ares/runnable.h"

namespace ares {

void Runnable::start() {
    if (pthread_create(&th_, NULL, thread_work, this)) {
        ARES_WARNING("start thread fail");
    }
}

void Runnable::join() {
    if (pthread_join(th_, NULL)) {
        ARES_WARNING("join thread fail");
    }
}

}
