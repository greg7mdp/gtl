
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

## Extended APIs

When the *parallel* containers are created with a mutex template parameter, such as [std::mutex](https://en.cppreference.com/w/cpp/thread/mutex), each submap is created with its own mutex, and internal operations such as `erase` are protected by the mutex of the target submap. 

However, standard apis like `find()` returns an iterator, or `operator[]()` returns a reference, and these are not usable in a multithreaded environment under the internal lock protection. For this reason, the *parallel* containers provide extended APIs with callback functions, allowing to execute user code under the internal mutex protection. These APIs are as follows:


#### `if_contains`

if the container contains the provided key, the lambda is called with the `value_type` (under read lock protection), and `if_contains` returns `true`, otherwise the lambda is not called and the function returns `false`. This is a const API and the lambda should not modify the parameter.

```c++
template <class K = key_type, class F>
bool if_contains(const key_arg<K>& key, F&& f) const;
```

example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 9} };
    
    auto val = 0; 
    auto get_value = [&val](const Map::value_type& v) { val = v.second; };
    m.if_contains(2, get_value);
    assert(val == 9);
    
```


#### `modify_if`

if the container contains the provided key, the lambda is called with the `value_type`  (under write lock protection) and `modify_if` returns `true`. This is a non-const API and the lambda is allowed to modify the mapped value.
    
```c++
template <class K = key_type, class F>
bool modify_if(const key_arg<K>& key, F&& f);
```

example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 9} };

    auto set_value = [](Map::value_type& v) { v.second = 11; };
    m.modify_if(2, set_value); // calls lambda which sets m[2] to 11, and returns true
    m.modify_if(3, set_value); // returns false, because m[3] does not exist
```


#### `erase_if`

if the container contains the provided key, the lambda is called with the `value_type` (under write lock protection). If the lambda returns `true`, the key is subsequently erased from the map (the write lock is only released after erase). Returns `true` if key was erased, `false` otherwise.
    
```c++
template <class K = key_type, class F>
bool erase_if(const key_arg<K>& key, F&& f);
```

example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 9}, {5, 6} };

    assert(m.erase_if(9, [](Map::value_type& v) { assert(0); return v.second == 12; }) == false); // m[9] not present - lambda not called
    assert(m.erase_if(5, [](Map::value_type& v) { return v.second == 12; }) == false);            // m[5] == 6, so erase not performed
    assert(m[5] == 6);
    assert(m.erase_if(5, [](Map::value_type& v) { return v.second == 6; }) == true);              // lambda returns true, so m[5] erased
```


#### `try_emplace_l`

if the container does not contains the provided key, it is inserted and the mapped value is value-constructed with the provided arguments (if any), as with `try_emplace`. If the container already  contains the provided key, then the lambda is called with the `value_type` (under write lock protection) and can update the mapped value. Returns `true` if the key was not already present, `false` otherwise.

```c++
template <class K = key_type, class F, class... Args>
bool try_emplace_l(K&& k, F&& f, Args&&... args);
```

example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 9} };

    // overwrite an existing value
    m.try_emplace_l(2, [](Map::value_type& v) { v.second = 5; });
    assert(m[2] == 5);

    // insert a value that is not already present. Will be default initialised to 0 and lambda not called
    m.try_emplace_l(3, 
                    [](Map::value_type& v) { v.second = 6; }); // called only when key was already present
    assert(m[3] == 0);
    
    // insert a value that is not already present, provide argument to value-construct it
    m.try_emplace_l(4, 
                    [](Map::value_type& ) {}, // called only when key was already present
                    999);                     // argument to construct new value is key not present

    assert(m[4] == 999);
```


#### `lazy_emplace_l`

If the container already contains the provided key, the first lambda is called with the `value_type` (under  write lock protection) and can update the mapped value.  if the container does not contains the provided key, the second lambda is called (under  write lock protection as well) and it should invoke the passed constructor to construct the value. Returns `true` if key was not already present, `false` otherwise.
    
```c++
template <class K = key_type, class FExists, class FEmplace>
bool lazy_emplace_l(const key_arg<K>& key, FExists&& fExists, FEmplace&& fEmplace);
```
example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 9} };
 
    // insert a value that is not already present.
    // right now m[5] does not exist
    m.lazy_emplace_l(5, 
                     [](Map::value_type& v) { v.second = 6; },           // called only when key was already present
                     [](const Map::constructor& ctor) { ctor(5, 13); }); // construct value_type in place when key not present 
    assert(m[5] == 13);

    // change a value that is present. Currently m[5] == 13
    m.lazy_emplace_l(5, 
                     [](Map::value_type& v) { v.second = 6; },           // called only when key was already present
                     [](const Map::constructor& ctor) { ctor(5, 13); }); // construct value_type in place when key not present
    assert(m[5] == 6);
```


#### `with_submap`/ `with_submap_m`

Access internal submaps by index (under lock protection). 

```c++
template <class F>
void with_submap(size_t idx, F&& fCallback) const;

template <class F>
void with_submap_m(size_t idx, F&& fCallback); // non-const version
    
```

example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 8}, {5, 11} };
    int counter = 0;
    for (size_t i=0; i<m.subcnt(); ++i) {
        m.with_submap(i, [&](const Map::EmbeddedSet& set) {
            for (auto& p : set) {
                ++counter;
                assert(p.first + 6 == p.second);
            }
        });
    }
    assert(counter == 3);
```


#### `for_each` / `for_each_m`

Iterate over all the values (similar to `std::for_each`), but using the lambda so that the container entries can be accessed under the mutex protection.

```c++
template <class F>
void for_each(F&& fCallback) const;

template <class F>
void for_each_m(F&& fCallback); // non-const version
```

example:

```c++
    using Map = gtl::parallel_flat_hash_map<int, int, gtl::priv::hash_default_hash<int>,
                                            gtl::priv::hash_default_eq<int>,
                                            std::allocator<std::pair<const int, int>>, 
                                            4, std::mutex>;
    Map m = { {1, 7}, {2, 8}, {5, 11} };

    // increment all values by 1
    m.for_each_m([](Map::value_type &pair) {  ++pair.second; });

    int counter = 0;
    m.for_each([&counter](const Map::value_type &pair) {
            ++counter;
            assert(pair.first + 7 == pair.second);
        });
    assert(counter == 3);
```

#### `subcnt`

Returns the number of submaps within  this container.

```c++
static constexpr size_t subcnt();
```


#### `subidx`

Returns the index of the submap that this hash value would map to.

```c++
static size_t subidx(size_t hashval);
```


## Acknowledgements

Many thanks to the Abseil developers for implementing the swiss table (see [abseil-cpp](https://github.com/abseil/abseil-cpp)) upon which this work is based, and to Google for releasing it as open-source. 
