/*
 * cache_table.h
 *
 *  Created on: 2013年8月11日
 *      Author: seven
 */

#ifndef CACHE_TABLE_H_
#define CACHE_TABLE_H_
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "ares/slice.h"

namespace ares {

const static uint32_t REHASH_NUM_ONCE = 100;

template<class T>
class CacheTable{
public:
	CacheTable() :
		slots_(NULL),
		slots_2_(NULL),
		slot_size_2_(0),
		rehashing_idx_(0),
		slot_size_(0),
		ele_num_(0){
		Resize2();
	}

	~CacheTable(){
		delete [] slots_;
		delete [] slots_2_;
	}

	T * Insert(const Slice & key, T * handle){
		bool rehashing = isRehasing();
		T * old = NULL;
		if(!rehashing){//not in rehashing progress
			T ** ptr = FindPtr(slots_, slot_size_, key, handle->hash_);
			old = *ptr;
			handle->next_ = (old == NULL ? NULL : old->next_);
			*ptr = handle;

			if(old == NULL){
				++ele_num_;
				if(ele_num_ > slot_size_ && !rehashing){
					Resize2();
				}
			}
		}else{
			ReHashing();
			rehashing = isRehasing();
			T ** ptr = FindPtr(slots_, slot_size_, key, handle->hash_);
			if(rehashing && *ptr == NULL)
				ptr = FindPtr(slots_2_, slot_size_2_, key, handle->hash_);

			old = *ptr;
			handle->next_ = (old == NULL ? NULL : old->next_);
			*ptr = handle;

			if (old == NULL) {
				++ele_num_;
				++rehahing_count_;
			}
		}
		return old;
	}

	T * Get(const Slice & key, uint32_t hash){
		bool rehashing = isRehasing();
		if(!rehashing){//not in rehashing progress
			T ** ptr = FindPtr(slots_, slot_size_, key, hash);
			return *ptr;
		}else{
			ReHashing();
			rehashing = isRehasing();
			T ** ptr = FindPtr(slots_, slot_size_, key, hash);
			if(rehashing && *ptr == NULL) ptr = FindPtr(slots_2_, slot_size_2_, key, hash);
			return *ptr;
		}
	}

	T* Remove(const Slice & key, uint32_t hash){
		bool rehashing = isRehasing();
		T * result = NULL;
		if(!rehashing){//not in rehashing progress
			T ** ptr = FindPtr(slots_, slot_size_, key, hash);
			result = *ptr;
			if(result) {
				--ele_num_;
				*ptr = result->next_;
			}
		}else{
			ReHashing();
			rehashing = isRehasing();
			T ** ptr = FindPtr(slots_, slot_size_, key, hash);
			bool is_in_slots2 = false;
			if(rehashing && *ptr == NULL){
				ptr = FindPtr(slots_2_, slot_size_2_, key, hash);
				is_in_slots2 = (*ptr != NULL);
			}
			result = *ptr;
			if(result) {
				--ele_num_;
				if(is_in_slots2) --rehahing_count_;
				*ptr = result->next_;
			}
		}
		return result;
	}

	bool isRehasing(){
		return slots_2_ != NULL;
	}

	/**
	 * @brief do rehashing, can be called by application in backstage thread to accelerate rehashing process
	 */
	void ReHashing(){
		uint32_t count = 0;
		while(count < REHASH_NUM_ONCE && rehashing_idx_ < slot_size_){
			T ** ptr = &slots_[rehashing_idx_];
			while(*ptr) {
				T * tmp = (*ptr)->next_;
				T ** newPtr = &slots_2_[(*ptr)->hash_ & (slot_size_2_ - 1)];
				(*ptr)->next_ = *newPtr;
				*newPtr = *ptr;
				*ptr = tmp;
				++count;
			}
			++rehashing_idx_;
		}
		rehahing_count_ += count;
		if(rehashing_idx_ == slot_size_ && slots_2_){
			rehashing_idx_ = slot_size_ = slot_size_2_;
			assert(ele_num_ == rehahing_count_);
			ele_num_ = rehahing_count_;
			rehahing_count_ = 0;
			delete [] slots_;
			slots_ = slots_2_;
			slots_2_ = NULL;
		}
	}


private:

	T ** FindPtr(T ** slots, uint32_t slot_size, const Slice & key, uint32_t hash){
		T ** ptr = &slots[hash & (slot_size - 1)];
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
			T ** ptr = &slots_[i];
			while(*ptr){
				T * tmp = (*ptr)->next_;
				T ** newPtr = &newSlot[(*ptr)->hash_ & (base - 1)];
				(*ptr)->next_ = *newPtr;
				*newPtr = (*ptr);
				(*ptr) = tmp;
				++count;
			}
		}

		delete [] slots_;
		assert(count == ele_num_);
		slot_size_ = base;
		slots_ = newSlot;
	}

	void Resize2(){
		if(rehashing_idx_ < slot_size_) return;
		uint32_t base = 8;
		while(base <= ele_num_){
			base <<= 1;
		}
		slot_size_2_ = base;
		slots_2_ = new T*[slot_size_2_];
		memset(slots_2_, 0 , sizeof(slots_2_[0]) * slot_size_2_);
		rehahing_count_ = 0;
		rehashing_idx_ = 0;
		ReHashing();
	}


	T ** slots_;
	T ** slots_2_;//for rehashing
	uint32_t slot_size_;
	uint32_t slot_size_2_;//set before rehashing
	uint32_t rehahing_count_;
	uint32_t rehashing_idx_;//indicate rehashing progress on slot
	uint32_t ele_num_;
};

} /* namespace ares */
#endif /* CACHE_TABLE_H_ */
