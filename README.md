ares
====

一些常用工具集合
1 object_cache 是一个对象内存cache的基类. ShardedLRUObjectCache是object_cache的一个实现。
  它是基于LRU策略的内存cache.参照于leveldb的cache实现，区别在于通过shared_ptr管理对象，而
  不是返回cache自己的handle.省去了让用户管理cache_handle的过程。


使用方法:</br>
在Makefile中指定BOOST路径</br>
make clean</br>
make</br>
./test_objcache</br>


