/*
 * test_mc.cpp

 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "string/slice.h"
#include "cache/cache_visitor.h"
#include "cache/object_cache.h"
#include "thread/runnable.h"

using namespace ares;

struct TestObject{
	uint64_t idx_;
	std::string s_;
};

void DelTestObject(void * arg){
	TestObject * tobject = reinterpret_cast<TestObject *>(arg);
	delete tobject;
}

class TestVisitor : public ares::ObjectCacheVisitor{
public:
	TestVisitor() : count_(0) {}
	~TestVisitor() {}
	void visit(const Slice & key, const boost::shared_ptr<void> & value){
		++count_;
		const TestObject * tp = static_cast<TestObject *>(value.get());
		fprintf(stderr, "visit key: %s, %lu\n", key.data(), tp->idx_);
	}

	uint32_t GetCount() {return count_;}
private:
	uint32_t count_;
};

class TestRun : public Runnable{
public:
	TestRun(boost::shared_ptr<ObjectCache> cache, uint32_t index) :
		cache_(cache), index_(index){}
	void run(){
		for(uint32_t i = 0; i < 500; ++i){
			std::stringstream ss;
			ss<<"run_"<<index_<<"_key_"<<i;
			std::string key = ss.str();
			std::cout<<key<<std::endl;
			Slice skey(key);
			boost::shared_ptr<TestObject> tobject(new TestObject());
			tobject->idx_ = i;
			tobject->s_ = key;
			boost::shared_ptr<void> vtobject = tobject;
			cache_->Insert(skey, vtobject, 1);
		}
	};

private:
	boost::shared_ptr<ObjectCache> cache_;
	uint32_t index_;
};

int main(int argc, char ** argv){
	uint64_t chargeCap = 100;
	uint32_t countCap = 100;
	boost::shared_ptr<ObjectCache> objectCache = newShardedLRUObjectCache(chargeCap, countCap);

	for(uint32_t i = 0; i < 500; ++i){
		std::stringstream ss;
		ss<<"key_"<<i;
		std::string key = ss.str();
		std::cout<<key<<std::endl;
		Slice skey(key);
		boost::shared_ptr<TestObject> tobject(new TestObject());
		tobject->idx_ = i;
		tobject->s_ = key;
		boost::shared_ptr<void> vtobject = tobject;
		TestObject * tp = static_cast<TestObject *>(vtobject.get());
		tp->idx_;
		objectCache->Insert(skey, vtobject, 1);
	}
	TestVisitor visitor;
	objectCache->Visit(visitor);
	fprintf(stderr, "TestVisitor count: %u \n", visitor.GetCount());


	uint32_t runnum = 4;
	TestRun ** runnables = new TestRun*[runnum];
	for(uint32_t i = 0; i < runnum; ++i){
		runnables[i] = new TestRun(objectCache, i);
	}


	for(uint32_t i = 0; i < runnum; ++i){
		runnables[i]->run();
	}

	for(uint32_t i = 0; i < runnum; ++i){
		runnables[i]->join();
	}

	for(uint32_t i = 0; i < runnum; ++i){
		delete runnables[i];
	}
	delete [] runnables;

	TestVisitor visitor2;
	objectCache->Visit(visitor2);
	fprintf(stderr, "TestVisitor count: %u \n", visitor2.GetCount());
}
