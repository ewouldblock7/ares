/*
 * block_queue.h
 *
 *  Created on: 2013年9月1日
 *      Author: seven
 */

#ifndef ARES_BLOCK_QUEUE_H_
#define ARES_BLOCK_QUEUE_H_
#include <queue>
#include "ares/condition.h"

namespace ares {

template <class T>
class BlockQueue {
public:
	BlockQueue() : mutex_(), cond_(&mutex_) {}
	~BlockQueue(){}

	void Produce(const T & t) {
		MutexGuard guard(&mutex_);
		queue_.push(t);
		cond_.Signal();
	}

	T Consume(){
		MutexGuard guard(&mutex_);
		while(queue_.empty()){
			cond_.Wait();
		}
		T front = queue_.front();
		queue_.pop();
		return front;
	}

	uint32_t size() const {
		MutexGuard guard(&mutex_);
		return queue_.size();
	}

private:
	Mutex mutex_;
	Condition cond_;
	std::queue<T> queue_;
};


template <class T>
class BlockQueue <T *>{
public:
	BlockQueue() : mutex_(), cond_(&mutex_) {}
	~BlockQueue(){}

	void Produce(T * t) {
		MutexGuard guard(&mutex_);
		queue_.push(t);
		cond_.Signal();
	}

	T * Consume(){
		MutexGuard guard(&mutex_);
		while(queue_.empty()){
			cond_.Wait();
		}
		T * front = queue_.front();
		queue_.pop();
		return front;
	}

	uint32_t size() const {
		MutexGuard guard(&mutex_);
		return queue_.size();
	}

private:
	Mutex mutex_;
	Condition cond_;
	std::queue<T> queue_;
};

} /* namespace ares */
#endif /* ARES_BLOCK_QUEUE_H_ */
