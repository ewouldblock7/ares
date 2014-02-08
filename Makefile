
CXX = g++ -g -O0
BOOST_LIB = /home/ewouldblock/work/baidu/third-64/boost
GTEST = /home/ewouldblock/work/baidu/app/mobile/flyflow/server/cbs/lib/output/gtest-1.6.0
CXXFLAGS  = -I. -I./include -I$(BOOST_LIB)/include -I$(GTEST)/include
LFLAGS = -lpthread
LIB = 


test_oc_src = cache/object_cache.cpp util/hash.cpp lock/mutex.cpp thread/runnable.cpp test/obect_cache_ex.cpp
test_oc_obj = $(addsuffix .o,$(basename ${test_oc_src}))

test_sc_src = cache/string_cache.cpp util/hash.cpp lock/mutex.cpp thread/runnable.cpp test/string_cache_ex.cpp
test_sc_obj = $(addsuffix .o,$(basename ${test_sc_src}))

test_sc_pressure_src = cache/string_cache.cpp util/hash.cpp lock/mutex.cpp thread/runnable.cpp test/string_cache_pressure.cpp
test_sc_pressure_obj = $(addsuffix .o,$(basename ${test_sc_pressure_src}))

ut_cache_src = test/cache_test.cpp util/hash.cpp lock/mutex.cpp thread/runnable.cpp cache/string_cache.cpp cache/object_cache.cpp
ut_cache_obj = $(addsuffix .o,$(basename ${ut_cache_src}))

test_blockqueue_src = lock/mutex.cpp thread/runnable.cpp thread/condition.cpp test/block_queue_ex.cpp
test_blockqueue_obj = $(addsuffix .o,$(basename ${test_blockqueue_src}))

TEST_OC = test_objcache
TEST_SC = test_strcache
TEST_SC_PRESSURE = test_strcache_pressure
TEST_BLOCKQUEUE = test_blockqueue
UT_CACHE = unit_test_cache
all: $(TEST_OC) $(TEST_SC) $(TEST_SC_PRESSURE) $(TEST_BLOCKQUEUE) $(UT_CACHE)

$(TEST_OC) : $(test_oc_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)
	
$(TEST_SC) : $(test_sc_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)
	
$(TEST_SC_PRESSURE) : $(test_sc_pressure_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)

$(TEST_BLOCKQUEUE) : $(test_blockqueue_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)

$(UT_CACHE) : $(ut_cache_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB) -L$(GTEST)/lib -lgtest -lgtest_main

%.o	: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
clean:
	rm -rf output $(TEST_OC) $(TEST_SC) $(TEST_SC_PRESSURE) $(TEST_BLOCKQUEUE) $(UT_CACHE)
	rm -f core *.o */*.o
