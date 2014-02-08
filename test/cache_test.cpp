/*
 * CacheTest.cpp
 *
 *  Created on: 2013-12-18
 *      Author: ewouldblock
 */

#include "test/cache_test.h"
#include <vector>
#include <sstream>
#include "cache/cache_table.h"
#include "ares/string_cache.h"
#include "util/hash.h"

namespace ares {

CacheTest::CacheTest() {
	// TODO Auto-generated constructor stub

}

CacheTest::~CacheTest() {
	// TODO Auto-generated destructor stub
}

struct CacheHandle{
	CacheHandle * lru_prev_;
	CacheHandle * lru_next_;
	CacheHandle * next_;
	uint32_t hash_;
	uint32_t key_size_;
	uint32_t value_size_;
	uint32_t time_out_;
	char buf_[0];

	Slice key() {
		return Slice(buf_, key_size_);
	}
};

void CacheTest::TestCacheTable(){

	{
		CacheTable<CacheHandle> table;
		uint32_t count = 1024;
		std::vector<CacheHandle *> handles;
		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss<<"key_"<<i;
			std::string key = ss.str();
			uint32_t size = sizeof(CacheHandle) + key.size() + 1;
			CacheHandle * handle = static_cast<CacheHandle *>(malloc(size));
			memset(handle, 0 , size);
			memcpy(handle->buf_, key.c_str(), key.size());
			handle->hash_ = Hash(key.c_str(), key.size());
			handle->key_size_ = key.size();
			handles.push_back(handle);
		}

		for(uint32_t i = 0; i < count; ++i){
			table.Insert(handles[i]->key(), handles[i]);
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			CacheHandle * handle = table.Get(key, Hash(key.c_str(), key.size()));
			EXPECT_EQ(handle, handles[i]);
			if(handle != handles[i]){
				printf("%s get no equal \n", key.c_str());
			}
		}

		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			CacheHandle * handle = table.Get(key, Hash(key.c_str(), key.size()));
			EXPECT_EQ(handle, handles[i]);
			if(handle != handles[i]){
				printf("%s get no equal \n", key.c_str());
			}
		}

		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			CacheHandle * handle = table.Remove(key, Hash(key.c_str(), key.size()));
			EXPECT_EQ(handle, handles[i]);
			if (handle != handles[i]) {
				printf("%s remove no equal \n", key.c_str());
			}

			handle = table.Get(key, Hash(key.c_str(), key.size()));
			EXPECT_TRUE(handle == NULL);
		}

		for(uint32_t i = 0; i < handles.size(); ++i){
			free(handles[i]);
		}

	}



	{
		CacheTable<CacheHandle> table;
		uint32_t count = 1024;
		std::vector<CacheHandle *> handles;
		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss<<"key_"<<i;
			std::string key = ss.str();
			uint32_t size = sizeof(CacheHandle) + key.size() + 1;
			CacheHandle * handle = static_cast<CacheHandle *>(malloc(size));
			memset(handle, 0 , size);
			memcpy(handle->buf_, key.c_str(), key.size());
			handle->hash_ = Hash(key.c_str(), key.size());
			handle->key_size_ = key.size();
			handles.push_back(handle);
		}

		for(uint32_t i = 0; i < count; ++i){
			table.Insert(handles[i]->key(), handles[i]);
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			CacheHandle * handle = table.Get(key, Hash(key.c_str(), key.size()));
			EXPECT_EQ(handle, handles[i]);
			if(handle != handles[i]){
				printf("%s get no equal \n", key.c_str());
			}

			ss<<"no_exist";
			std::string key_noexist = ss.str();
			handle = table.Get(key_noexist,  Hash(key_noexist.c_str(), key_noexist.size()));
			EXPECT_TRUE(handle == NULL);
			handle = table.Remove(key_noexist,  Hash(key_noexist.c_str(), key_noexist.size()));
			EXPECT_TRUE(handle == NULL);

			if(i > count / 2 && i % 4 == 0){
				handle = table.Remove(key, Hash(key.c_str(), key.size()));
				EXPECT_EQ(handle, handles[i]);
				if(handle != handles[i]){
					printf("%s remove no equal \n", key.c_str());
				}
			}

		}

		for(uint32_t i = 0; i < handles.size(); ++i){
			free(handles[i]);
		}
	}





}


void CacheTest::TestStringCache(){

	{
		boost::shared_ptr<StringCache> strcache_ptr = newShardedLRUStringCache(10000, 1024 * 1024 * 10);
		uint32_t count = 1024;
		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			strcache_ptr->Insert(key, value);
		}

		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			std::string value_get;
			strcache_ptr->Get(key, value_get);
			EXPECT_EQ(value, value_get);
		}
	}

	{
		boost::shared_ptr<StringCache> strcache_ptr = newShardedLRUStringCache(10000, 1024 * 1024 * 10);
		uint32_t count = 1024;
		uint32_t tick = time(NULL);
		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			strcache_ptr->Insert(key, value, tick + 60);
		}

		sleep(61);
		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			std::string value_get;
			strcache_ptr->Get(key, value_get);
			EXPECT_TRUE(value_get.empty());
		}
	}


	{
		boost::shared_ptr<StringCache> strcache_ptr = newShardedLRUStringCache(512, 1024 * 1024 * 10);
		uint32_t count = 1024;
		for(uint32_t i = 0; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			strcache_ptr->Insert(key, value);
		}

		for(uint32_t i = 0; i < 100; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			std::string value_get;
			strcache_ptr->Get(key, value_get);
			EXPECT_TRUE(value_get.empty());
		}


		for(uint32_t i = 700; i < count; ++i){
			std::stringstream ss;
			ss << "key_" << i;
			std::string key = ss.str();
			ss.str("");
			ss << "value_" << i;
			std::string value = ss.str();
			std::string value_get;
			strcache_ptr->Get(key, value_get);
			EXPECT_EQ(value, value_get);
		}
	}
}


TEST_F(CacheTest, testCacheTable)
{
	TestCacheTable();
}

TEST_F(CacheTest, testStrCache)
{
	TestStringCache();
}

} /* namespace ares */
