
CXX = g++ -g
CXXFLAGS  = -I.
LFLAGS = -lpthread
LIB = 


test_oc_src = cache/object_cache.cpp util/hash.cpp test/obect_cache_ex.cpp
test_oc_obj = $(addsuffix .o,$(basename ${test_oc_src}))

test_sc_src = cache/string_cache.cpp util/hash.cpp test/string_cache_ex.cpp
test_sc_obj = $(addsuffix .o,$(basename ${test_sc_src}))

TEST_OC = test_objcache
TEST_SC = test_strcache
all: $(TEST_OC) $(TEST_SC)

$(TEST_OC) : $(test_oc_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)
	
$(TEST_SC) : $(test_sc_obj)
	$(CXX) -o $@ $^  $(LFLAGS) $(LIB)

%.o	: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
clean:
	rm -rf output $(TEST_OC) $(TEST_SC)
	rm -f core *.o */*.o
