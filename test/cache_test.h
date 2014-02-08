/*
 * CacheTest.h
 *
 *  Created on: 2013-12-18
 *      Author: ewouldblock
 */

#ifndef CACHETEST_H_
#define CACHETEST_H_
#include "gtest/gtest.h"

namespace ares {

class CacheTest : public ::testing::Test {
public:
	CacheTest();
	virtual ~CacheTest();

    void TestCacheTable();

    void TestStringCache();

protected:
    virtual void SetUp() {

    }
    virtual void TearDown(){

    }
};

} /* namespace ares */
#endif /* CACHETEST_H_ */
