ares
====

一些常用工具集合
1 object_cache 是一个对象内存KV-cache的基类. ShardedLRUObjectCache是object_cache的一个实现。  
  它是基于LRU策略的内存cache.参照于leveldb的cache实现，区别在于通过shared_ptr管理对象，而  
  不是返回cache自己的handle.省去了让用户管理cache_handle的过程。  
2 string_cache 是一个字符串的KV-cache的基类。ShardedLRUStringCache是string_cache的一个实现，  
  基于LRU策略的内存cache.支持key是字符串，value是字符串。  


使用方法:  
需要安装boost
make clean  
make  
./test_objcache  


