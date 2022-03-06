
<img src="https://github.com/greg7mdp/gtl/blob/main/html/img/phash.png?raw=true" width="120" align="middle"> 

# Parallel hash containers provided by [gtl](https://github.com/greg7mdp/gtl)







## Memory usage

|  type                 |    memory usage   | additional *peak* memory usage when resizing  |
|-----------------------|-------------------|-----------------------------------------------|
| parallel flat tables  | ![flat_mem_usage](https://github.com/greg7mdp/gtl/blob/main/html/img/flat_mem_usage.png?raw=true) | ![parallel_flat_peak](https://github.com/greg7mdp/gtl/blob/main/html/img/parallel_flat_peak.png?raw=true) |
| parallel node tables  | ![node_mem_usage](https://github.com/greg7mdp/gtl/blob/main/html/img/node_mem_usage.png?raw=true) | ![parallel_node_peak](https://github.com/greg7mdp/gtl/blob/main/html/img/parallel_node_peak.png?raw=true) |



- *size()* is the number of values in the container, as returned by the size() method
- *load_factor()* is the ratio: `size() / bucket_count()`. It varies between 0.4375 (just after the resize) to 0.875 (just before the resize). The size of the bucket array doubles at each resize.
- the value 9 comes from `sizeof(void *) + 1`, as the *node* hash maps store one pointer plus one byte of metadata for each entry in the bucket array.
- flat tables store the values, plus one byte of metadata per value), directly into the bucket array, hence the `sizeof(C::value_type) + 1`.
- the additional peak memory usage (when resizing) corresponds the the old bucket array (half the size of the new one, hence the 0.5), which contains the values to be copied to the new bucket array, and which is freed when the values have been copied.
- the *parallel* hashmaps, when created with a template parameter N=4, create 16 submaps. When the hash values are well distributed, and in single threaded mode, only one of these 16 submaps resizes at any given time, hence the factor `0.03` roughly equal to `0.5 / 16`

## Iterator invalidation for the parallel hash containers

The rules are the same as for `std::unordered_map`, and are valid for all the parallel hash containers:


|    Operations	                            | Invalidated                |
|-------------------------------------------|----------------------------|
| All read only operations, swap, std::swap | Never                      |
| clear, rehash, reserve, operator=         | Always                     |
| insert, emplace, emplace_hint, operator[] | Only if rehash triggered   |
| erase                                     | Only to the element erased |


## Thread safety

[Gtl](https://github.com/greg7mdp/gtl) containers follow the thread safety rules of the Standard C++ library. In Particular:

- A single hash container is thread safe for reading from multiple threads. For example, given a hash container A, it is safe to read A from thread 1 and from thread 2 simultaneously.

- If a single hash container is being written to by one thread, then all reads and writes to that hash container on the same or other threads must be protected. For example, given a hash container A, if thread 1 is writing to A, then thread 2 must be prevented from reading from or writing to A. 

- It is safe to read and write to one instance of a type even if another thread is reading or writing to a different instance of the same type. For example, given hash containers A and B of the same type, it is safe if A is being written in thread 1 and B is being read in thread 2.

- The *parallel* containers can be made internally thread-safe for concurrent read and write access, by providing a synchronization type (for example [std::mutex](https://en.cppreference.com/w/cpp/thread/mutex)) as the last template argument. Because locking is performed at the *submap* level, a high level of concurrency can still be achieved. Read access can be done safely using `if_contains()`, which passes a reference value to the callback while holding the *submap* lock. Similarly, write access can be done safely using `modify_if`, `try_emplace_l` or `lazy_emplace_l`. However, please be aware that iterators or references returned by standard APIs are not protected by the mutex, so they cannot be used reliably on a hash map which can be changed by another thread.

- Examples on how to use various mutex types, including boost::mutex, boost::shared_mutex and absl::Mutex can be found in `examples/bench.cpp`


## Acknowledgements

Many thanks to the Abseil developers for implementing the swiss table (see [abseil-cpp](https://github.com/abseil/abseil-cpp)) upon which this work is based, and to Google for releasing it as open-source. 
