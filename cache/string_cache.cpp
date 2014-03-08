/*
 * string_cache.cpp
 *
 *  Created on: 2013年8月10日
 *      Author: seven
 */

#include "ares/string_cache.h"
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
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

struct LRUStringCacheHandle {
    LRUStringCacheHandle * lru_prev_;
    LRUStringCacheHandle * lru_next_;
    LRUStringCacheHandle * next_;
    uint32_t hash_;
    uint32_t key_size_;
    uint32_t value_size_;
    uint32_t time_out_;
    char buf_[0];

    Slice key() {
        return Slice(buf_, key_size_);
    }
};

class LRUStringCache {
public:
    LRUStringCache(uint32_t cap_count, uint64_t cap_size) :
            cap_size_(cap_size), size_(0), cap_count_(cap_count), count_(0), get_count_(
                    0), hit_count_(0) {

        memset(&head_, 0, sizeof(head_));
        head_.lru_prev_ = &head_;
        head_.lru_next_ = &head_;

    }

    ~LRUStringCache() {
        ClearWithoutLock();
    }

    void Insert(const Slice & key, const Slice & value, uint32_t hash,
            uint32_t timeout = 0);

    uint32_t Get(const Slice & key, uint32_t hash, std::string & value);

    uint32_t Get(const Slice & key, uint32_t hash, char * value,
            uint32_t maxsize);

    bool Check(const Slice & key, uint32_t hash);

    bool Remove(const Slice & key, uint32_t hash);

    void Clear();

    void Visit(StringCache::Visitor & visitor);

    struct Stat {
        Stat() :
                size_(0), count_(0), hit_count_(0), get_count_(0) {
        }

        uint64_t size_;
        uint32_t count_;
        uint32_t hit_count_;
        uint32_t get_count_;
    };

    Stat GetStatus() {
        Stat stat;
        stat.size_ = size_;
        stat.count_ = count_;
        stat.get_count_ = get_count_;
        stat.hit_count_ = hit_count_;
        return stat;
    }

private:
    void deRef(LRUStringCacheHandle * handle) {
        assert(count_ > 0);
        assert(size_ > 0);
        --count_;
        size_ -= sizeof(LRUStringCacheHandle) + handle->key_size_
                + handle->value_size_;
        free(handle);
    }

    void LRUAppend(LRUStringCacheHandle * handle) {
        head_.lru_next_->lru_prev_ = handle;
        handle->lru_next_ = head_.lru_next_;
        head_.lru_next_ = handle;
        handle->lru_prev_ = &head_;
    }

    void LRURemove(LRUStringCacheHandle * handle) {
        handle->lru_prev_->lru_next_ = handle->lru_next_;
        handle->lru_next_->lru_prev_ = handle->lru_prev_;
    }

    void Eliminate(uint32_t now_tick);

    uint32_t CopyValue(LRUStringCacheHandle * ptr, char * value,
            uint32_t maxsize) {
        if (ptr->value_size_ + 1 <= maxsize) {
            memcpy(value, ptr->buf_ + ptr->key_size_, ptr->value_size_);
            *(value + ptr->value_size_) = '\0';
            return ptr->value_size_;
        }
        return 0;
    }

    void ClearWithoutLock() {
        LRUStringCacheHandle * ptr = head_.lru_next_;
        while (ptr != &head_) {
            LRUStringCacheHandle * tmp = ptr->lru_next_;
            deRef(ptr);
            ptr = tmp;
        }
    }

    Mutex mutex_;
    LRUStringCacheHandle head_;
    CacheTable<LRUStringCacheHandle> cache_table_;
    uint64_t cap_size_;
    uint64_t size_;
    uint32_t cap_count_;
    uint32_t count_;
    uint32_t get_count_;
    uint32_t hit_count_;
};


void LRUStringCache::Insert(const Slice & key, const Slice & value, uint32_t hash,
        uint32_t timeout) {
    uint32_t len = sizeof(LRUStringCacheHandle) + key.size() + value.size();
    LRUStringCacheHandle * handle =
            reinterpret_cast<LRUStringCacheHandle *>(malloc(len));
    memset(handle, 0, len);
    handle->hash_ = hash;
    handle->key_size_ = key.size();
    handle->value_size_ = value.size();
    struct timeval now_timeval;
    gettimeofday(&now_timeval, NULL);
    if (timeout && timeout < now_timeval.tv_sec) {
        timeout = now_timeval.tv_sec + timeout;
    } else if (timeout == 0) {
        timeout = UINT_MAX;
    }
    handle->time_out_ = timeout;
    memcpy(handle->buf_, key.data(), key.size());
    memcpy(handle->buf_ + key.size(), value.data(), value.size());

    MutexGuard guard(&mutex_);
    LRUStringCacheHandle * old = cache_table_.Insert(key, handle);
    ++count_;
    size_ += len;
    if (!old) {
        LRUAppend(handle);
        while ((size_ > cap_size_ || count_ > cap_count_)
                && head_.lru_prev_ != &head_) {
            Eliminate(now_timeval.tv_sec);
        }
    } else {
        LRURemove(old);
        LRUAppend(handle);
        deRef(old);
    }
}

uint32_t LRUStringCache::Get(const Slice & key, uint32_t hash, std::string & value) {
    MutexGuard guard(&mutex_);
    ++get_count_;
    LRUStringCacheHandle * ptr = cache_table_.Get(key, hash);
    if (ptr) {
        if (ptr->time_out_) {
            struct timeval now_timeval;
            gettimeofday(&now_timeval, NULL);
            if (now_timeval.tv_sec <= ptr->time_out_) {
                LRURemove(ptr);
                LRUAppend(ptr);
                ++hit_count_;
                value.assign(ptr->buf_ + key.size(), ptr->value_size_);
            } else {
                cache_table_.Remove(key, hash);
                LRURemove(ptr);
                deRef(ptr);
            }
        } else {
            LRURemove(ptr);
            LRUAppend(ptr);
            ++hit_count_;
            value.assign(ptr->buf_ + key.size(), ptr->value_size_);
        }
    }
    return value.size();
}

uint32_t LRUStringCache::Get(const Slice & key, uint32_t hash, char * value,
        uint32_t maxsize) {
    MutexGuard guard(&mutex_);
    ++get_count_;
    LRUStringCacheHandle * ptr = cache_table_.Get(key, hash);
    int ret = 0;
    if (ptr) {
        if (ptr->time_out_) {
            struct timeval now_timeval;
            gettimeofday(&now_timeval, NULL);
            if (now_timeval.tv_sec <= ptr->time_out_) {
                LRURemove(ptr);
                LRUAppend(ptr);
                ++hit_count_;
                ret = CopyValue(ptr, value, maxsize);
            } else {
                cache_table_.Remove(key, hash);
                LRURemove(ptr);
                deRef(ptr);
            }
        } else {
            LRURemove(ptr);
            LRUAppend(ptr);
            ++hit_count_;
            ret = CopyValue(ptr, value, maxsize);
        }
    }
    return ret;
}

bool LRUStringCache::Check(const Slice & key, uint32_t hash) {
    MutexGuard guard(&mutex_);
    LRUStringCacheHandle * ptr = cache_table_.Get(key, hash);
    if (ptr) {
        if (ptr->time_out_) {
            struct timeval now_timeval;
            gettimeofday(&now_timeval, NULL);

            if (now_timeval.tv_sec <= ptr->time_out_) {
                LRURemove(ptr);
                LRUAppend(ptr);
            } else {
                cache_table_.Remove(key, hash);
                LRURemove(ptr);
                deRef(ptr);
            }
        } else {
            LRURemove(ptr);
            LRUAppend(ptr);
        }
    }
    return ptr != NULL;
}

bool LRUStringCache::Remove(const Slice & key, uint32_t hash) {
    MutexGuard guard(&mutex_);
    LRUStringCacheHandle * ptr = cache_table_.Remove(key, hash);
    if (ptr) {
        LRURemove(ptr);
        deRef(ptr);
        return true;
    }
    return false;
}

void LRUStringCache::Clear() {
    MutexGuard guard(&mutex_);
    ClearWithoutLock();
}

void LRUStringCache::Visit(StringCache::Visitor & visitor) {
    MutexGuard guard(&mutex_);
    LRUStringCacheHandle * ptr = head_.lru_next_;
    while (ptr != &head_) {
        LRUStringCacheHandle * tmp = ptr->lru_next_;
        const Slice key = ptr->key();
        const Slice value(ptr->buf_ + key.size(), ptr->value_size_);
        visitor.visit(key, value);
        ptr = tmp;
    }
}

void LRUStringCache::Eliminate(uint32_t now_tick) {

    uint32_t times = 0;
    std::vector<LRUStringCacheHandle *> recors;
    for (LRUStringCacheHandle * ptr = head_.lru_prev_;
            times < 20 && ptr != &head_; ptr = ptr->lru_prev_, ++times) {
        if (ptr->time_out_) {
            if (now_tick > ptr->time_out_) {
                recors.push_back(ptr);
            }
        }
    };

    LRUStringCacheHandle * ptr = head_.lru_prev_;
    if (recors.empty() && ptr != &head_) {
        recors.push_back(ptr);
    }

    for (std::vector<LRUStringCacheHandle *>::iterator ite = recors.begin();
            ite != recors.end(); ++ite) {
        cache_table_.Remove((*ite)->key(), (*ite)->hash_);
        LRURemove(*ite);
        deRef(*ite);
    }

}

class ShardedLRUStringCache: public StringCache {
public:
    ShardedLRUStringCache(uint32_t cap_count, uint64_t cap_size,
            CacheShardedNum shard_num) :
            cap_size_(cap_size), cap_count_(cap_count), slot_bits_(
                    cacheShardedNumBits(shard_num)), slot_size_(shard_num), lru_string_caches_(
                    NULL) {
        lru_string_caches_ = new LRUStringCache *[slot_size_];
        for (uint32_t i = 0; i < slot_size_; ++i) {
            lru_string_caches_[i] = new LRUStringCache(cap_count / slot_size_,
                    cap_size / slot_size_);
        }
    }

    ~ShardedLRUStringCache() {
        for (uint32_t i = 0; i < slot_size_; ++i) {
            delete lru_string_caches_[i];
        }
        delete[] lru_string_caches_;
    }

    virtual void Insert(const Slice & key, const Slice & value,
            uint32_t timeout) {
        uint32_t hash = Hash(key.data(), key.size());
        lru_string_caches_[hash >> (32 - slot_bits_)]->Insert(key, value, hash,
                timeout);
    }

    virtual void Get(const Slice & key, std::string & value) {
        uint32_t hash = Hash(key.data(), key.size());
        lru_string_caches_[hash >> (32 - slot_bits_)]->Get(key, hash, value);
    }

    virtual uint32_t Get(const Slice & key, char * value, uint32_t maxsize) {
        uint32_t hash = Hash(key.data(), key.size());
        return lru_string_caches_[hash >> (32 - slot_bits_)]->Get(key, hash,
                value, maxsize);
    }

    virtual bool Check(const Slice & key) {
        uint32_t hash = Hash(key.data(), key.size());
        return lru_string_caches_[hash >> (32 - slot_bits_)]->Check(key, hash);
    }
    virtual bool Remove(const Slice & key) {
        uint32_t hash = Hash(key.data(), key.size());
        return lru_string_caches_[hash >> (32 - slot_bits_)]->Remove(key, hash);
    }

    virtual void Clear() {
        for (uint32_t i = 0; i < slot_size_; ++i) {
            lru_string_caches_[i]->Clear();
        }
    }
    virtual void Visit(Visitor & visitor) {
        for (uint32_t i = 0; i < slot_size_; ++i) {
            lru_string_caches_[i]->Visit(visitor);
        }
    }

    virtual void Status(std::string & status);

private:
    uint64_t cap_size_;
    uint32_t cap_count_;
    uint32_t slot_bits_;
    uint32_t slot_size_;
    LRUStringCache ** lru_string_caches_;
};

void ShardedLRUStringCache::Status(std::string & status) {
    LRUStringCache::Stat total_stat;
    for (uint32_t i = 0; i < slot_size_; ++i) {
        LRUStringCache::Stat stat = lru_string_caches_[i]->GetStatus();
        total_stat.count_ += stat.count_;
        total_stat.size_ += stat.size_;
        total_stat.hit_count_ += stat.hit_count_;
        total_stat.get_count_ += stat.get_count_;
    }

    std::stringstream ss;
    ss << "count:" << total_stat.count_ << std::endl;
    ss << "size:" << total_stat.size_ << std::endl;
    ss << "hit:" << total_stat.hit_count_ << std::endl;
    ss << "get:" << total_stat.get_count_ << std::endl;
    if (total_stat.get_count_)
        ss << "hitrate:"
                << ((double) total_stat.hit_count_) / total_stat.get_count_
                << std::endl;
    status = ss.str();
}

extern boost::shared_ptr<StringCache> newShardedLRUStringCache(
        uint32_t cap_count, uint64_t cap_size, CacheShardedNum shard_num) {
    return boost::shared_ptr<StringCache>(
            new ShardedLRUStringCache(cap_count, cap_size, shard_num));
}

} /* namespace ares */
