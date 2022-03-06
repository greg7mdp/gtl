
<img src="https://github.com/greg7mdp/gtl/blob/main/html/img/phash.png?raw=true" width="120" align="middle"> 

# Hash containers provided by [gtl](https://github.com/greg7mdp/gtl)

[Gtl](https://github.com/greg7mdp/gtl) provides a set of hash containers (maps and sets) implemented using open addressing (single array of values, very cache friendly), as well as advanced SSE lookup optimizations allowing for excellent performance even when the table is up to 95% full. These containers have the same API as the `unordered` versions from the STL, and are significantly outperforming the unordered version both in terms of speed and space.

Here are some of the benefits they offer:


- **drop-in replacement** for `std::unordered_map`, `std::unordered_set`, `std::map` and `std::set`

- **Very efficient**, significantly faster than your compiler's unordered map/set or Boost's

- **Memory friendly**: low memory usage, although a little higher than [sparsepp](https://github.com/greg7mdp/sparsepp)

- Supports **heterogeneous lookup**

- Easy to **forward declare**: just include `phmap_fwd_decl.hpp` in your header files to forward declare Parallel Hashmap containers [note: this does not work currently for hash maps with pointer keys]

- **Dump/load** feature: when a `flat` hash map stores data that is `std::trivially_copyable`, the table can be dumped to disk and restored as a single array, very efficiently, and without requiring any hash computation. This is typically about 10 times faster than doing element-wise serialization to disk, but it will use 10% to 60% extra disk space. See `examples/serialize.cpp`. _(flat hash map/set only)_. When using [Cereal](https://uscilab.github.io/cereal/) and a `BinaryArchive`, flat hash containers automatically use the direct table dump functionality.

- Automatic support for **boost's hash_value()** method for providing the hash function (see `examples/hash_value.hpp`). Also default hash support for `std::pair` and `std::tuple`.

- **natvis** visualization support in Visual Studio

@byronhe kindly provided this [Chinese translation](https://byronhe.com/post/2020/11/10/parallel-hashmap-btree-fast-multi-thread-intro/) of the README.md.


The hashmaps provided here are built upon those open sourced by Google in the Abseil library. The hashmaps use closed hashing, where values are stored directly into a memory array, avoiding memory indirections. By using parallel SSE2 instructions, these hashmaps are able to look up items by checking 16 slots in parallel,  allowing the implementation to remain fast even when the table is filled up to 87.5% capacity.

> **IMPORTANT:** This repository borrows code from the [abseil-cpp](https://github.com/abseil/abseil-cpp) repository, with modifications, and may behave differently from the original. This repository is an independent work, with no guarantees implied or provided by the authors. Please visit [abseil-cpp](https://github.com/abseil/abseil-cpp) for the official Abseil libraries.

The full types with template parameters can be found in the [gtl/phmap_fwd_decl.hpp](https://raw.githubusercontent.com/greg7mdp/gtl/main/gtl/phmap_fwd_decl.hpp) header, which is useful for forward declaring the Parallel Hashmaps when necessary.

## Changes to Abseil's hashmaps

- The default hash framework is std::hash, not absl::Hash. However, if you prefer the default to be the Abseil hash framework, include the Abseil headers before `phmap.hpp` and define the preprocessor macro `GTL_USE_ABSL_HASH`.

- The `erase(iterator)` and `erase(const_iterator)` both return an iterator to the element following the removed element, as does the std::unordered_map. A non-standard `void _erase(iterator)` is provided in case the return value is not needed.

- No new types, such as `absl::string_view`, are provided. All types with a `std::hash<>` implementation are supported by phmap tables (including `std::string_view` of course if your compiler provides it).

- The Abseil hash tables internally randomize a hash seed, so that the table iteration order is non-deterministic. This can be useful to prevent *Denial Of Service*  attacks when a hash table is used for a customer facing web service, but it can make debugging more difficult. The *phmap* hashmaps by default do **not** implement this randomization, but it can be enabled by adding `#define GTL_NON_DETERMINISTIC 1` before including the header `phmap.hpp` (as is done in raw_hash_set_test.cpp).

- Unlike the Abseil hash maps, we do an internal mixing of the hash value provided. This prevents serious degradation of the hash table performance when the hash function provided by the user has poor entropy distribution. The cost in performance is very minimal, and this helps provide reliable performance even with *imperfect* hash functions. 


## Memory usage

|  type                 |    memory usage   | additional *peak* memory usage when resizing  |
|-----------------------|-------------------|-----------------------------------------------|
| flat tables           | ![flat_mem_usage](https://github.com/greg7mdp/gtl/blob/main/html/img/flat_mem_usage.png?raw=true) | ![flat_peak_usage](https://github.com/greg7mdp/gtl/blob/main/html/img/flat_peak.png?raw=true) |
| node tables           | ![node_mem_usage](https://github.com/greg7mdp/gtl/blob/main/html/img/node_mem_usage.png?raw=true) | ![node_peak_usage](https://github.com/greg7mdp/gtl/blob/main/html/img/node_peak.png?raw=true) |


- *size()* is the number of values in the container, as returned by the size() method
- *load_factor()* is the ratio: `size() / bucket_count()`. It varies between 0.4375 (just after the resize) to 0.875 (just before the resize). The size of the bucket array doubles at each resize.
- the value 9 comes from `sizeof(void *) + 1`, as the *node* hash maps store one pointer plus one byte of metadata for each entry in the bucket array.
- flat tables store the values, plus one byte of metadata per value), directly into the bucket array, hence the `sizeof(C::value_type) + 1`.
- the additional peak memory usage (when resizing) corresponds the the old bucket array (half the size of the new one, hence the 0.5), which contains the values to be copied to the new bucket array, and which is freed when the values have been copied.

## Iterator invalidation for hash containers

The rules are the same as for `std::unordered_map`, and are valid for all the gtl hash containers:


|    Operations	                            | Invalidated                |
|-------------------------------------------|----------------------------|
| All read only operations, swap, std::swap | Never                      |
| clear, rehash, reserve, operator=         | Always                     |
| insert, emplace, emplace_hint, operator[] | Only if rehash triggered   |
| erase                                     | Only to the element erased |


## Example - providing a hash function for a user-defined class

In order to use a flat_hash_set or flat_hash_map, a hash function should be provided. This can be done with one of the following methods:

- Provide a hash functor via the HashFcn template parameter

- As with boost, you may add a `hash_value()` friend function in your class. 

For example:
 
```c++
#include <gtl/phmap_utils.hpp> // minimal header providing gtl::HashState()
#include <string>
using std::string;

struct Person
{
    bool operator==(const Person &o) const
    { 
        return _first == o._first && _last == o._last && _age == o._age; 
    }

    friend size_t hash_value(const Person &p)
    {
        return gtl::HashState().combine(0, p._first, p._last, p._age);
    }

    string _first;
    string _last;
    int    _age;
};
```

- Inject a specialization of `std::hash` for the class into the "std" namespace. We provide a convenient and small header `phmap_utils.hpp` which allows to easily add such specializations.

For example:

### file "Person.h"

```c++
#include <gtl/phmap_utils.hpp> // minimal header providing gtl::HashState()
#include <string>
using std::string;

struct Person
{
    bool operator==(const Person &o) const
    { 
        return _first == o._first && _last == o._last && _age == o._age; 
    }

    string _first;
    string _last;
    int    _age;
};

namespace std
{
    // inject specialization of std::hash for Person into namespace std
    // ----------------------------------------------------------------
    template<> struct hash<Person>
    {
        std::size_t operator()(Person const &p) const
        {
            return gtl::HashState().combine(0, p._first, p._last, p._age);
        }
    };
}
```

The `std::hash` specialization for `Person` combines the hash values for both first and last name and age, using the convenient gtl::HashState() function, and returns the combined hash value. 

### file "main.cpp"

```c++
#include "Person.h"   // defines Person  with std::hash specialization

#include <iostream>
#include <gtl/phmap.hpp>

int main()
{
    // As we have defined a specialization of std::hash() for Person,
    // we can now create sparse_hash_set or sparse_hash_map of Persons
    // ----------------------------------------------------------------
    gtl::flat_hash_set<Person> persons = 
        { { "John", "Mitchell", 35 },
          { "Jane", "Smith",    32 },
          { "Jane", "Smith",    30 },
        };

    for (auto& p: persons)
        std::cout << p._first << ' ' << p._last << " (" << p._age << ")" << '\n';

}
```


## Thread safety

[Gtl](https://github.com/greg7mdp/gtl) hash containers follow the thread safety rules of the Standard C++ library. In Particular:

- A single hash container is thread safe for reading from multiple threads. For example, given a hash container A, it is safe to read A from thread 1 and from thread 2 simultaneously.

- If a single hash container is being written to by one thread, then all reads and writes to that hash container on the same or other threads must be protected. For example, given a hash container A, if thread 1 is writing to A, then thread 2 must be prevented from reading from or writing to A. 

- It is safe to read and write to one instance of a type even if another thread is reading or writing to a different instance of the same type. For example, given hash containers A and B of the same type, it is safe if A is being written in thread 1 and B is being read in thread 2.


## Acknowledgements

Many thanks to the Abseil developers for implementing the swiss table (see [abseil-cpp](https://github.com/abseil/abseil-cpp)) upon which this work is based, and to Google for releasing it as open-source. 
