/*
 * test_condition.cpp

 *
 *  Created on: 2013年8月31日
 *      Author: seven
 */
#include <stdint.h>
#include <string>
#include <queue>
#include <sstream>
#include "ares/block_queue.h"
#include "ares/runnable.h"
#include "ares/mutex.h"
#include "util/log.h"

struct Channel{
	Channel() {
		pthread_cond_init(&cond_, NULL);
	}

	~Channel() {
		pthread_cond_destroy(&cond_);
	}

	ares::Mutex mutex_;
	pthread_cond_t cond_;
	std::queue<std::string> queue_;
};

class Consumer : public ares::Runnable{
public:
	Consumer(uint32_t idx, ares::BlockQueue<std::string> * queue) :
		idx_(idx),
		queue_(queue){

	}
	virtual void run(){
		running_ = true;
		while(running_){
			std::string top = queue_->Consume();
			ARES_DEBUG("Consumer %-5u, %s", idx_, top.c_str());
		}
	}
private:
	uint32_t idx_;
	ares::BlockQueue<std::string> * queue_;
};


class Producer : public ares::Runnable{
public:
	Producer(uint32_t idx, ares::BlockQueue<std::string> * queue) :
		idx_(idx),
		queue_(queue){

	}
	virtual void run(){
		running_ = true;
		uint32_t count = 0;
		while(running_ && count < 1000){
			{
				ARES_DEBUG("Producer %-5u, %u", idx_, count);
				for(uint32_t i = 0; i < 10u; ++i){
					std::stringstream ss;
					ss<<"message_"<<count;
					queue_->Produce(ss.str());
					++count;
				}
			}

			sleep(3);

		}

	}
private:
	uint32_t idx_;
	ares::BlockQueue<std::string> * queue_;
};

int main(int argc, char ** argv){

	ares::BlockQueue<std::string> blockqueue_;

	Channel channel;
	Producer producer(0, &blockqueue_);

	int consumer_num = 1;
	if(argc > 1)
		consumer_num = atoi(argv[1]);
	std::vector<Consumer *> consumers;
	for(uint32_t i = 0; i < consumer_num; ++i){
		consumers.push_back(new Consumer(i + 1, &blockqueue_));
		consumers[i]->start();
	}
	producer.start();

	sleep(30);

	for(uint32_t i = 0; i < consumer_num; ++i){
		consumers[i]->stop();
	}
	producer.stop();

	for(uint32_t i = 0; i < consumer_num; ++i){
		consumers[i]->join();
	}
	producer.join();

	for(uint32_t i = 0; i < consumer_num; ++i){
		delete consumers[i];
	}


}


