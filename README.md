ares 一个多线程安全内存cache实现
========


1. string_cache 是一个字符串的KV-内存cache的基类。ShardedLRUStringCache是string_cache的一个  
   实现，支持LRU策略，支持渐进式rehash，支持缓存状态输出  



主要逻辑在cache目录下  
cache/cache_table.h  
cache/string_cache.cpp  

使用方法:  
依赖库：boost,gtest,tcmalloc  
修改makefile里的boost路径,gtest路径,tcmalloc路径为使用者自己配置的路径  
make clean  
make  
./unit_test_cache


