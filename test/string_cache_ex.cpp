/*
 * stringcache.cpp


 *
 *  Created on: 2013年8月17日
 *      Author: seven
 */
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/time.h>
#include "ares/slice.h"
#include "ares/string_cache.h"
#include "ares/runnable.h"

using namespace ares;

class TestRun : public Runnable{
public:
	TestRun(boost::shared_ptr<StringCache> cache, uint32_t index) :
		cache_(cache), index_(index){}
	void run(){
		for(uint32_t i = 0; i < 1000; ++i){
			std::stringstream ss;
			ss<<"run_"<<index_<<"_key_"<<i;
			std::string key = ss.str();
			std::cout<<key<<std::endl;
			Slice skey(key);
			ss.str("");
			ss<<"run_"<<index_<<"_value_"<<i;
			std::string value = ss.str();
			Slice sValue(value);
			cache_->Insert(skey, sValue);
		}
	};

private:
	boost::shared_ptr<StringCache> cache_;
	uint32_t index_;
};

class TestVisitor : public ares::StringCache::Visitor{
public:
	TestVisitor() : count_(0) {}
	~TestVisitor() {}
	void visit(const Slice & key, const Slice & value){
		++count_;
		std::string keyStr(key.data(), key.size());
		std::string valueStr(value.data(), value.size());
		fprintf(stderr, "visit key: %s, %s\n", keyStr.c_str(), valueStr.c_str());
	}

	uint32_t GetCount() {return count_;}
private:
	uint32_t count_;
};


int main(int argc, char ** argv){
	uint32_t capCount = 2000;
	uint64_t capSize = 100000;
	boost::shared_ptr<StringCache> stringCache = newShardedLRUStringCache(capCount, capSize);

	printf("sizeof(int *) %lu, sizeof(int) %lu \n", sizeof(int *), sizeof(int));

	struct timeval now_timeval;
	gettimeofday(&now_timeval, NULL);

	for(uint32_t i = 0; i < 2000; ++i){
		uint32_t index_ = 77;
		std::string ttkey("test_timeout_key");
		std::stringstream ss;
		ss<<"run_"<<index_<<"test_timeout_key_"<<i;
		std::string key = ss.str();
		std::cout<<key<<std::endl;
		Slice skey(key);
		ss.str("");
		ss<<"run_"<<index_<<"test_timeout_value_"<<i;
		std::string value = ss.str();

		uint32_t timeout = 10;
		if(i < 1500 && i > 1400){
			timeout  = 3600;
		}

		stringCache->Insert(key, value, timeout);
		if(i == 1000){
			sleep(10);
		}
	}

	sleep(12);

	char value_buffer[256] = {0};
	for(uint32_t i = 0; i < 2000; ++i){
		uint32_t index_ = 77;
		std::string ttkey("test_timeout_key");
		std::stringstream ss;
		ss<<"run_"<<index_<<"test_timeout_key_"<<i;
		std::string key = ss.str();
		uint32_t  ret = stringCache->Get(key, value_buffer, sizeof(value_buffer));
		if (ret)
			printf("get key:%s ret:%u, value:%s \n", key.c_str(), ret,
					value_buffer);
		else
			printf("get key:%s ret:%u \n", key.c_str(), ret);
	}


//	uint32_t runnum = 4;
//	TestRun ** runnables = new TestRun*[runnum];
//	for(uint32_t i = 0; i < runnum; ++i){
//		runnables[i] = new TestRun(stringCache, i);
//	}
//
//
//	for(uint32_t i = 0; i < runnum; ++i){
//		runnables[i]->start();
//	}
//
//
//	for(uint32_t i = 0; i < runnum; ++i){
//		runnables[i]->join();
//	}
//
//	TestVisitor visitor;
//	stringCache->Visit(visitor);
//
//	fprintf(stderr, "TestVisitor count: %u \n", visitor.GetCount());
//
//	for(uint32_t i = 0; i < runnum; ++i){
//		delete runnables[i];
//	}
//	delete [] runnables;
}



