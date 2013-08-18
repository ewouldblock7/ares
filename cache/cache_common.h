/*
 * cache_common.h
 *
 *  Created on: 2013年8月17日
 *      Author: seven
 */

#ifndef CACHE_COMMON_H_
#define CACHE_COMMON_H_
#include <stdint.h>

enum CacheShardedNum{
	CacheSharded_4 = 4,
	CacheSharded_8 = 8,
	CacheSharded_16 = 16,
	CacheSharded_32 = 32,
	CacheSharded_64 = 64,
	CacheSharded_128 = 128,
	CacheSharded_256 = 256,
	CacheSharded_512 = 512
};

extern inline uint32_t cacheShardedNumBits(CacheShardedNum num){
	switch(num){
	case CacheSharded_4:
		return 2u;
	case CacheSharded_8:
		return 3u;
	case CacheSharded_16:
		return 4u;
	case CacheSharded_32:
		return 5u;
	case CacheSharded_64:
		return 6u;
	case CacheSharded_128:
		return 7u;
	case CacheSharded_256:
		return 8u;
	case CacheSharded_512:
		return 9u;
	}

}


#endif /* CACHE_COMMON_H_ */
