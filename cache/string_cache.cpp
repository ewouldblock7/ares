/*
 * string_cache.cpp
 *
 *  Created on: 2013年8月10日
 *      Author: seven
 */

#include "ares/string_cache.h"
#include <stdlib.h>
#include <assert.h>
#include <string>

#include "ares/mutex.h"
#include "ares/slice.h"
#include "cache/cache_table.h"
#include "util/hash.h"
#include "util/log.h"

namespace ares {

StringCache::StringCache() {
	// TODO Auto-generated constructor stub

}

StringCache::~StringCache() {
	// TODO Auto-generated destructor stub
}

struct LRUStringCacheHandle{
	LRUStringCacheHandle * lru_prev_;
	LRUStringCacheHandle * lru_next_;
	LRUStringCacheHandle * next_;
	uint32_t hash_;
	uint32_t key_size_;
	uint32_t value_size_;
	char buf_[0];

	Slice key() {
		return Slice(buf_, key_size_);
	}
};


class LRUStringCache{
public:
	LRUStringCache(uint32_t cap_count, uint64_t cap_size) :
		cap_count_(cap_count),
		cap_size_(cap_size),
		count_(0),
		size_(0){
		memset(&head_, 0, sizeof(head_));
		head_.lru_prev_ = &head_;
		head_.lru_next_ = &head_;
	}

	~LRUStringCache() {
		ClearWithoutLock();
	}

	void Insert(const Slice & key, const Slice & value, uint32_t hash){
		uint32_t len = sizeof(LRUStringCacheHandle) + key.size() + value.size();
		LRUStringCacheHandle * handle = reinterpret_cast<LRUStringCacheHandle *>(malloc(len));
		memset(handle, 0, len);
		handle->hash_ = hash;
		handle->key_size_ = key.size();
		handle->value_size_ = value.size();
		memcpy(handle->buf_, key.data(), key.size());
		memcpy(handle->buf_ + key.size(), value.data(), value.size());

		MutexGuard guard(&mutex_);
		LRUStringCacheHandle * old = cache_table_.Insert(key, handle);
		if(!old){
			LRUAppend(handle);
			++count_;
			size_ += len;
			while((size_ > cap_size_ || count_ > cap_count_) && head_.lru_prev_ != &head_){
				LRUStringCacheHandle * tmp = head_.lru_prev_;
				cache_table_.Remove(tmp->key(), tmp->hash_);
				LRURemove(tmp);
				deRef(tmp);
			}
		}else {
			LRURemove(old);
			LRUAppend(handle);
			deRef(old);
		}
	}

	uint32_t Get(const Slice & key, uint32_t hash, std::string & value){
		MutexGuard guard(&mutex_);
		LRUStringCacheHandle * ptr = cache_table_.Get(key, hash);
		if(ptr){
			LRURemove(ptr);
			LRUAppend(ptr);
			value.assign(ptr->buf_ + key.size(), ptr->value_size_);
		}
		return value.size();
	}

	bool Check(const Slice & key, uint32_t hash){
		MutexGuard guard(&mutex_);
		LRUStringCacheHandle * ptr = cache_table_.Get(key, hash);
		if(ptr){
			LRURemove(ptr);
			LRUAppend(ptr);
		}
		return ptr != NULL;
	}

	bool Remove(const Slice & key, uint32_t hash){
		MutexGuard guard(&mutex_);
		LRUStringCacheHandle * ptr = cache_table_.Remove(key, hash);
		if (ptr) {
			LRURemove (ptr);
			deRef(ptr);
			return true;
		}
		return false;
	}

	void Clear() {
		MutexGuard guard(&mutex_);
		ClearWithoutLock();
	}

	void Visit(StringCache::Visitor & visitor) {
		MutexGuard guard(&mutex_);
		LRUStringCacheHandle * ptr = head_.lru_next_;
		while(ptr != &head_){
			LRUStringCacheHandle * tmp = ptr->lru_next_;
			const Slice key = ptr->key();
			const Slice value(ptr->buf_ + key.size(), ptr->value_size_);
			visitor.visit(key, value);
			ptr = tmp;
		}
	}

private:
	void deRef(LRUStringCacheHandle * handle){
		--count_;
		size_ -= sizeof(LRUStringCacheHandle) + handle->key_size_ + handle->value_size_;
		free(handle);
	}

	void LRUAppend(LRUStringCacheHandle * handle){
		head_.lru_next_->lru_prev_ = handle;
		handle->lru_next_ = head_.lru_next_;
		head_.lru_next_ = handle;
		handle->lru_prev_ = &head_;
	}

	void LRURemove(LRUStringCacheHandle * handle){
		handle->lru_prev_->lru_next_ = handle->lru_next_;
		handle->lru_next_->lru_prev_ = handle->lru_prev_;
	}

	void ClearWithoutLock(){
		LRUStringCacheHandle * ptr = head_.lru_next_;
		while(ptr != &head_){
			LRUStringCacheHandle * tmp = ptr->lru_next_;
			deRef(ptr);
			ptr = tmp;
		}
	}

	Mutex mutex_;
	LRUStringCacheHandle head_;
	CacheTable<LRUStringCacheHandle> cache_table_;
	uint32_t cap_count_;
	uint64_t cap_size_;
	uint32_t count_;
	uint32_t size_;
};

class ShardedLRUStringCache : public StringCache{
public:
	ShardedLRUStringCache(uint32_t cap_count, uint64_t cap_size, CacheShardedNum shard_num) :
		cap_size_(cap_size),
		cap_count_(cap_count),
		slot_bits_(cacheShardedNumBits(shard_num)),
		slot_size_(shard_num),
		lru_string_caches_(NULL){
		lru_string_caches_ = new LRUStringCache * [slot_size_];
		for(uint32_t i = 0; i < slot_size_; ++i){
			lru_string_caches_[i] = new LRUStringCache(cap_count/slot_size_, cap_size/slot_size_);
		}
	}

	~ShardedLRUStringCache(){
		for(uint32_t i = 0; i < slot_size_; ++i){
			delete lru_string_caches_[i];
		}
		delete [] lru_string_caches_;
	}

	virtual void Insert(const Slice & key, const Slice & value) {
		uint32_t hash = Hash(key.data(), key.size());
		ARES_DEBUG("insert to lru slot: %u", hash>>(32 - slot_bits_));
		lru_string_caches_[hash>>(32 - slot_bits_)]->Insert(key, value, hash);
	}

	virtual void Get(const Slice & key, std::string & value) {
		uint32_t hash = Hash(key.data(), key.size());
		lru_string_caches_[hash>>(32 - slot_bits_)]->Get(key, hash, value);
	}

	virtual bool Check(const Slice & key) {
		uint32_t hash = Hash(key.data(), key.size());
		return lru_string_caches_[hash>>(32 - slot_bits_)]->Check(key, hash);
	}
	virtual bool Remove(const Slice & key) {
		uint32_t hash = Hash(key.data(), key.size());
		return lru_string_caches_[hash>>(32 - slot_bits_)]->Remove(key, hash);
	}

	virtual void Clear() {
		for(uint32_t i = 0; i < slot_size_; ++i){
			lru_string_caches_[i]->Clear();
		}
	}
	virtual void Visit(Visitor & visitor) {
		for(uint32_t i = 0; i < slot_size_; ++i){
			lru_string_caches_[i]->Visit(visitor);
		}
	}
private:
	uint64_t cap_size_;
	uint32_t cap_count_;
	uint32_t slot_bits_;
	uint32_t slot_size_;
	LRUStringCache ** lru_string_caches_;
};

extern boost::shared_ptr<StringCache> newShardedLRUStringCache(
		uint32_t cap_count, uint64_t cap_size, CacheShardedNum shard_num){
	return boost::shared_ptr<StringCache>(new ShardedLRUStringCache(cap_count, cap_size, shard_num));
}

} /* namespace ares */
