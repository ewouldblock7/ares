/*
 * util.h
 *
 *  Created on: 2013-12-16
 *      Author: ewouldblock
 */

#ifndef UTIL_H_
#define UTIL_H_
#include <sys/time.h>
#include <stdint.h>


namespace ares {

class Util {
public:
	Util(){};
	~Util(){};

	inline const timeval SubTv(const timeval & end, const timeval & start){
		timeval tv = end;
		return SubTvSelf(tv, start);
	}

	inline const uint32_t Tv2Ms(const timeval &tv){
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}

	inline const uint64_t Tv2Us(const timeval &tv){
		return tv.tv_sec * 1000000UL + tv.tv_usec;
	}

private:

	inline timeval & SubTvSelf(timeval & end, const timeval & start){
		end.tv_sec -= start.tv_sec;
		end.tv_usec -= start.tv_usec;
		if(end.tv_usec < 0){
			--end.tv_sec;
			end.tv_usec += 1000000;
		}
		return end;
	}
};

} /* namespace ares */
#endif /* UTIL_H_ */
