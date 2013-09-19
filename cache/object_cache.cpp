/*
 * MemCache.cpp

 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#include "ares/object_cache.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ares/mutex.h"
#include "ares/slice.h"
#include "cache/cache_table.h"
#include "util/hash.h"
#include "util/log.h"

namespace ares {

ObjectCache::ObjectCache() {
	// TODO Auto-generated constructor stub

}

ObjectCache::~ObjectCache() {
	// TODO Auto-generated destructor stub
}


struct LRUObjectHandle {
	boost::shared_ptr<void> * value_;
	LRUObjectHandle * lru_prev_;
	LRUObjectHandle * lru_next_;
	LRUObjectHandle * next_;
	uint32_t charge_;
	uint32_t key_size_;
	uint32_t hash_;
	char key_[1];

	const Slice key() const {
		return Slice(key_, key_size_);
	}
};


class LRUObjectCache{
public:
	LRUObjectCache(uint64_t cap_charge, uint32_t cap_count, uint32_t idx = 0) :
		cap_charge_(cap_charge),
		cap_count_(cap_count),
		charge_(0),
		count_(0),
		idx_(idx){

		memset(&head_, 0, sizeof(head_));
		head_.lru_next_ = &head_;
		head_.lru_prev_ = &head_;
	}

	~LRUObjectCache(){
		DoClear();
	}

	void Insert(const Slice & key, boost::shared_ptr<void> & value, uint32_t charge, uint32_t hash){
		MutexGuard guard(&mutex_);
		uint32_t msize = sizeof(LRUObjectHandle) + key.size() - 1;
		LRUObjectHandle * handle = static_cast<LRUObjectHandle *>(malloc(msize));
		memset(handle, 0 , msize);
		handle->value_ = new boost::shared_ptr<void>(value);
		handle->charge_ = charge;
		handle->hash_ = hash;
		memcpy(handle->key_, key.data(), key.size());
		LRUObjectHandle * old = table_.Insert(key, handle);
		if(old) {
			LRURemove(old);
			deRef(old);
		}
		LRUAppend(handle);
		++count_;
		charge_ += charge;
		while((count_ > cap_count_ || charge_ > cap_charge_) && head_.lru_prev_ != &head_){
			LRUObjectHandle * oldest = head_.lru_prev_;
			assert(oldest != &head_);
			assert(oldest != NULL);
			table_.Remove(oldest->key(), oldest->hash_);
			LRURemove(oldest);
			deRef(oldest);
		}
	}

	boost::shared_ptr<void> Get(const Slice & key, uint32_t hash){
		MutexGuard guard(&mutex_);
		LRUObjectHandle * handle = table_.Get(key, hash);
		if(handle) {
			LRURemove(handle);
			LRUAppend(handle);
		}
		return *handle->value_;
	}

	bool Check(const Slice & key, uint32_t hash){
		MutexGuard guard(&mutex_);
		LRUObjectHandle * handle = table_.Get(key, hash);
		if(handle){
			LRURemove(handle);
			LRUAppend(handle);
			return true;
		}
		return false;
	}

	bool Remove(const Slice & key, uint32_t hash){
		MutexGuard guard(&mutex_);
		LRUObjectHandle * handle = table_.Get(key, hash);
		if(handle){
			LRURemove(handle);
			deRef(handle);
			return true;
		}
		return false;
	}

	void Clear(){
		MutexGuard guard(&mutex_);
		DoClear();
	}

	void Visit(ObjectCache::IterateVisitor & visitor){
		LRUObjectHandle * ptr = head_.lru_next_;
		while(ptr != &head_){
			visitor.Visit(ptr->key(), *ptr->value_);
			ptr = ptr->lru_next_;
		}
	}
private:
	void DoClear(){
		LRUObjectHandle * ptr = head_.lru_next_;
		while(ptr != &head_){
			LRUObjectHandle * tmp = ptr->lru_next_;
			deRef(ptr);
			ptr = tmp;
		}
	}

	void LRUAppend(LRUObjectHandle * handle){
		handle->lru_next_ = head_.lru_next_;
		handle->lru_prev_ = &head_;
		handle->lru_next_->lru_prev_ = handle;
		handle->lru_prev_->lru_next_ = handle;
	}
	void LRURemove(LRUObjectHandle * handle){
		handle->lru_prev_->lru_next_ = handle->lru_next_;
		handle->lru_next_->lru_prev_ = handle->lru_prev_;
	}
	void deRef(LRUObjectHandle * handle){
		--count_;
		charge_ -= handle->charge_;
		delete handle->value_;
		free(handle);
	}

	CacheTable<LRUObjectHandle> table_;
	LRUObjectHandle head_;
	Mutex mutex_;
	uint64_t cap_charge_;
	uint64_t charge_;
	uint32_t cap_count_;
	uint32_t count_;
	uint32_t idx_;
};

const static uint32_t LRU_SHARD_BITS = 4;
const static uint32_t LRU_SHARD_NUM = 1<<LRU_SHARD_BITS;


class LRUShardedObjectCache : public ObjectCache{
public:
	LRUShardedObjectCache(uint64_t capacity, uint32_t max_elenum) :
		capacity_(capacity),
		max_elenum_(max_elenum),
		shards_(NULL),
		shard_num_(LRU_SHARD_NUM),
		shard_num_bits_(LRU_SHARD_BITS){
		inital();
	}

	~LRUShardedObjectCache(){
		for(uint32_t i = 0; i < shard_num_; ++i){
			delete shards_[i];
		}
		delete [] shards_;
	}

	virtual void Insert(const Slice & key, boost::shared_ptr<void> & value, uint32_t charge){
		uint32_t hash = HashSlice(key);
		uint32_t shard = ShardHash(hash);
		shards_[shard]->Insert(key,value, charge, hash);
	}
	virtual boost::shared_ptr<void> Get(const Slice & key){
		uint32_t hash = HashSlice(key);
		uint32_t shard = ShardHash(hash);
		return shards_[shard]->Get(key, hash);
	}
	virtual bool Check(const Slice & key){
		uint32_t hash = HashSlice(key);
		uint32_t shard = ShardHash(hash);
		return shards_[shard]->Check(key, hash);
	}
	virtual bool Remove(const Slice & key){
		uint32_t hash = HashSlice(key);
		uint32_t shard = ShardHash(hash);
		return shards_[shard]->Remove(key, hash);
	}

	virtual void Clear(){
		for(uint32_t i = 0; i < shard_num_; ++i){
			shards_[i]->Clear();
		}
	}
	void Visit(IterateVisitor & visitor){
		for(uint32_t i = 0; i < shard_num_; ++i){
			shards_[i]->Visit(visitor);
		}
	}

private:

	void inital() {
		shards_ = new LRUObjectCache *[shard_num_];
		for(uint32_t i = 0; i < shard_num_; ++i){
			shards_[i] = new LRUObjectCache(capacity_ / shard_num_, max_elenum_ / shard_num_, i);
		}
	}

	uint32_t HashSlice(const Slice &key){
		return Hash(key.data(), key.size());
	}
	uint32_t ShardHash(uint32_t hash){
		return hash >> (32 - shard_num_bits_);
	}

	uint64_t capacity_;
	uint32_t max_elenum_;
	LRUObjectCache ** shards_;
	uint32_t shard_num_;
	uint32_t shard_num_bits_;

};

extern boost::shared_ptr<ObjectCache> newShardedLRUObjectCache(uint64_t charge_cap, uint32_t count_cap){
	return boost::shared_ptr<ObjectCache>(new LRUShardedObjectCache(charge_cap, count_cap));
}


} /* namespace ares */
