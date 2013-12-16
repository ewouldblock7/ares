/*
 * stringcache.cpp


 *
 *  Created on: 2013年8月17日
 *      Author: ewouldblock7
 */
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "ares/slice.h"
#include "ares/string_cache.h"
#include "ares/runnable.h"
#include "util/tick_count.h"

using namespace ares;

static uint64_t ITEM_NUM = 1000000;
static uint32_t ITEM_SIZE = 5048;
static uint32_t TASK_NUM = 5;

static uint64_t CAP_SIZE = 1024UL * 1024 * 1024;

void generateRandomString(std::string & str){
    str.reserve(ITEM_SIZE);
    for(uint32_t i = 0; i < ITEM_SIZE; ++i){
        char c = random() % 26 + 'a';
        str.append(1, c);
    }
}

class TestRun : public Runnable{
public:
	TestRun(boost::shared_ptr<StringCache> cache, uint32_t index) :
		cache_(cache), index_(index), longest_consume_(0), shortest_consume_(ULONG_MAX){}
	virtual void run(){
		for(uint32_t i = 0; i < ITEM_NUM; ++i){
			std::stringstream ss;
			ss<<"run_"<<index_<<"_key_"<<i;
			std::string key = ss.str();
			std::cout<<"write:"<<key<<std::endl;
			Slice skey(key);
			ss.str("");

			std::string value;
			generateRandomString(value);

			ss<<"run_"<<index_<<"_value_"<<i<<value;
			std::string add_value = ss.str();
			Slice sValue(add_value);
			TickCount tick_count;
			tick_count.Begin();
			cache_->Insert(skey, sValue);
			tick_count.End();
			uint64_t consume = tick_count.GetConsumeUs();
			if(consume > longest_consume_) longest_consume_ = consume;
			if(consume < shortest_consume_) shortest_consume_ = consume;
		}
	};

	uint64_t getLongGestConsume() {return longest_consume_;}
	uint64_t getShortedGestConsume() {return shortest_consume_;}

	virtual const char * descripe() {return "run insert";}

protected:
	boost::shared_ptr<StringCache> cache_;
	uint32_t index_;
	uint64_t longest_consume_;
	uint64_t shortest_consume_;
};

class TestRunInsertNoRead : public TestRun{
public:
	TestRunInsertNoRead(boost::shared_ptr<StringCache> cache, uint32_t index) :
			TestRun(cache, index) {
	}
	virtual void run(){
        for(uint32_t i = 0; i < ITEM_NUM; ++i){
            std::stringstream ss;
            ss<<"run_"<<index_<<"_key_"<<i;
            std::string key = ss.str();
            std::cout<<"write:"<<key<<std::endl;
            Slice skey(key);

            std::string add_value;
            Slice sValue(add_value);
			TickCount tick_count;
			tick_count.Begin();
            cache_->Insert(skey, sValue);
			tick_count.End();
			uint64_t consume = tick_count.GetConsumeUs();
			if (consume > longest_consume_)
				longest_consume_ = consume;
			if (consume < shortest_consume_)
				shortest_consume_ = consume;
        }
    };

	virtual const char * descripe() {return "run insert no read";}

};

class TestRunRead : public TestRun{
public:
	TestRunRead(boost::shared_ptr<StringCache> cache, uint32_t index) :
			TestRun(cache, index) {
	}
    virtual void run(){
        for(uint32_t i = 0; i < ITEM_NUM; ++i){
            std::stringstream ss;
            ss<<"run_"<<index_<<"_key_"<<i;
            std::string key = ss.str();
            std::cout<<"read:"<<key<<std::endl;
            Slice skey(key);
            std::string value;
			TickCount tick_count;
			tick_count.Begin();
            cache_->Get(skey, value);
			tick_count.End();
			uint64_t consume = tick_count.GetConsumeUs();
			if (consume > longest_consume_)
				longest_consume_ = consume;
			if (consume < shortest_consume_)
				shortest_consume_ = consume;
        }
    };

    virtual const char * descripe() {return "run read";}
};

class TestVisitor : public ares::StringCache::Visitor{
public:
	TestVisitor() : count_(0) {}
	~TestVisitor() {}
	void visit(const Slice & key, const Slice & value){
		++count_;
		std::string keyStr(key.data(), key.size());
		std::string valueStr(value.data(), value.size());
	}

	uint32_t GetCount() {return count_;}
private:
	uint32_t count_;
};


int main(int argc, char ** argv){
	uint32_t capCount = ITEM_NUM * TASK_NUM;
	char c = 0;
    while (-1 != (c = getopt(argc, argv, "n:c:m:t"))) {
        switch (c) {
        case 'n':
        	ITEM_NUM = atol(optarg);
        	capCount = ITEM_NUM * TASK_NUM;
            break;
        case 'c':
        	capCount = atol(optarg);
            break;
        case 'm':
        	CAP_SIZE = atol(optarg);
            break;
        case 'f':
        	TASK_NUM = atoi(optarg);
        	capCount = ITEM_NUM * TASK_NUM;
            break;
        default:;
        }
    }


	boost::shared_ptr<StringCache> stringCache = newShardedLRUStringCache(capCount, CAP_SIZE);

	printf("sizeof(int *) %lu, sizeof(int) %lu \n", sizeof(int *), sizeof(int));

	uint32_t runnum = TASK_NUM;
	std::vector<TestRun *> runnables;
	for(uint32_t i = 0; i < runnum; ++i){
	    runnables.push_back(new TestRun(stringCache, i));
	}
	runnables.push_back(new TestRunInsertNoRead(stringCache, 10));

	for(uint32_t i = 0; i < runnum; ++i){
	    runnables.push_back(new TestRunRead(stringCache, i));
	}

	runnables.push_back(new TestRunRead(stringCache, 12));


	for(uint32_t i = 0; i < runnables.size(); ++i){
		runnables[i]->start();
	}


	for(uint32_t i = 0; i < runnables.size(); ++i){
		runnables[i]->join();
	}

	for(uint32_t i = 0; i < runnables.size(); ++i){
		TestRun * test_run = runnables[i];
		fprintf(stderr, "%-20s longest_consume: %-10lu, shortest_consume:%-10lu \n",
				test_run->descripe(), test_run->getLongGestConsume(), test_run->getShortedGestConsume());
	}

	TestVisitor visitor;
	stringCache->Visit(visitor);

	fprintf(stderr, "TestVisitor count: %u \n", visitor.GetCount());

	for(uint32_t i = 0; i < runnables.size(); ++i){
		delete runnables[i];
	}
}







