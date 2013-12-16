/*
 * tick_count.h
 *
 *  Created on: 2013-12-16
 *      Author: ewouldblock
 */

#ifndef TICK_COUNT_H_
#define TICK_COUNT_H_
#include <stdlib.h>
#include <sys/time.h>
#include "util/util.h"
namespace ares {

class TickCount {
public:
	TickCount() : consume_us_(0){};
	~TickCount(){};

	void Begin() {gettimeofday(&begin_, NULL);}

	void End() {
		gettimeofday(&end_, NULL);
		Util util;
		timeval consume = util.SubTv(end_, begin_);
		consume_us_ = util.Tv2Us(consume);
	}

	uint64_t GetConsumeUs() {return consume_us_;}

private:
	timeval begin_;
	timeval end_;
	uint64_t consume_us_;
};

} /* namespace ares */
#endif /* TICK_COUNT_H_ */
