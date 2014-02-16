/*
 * string_cache.h
 *
 *  Created on: 2013年8月10日
 *      Author: seven
 */

#ifndef STRING_CACHE_H_
#define STRING_CACHE_H_
#include "ares/cache_common.h"
#include <boost/shared_ptr.hpp>

namespace ares {
class Slice;
class StringCacheVisitor;

class StringCache {
public:
	StringCache();
	virtual ~StringCache();

	class Visitor {
	public:
		Visitor(){};
		virtual ~Visitor(){};
		virtual void visit(const Slice & key, const Slice & value) = 0;

	};

	virtual void Insert(const Slice & key, const Slice & value, uint32_t timeout = 0) = 0;
	virtual void Get(const Slice & key, std::string & value) = 0;
	virtual uint32_t Get(const Slice & key, char * value, uint32_t maxsize) = 0;
	virtual bool Check(const Slice & key) = 0;
	virtual bool Remove(const Slice & key) = 0;
	virtual void Clear() = 0;
	virtual void Visit(Visitor & visitor) = 0;
	virtual void Status(std::string & status) = 0;

};

extern boost::shared_ptr<StringCache> newShardedLRUStringCache(uint32_t cap_count,
		uint64_t cap_size, CacheShardedNum shard_num = CacheSharded_8);

} /* namespace ares */
#endif /* STRING_CACHE_H_ */
