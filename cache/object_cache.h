/*
 * MemCache.h


 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#ifndef OBJECT_CACHE_H_
#define OBJECT_CACHE_H_
#include <stdint.h>
#include <boost/shared_ptr.hpp>

namespace ares {
class Slice;
class CacheVisitor;

class ObjectCache {
public:
	ObjectCache();
	virtual ~ObjectCache();

	virtual void Insert(const Slice & key, boost::shared_ptr<void> & value, uint32_t charge) = 0;
	virtual boost::shared_ptr<void> Get(const Slice & key) = 0;
	virtual bool Check(const Slice & key) = 0;
	virtual bool Remove(const Slice & key) = 0;
	virtual void Clear() = 0;
	virtual void Visit(CacheVisitor & visitor) = 0;
};

extern boost::shared_ptr<ObjectCache> newShardedLRUObjectCache(uint64_t charge_cap, uint32_t count_cap);

} /* namespace ares */
#endif /* OBJECT_CACHE_H_ */
