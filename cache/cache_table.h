/*
 * cache_table.h
 *
 *  Created on: 2013年8月11日
 *      Author: seven
 */

#ifndef CACHE_TABLE_H_
#define CACHE_TABLE_H_
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

namespace ares {

template<class T>
class CacheTable{
public:
	CacheTable() :
		slots_(NULL),
		slot_size_(0),
		ele_num_(0){
		Resize();
	}

	~CacheTable(){
		delete [] slots_;
	}

	T * Insert(const Slice & key, T * handle){
		T ** ptr = FindPtr(key, handle->hash_);
		T * old = *ptr;
		handle->next_ = (old == NULL ? NULL : old->next_);
		*ptr = handle;

		if(old == NULL){
			++ele_num_;
			if(ele_num_ > slot_size_){
				Resize();
			}
		}
		return old;
	}

	T * Get(const Slice & key, uint32_t hash){
		T ** ptr = FindPtr(key, hash);
		return *ptr;
	}

	T* Remove(const Slice & key, uint32_t hash){
		T ** ptr = FindPtr(key, hash);
		T * result = *ptr;
		if(result) {
			--ele_num_;
			*ptr = result->next_;
		}
		return result;
	}


private:

	T ** FindPtr(const Slice & key, uint32_t hash){
		T ** ptr = &slots_[hash & (slot_size_ - 1)];
		while(*ptr != NULL &&
				((*ptr)->hash_ != hash || (*ptr)->key() != key)){
			ptr = &((*ptr)->next_);
		}
		return ptr;
	}

	void Resize(){
		uint32_t base = 8;
		while(base <= ele_num_){
			base <<= 1;
		}

		T ** newSlot = new T*[base];
		memset(newSlot, 0 , sizeof(newSlot[0]) * base);
		uint32_t count = 0;
		for(uint32_t i = 0; i < slot_size_; ++i){
			T * ptr = slots_[i];
			while(ptr){
				T * tmp = ptr->next_;
				T ** newPtr = &newSlot[ptr->hash_ & (base - 1)];
				ptr->next_ = *newPtr;
				*newPtr = ptr;
				ptr = tmp;
				++count;
			}
		}

		delete [] slots_;
		assert(count == ele_num_);
		slot_size_ = base;
		slots_ = newSlot;
	}


	T ** slots_;
	uint32_t slot_size_;
	uint32_t ele_num_;
};

} /* namespace ares */
#endif /* CACHE_TABLE_H_ */
