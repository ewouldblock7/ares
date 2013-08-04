
BOOST_DIR=/home/seven/work/baidu/third-64/boost
CXX = g++
CXXFLAGS  = -I. -I$(BOOST_DIR)/include
LFLAGS = -lpthread
LIB = 


test_oc_src = cache/object_cache.cpp test/test_mc.cpp util/hash.cpp
test_oc_obj = $(addsuffix .o,$(basename ${test_oc_src}))

TEST_OC = test_objcache
all: $(TEST_OC)

$(TEST_OC) : $(test_oc_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)

%.o	: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
clean:
	rm -rf output $(TEST_OC)
	rm -f *.o */*.o
