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
	uint32_t capCount = 1000;
	uint64_t capSize = 10000;
	boost::shared_ptr<StringCache> stringCache = newShardedLRUStringCache(capCount, capSize);
	uint32_t runnum = 4;
	TestRun ** runnables = new TestRun*[runnum];
	for(uint32_t i = 0; i < runnum; ++i){
		runnables[i] = new TestRun(stringCache, i);
	}


	for(uint32_t i = 0; i < runnum; ++i){
		runnables[i]->start();
	}


	for(uint32_t i = 0; i < runnum; ++i){
		runnables[i]->join();
	}

	TestVisitor visitor;
	stringCache->Visit(visitor);

	fprintf(stderr, "TestVisitor count: %u \n", visitor.GetCount());

	for(uint32_t i = 0; i < runnum; ++i){
		delete runnables[i];
	}
	delete [] runnables;
}



