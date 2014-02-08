ares 一个多线程安全内存cache实现
========


1. string_cache 是一个字符串的KV-cache的基类。ShardedLRUStringCache是string_cache的一个实现，  
  并且基于LRU策略的内存cache.支持key是字符串，value是字符串。  



主要逻辑在cache目录下  


使用方法:  
依赖库：boost和gest  
修改makefile里的boost路径和gtest路径为使用者自己配置的路径  
make clean  
make  
./unit_test_cache


