
<img src="https://github.com/greg7mdp/gtl/blob/main/html/img/phash.png?raw=true" width="120" align="middle"> 

# Greg's Template Library of useful classes.

  [![License: Apache-2.0](https://img.shields.io/badge/License-Apache-yellow.svg)](https://opensource.org/licenses/Apache-2.0) [![Linux](https://github.com/greg7mdp/gtl/actions/workflows/linux.yml/badge.svg)](https://github.com/greg7mdp/gtl/actions/workflows/linux.yml)  [![MacOS](https://github.com/greg7mdp/gtl/actions/workflows/macos.yml/badge.svg)](https://github.com/greg7mdp/gtl/actions/workflows/macos.yml) [![Windows](https://github.com/greg7mdp/gtl/actions/workflows/windows.yml/badge.svg)](https://github.com/greg7mdp/gtl/actions/workflows/windows.yml) 

## Overview

This repository aims to provide many classes that are commonly needed in substantial C++ projects, but that are either not available in the C++ standard library, or have a specification which makes them slower than they could beslow implementation. In some cases, the C++ standard requirements prevents from providing faster alternatives (for example the  pointer stability requirement for unordered maps or sets prevents providing implementations using open addressing).

Among the many classes offered by [gtl](https://github.com/greg7mdp/gtl), we have a set of excellent **hash map** implementations, as well as a **btree** alternative to `std::map` and `std::set`. These are *drop-in replacements* for the standard C++ classes and provide the same API, but are significantly faster and use less memory.

We are happy to integrate new classes into [gtl](https://github.com/greg7mdp/gtl), provided the license is compatible with ours, and we feel they will be useful to most users. Often, when integrating classes from other sources, we are able to improve their performance both in time and space by using other classes already available in [gtl](https://github.com/greg7mdp/gtl) (such as hash maps, btree, bit_vector, etc...)  instead of the spandard ones.

[gtl](https://github.com/greg7mdp/gtl) requires a C++20 compiler. We currently support:  `Visual Studio 2019 +`, `gcc 11.1 +`, and `clang from Xcode 13.2+`.

Because [gtl](https://github.com/greg7mdp/gtl) is a header only library, installation is trivial, just copy the `gtl` directory to your project and you are good to go. We also support common package managers such as [Conan](https://conan.io/) and [vcpkg](https://vcpkg.io/en/index.html).

## Installation

Copy the gtl directory to your project. Update your include path. That's all.

If you are using Visual Studio, you probably want to add `gtl/debug_vis/gtl.natvis` to your projects. This will allow for a user friendly display of gtl containers in the debugger. Similar debug visualizers are also provided for gdb and lldb in the \gtl/debug_vis` directory.

> A cmake configuration files (CMakeLists.txt) is provided for building the tests and examples. Command for building and running the tests is: <br>
> `mkdir build && cd build && cmake -DGTL_BUILD_TESTS=ON -DGTL_BUILD_EXAMPLES=ON .. && cmake --build . && make test`


Following is a short look at the various classes available in [gtl](https://github.com/greg7mdp/gtl). In many cases, a more complete description is linked.


## Hash containers

[Gtl](https://github.com/greg7mdp/gtl) provides a set of hash containers (maps and sets) implemented using open addressing (single array of values, very cache friendly), as well as advanced SSE lookup optimizations allowing for excellent performance even when the container is up to 87% full. These containers have the same API as the `unordered` versions from the STL, and are significantly outperforming the unordered version both in terms of speed and space.

The four provided hash containers are:
    - `gtl::flat_hash_map`
    - `gtl::flat_hash_set`
    - `gtl::node_hash_map`
    - `gtl::node_hash_set`

For more information on the hash containers, please see [gtl hash containers](https://github.com/greg7mdp/gtl/tree/main/docs/hmap.md)

Here is a very basic example of using the gtl::flat_hash_map:

```c++
#include <iostream>
#include <string>
#include <gtl/phmap.hpp>

using gtl::flat_hash_map;
 
int main()
{
    // Create an unordered_map of three strings (that map to strings)
    flat_hash_map<std::string, std::string> email = 
    {
        { "tom",  "tom@gmail.com"},
        { "jeff", "jk@gmail.com"},
        { "jim",  "jimg@microsoft.com"}
    };
 
    // Iterate and print keys and values 
    for (const auto& n : email) 
        std::cout << n.first << "'s email is: " << n.second << "\n";
 
    // Add a new entry
    email["bill"] = "bg@whatever.com";
 
    // and print it
    std::cout << "bill's email is: " << email["bill"] << "\n";
 
    return 0;
}
```

**Key decision points for hash containers:**

- The `flat` hash containers do not provide pointer stability. This means that when the container resizes, it will move the keys and values in memory. So pointers to something inside a `flat` hash containerc will become invalid when the container is resized. The `node` hash containers do provide pointer stability, and should be used instead if this is an issue.

- The `flat` hash containers will use less memory, and usually are faster than the `node` hash containers, so use them if you can. the exception is when the values inserted in the hash container are large (say more than 100 bytes [*needs testing*]) and expensive to move.

- The `parallel` hash containers are preferred when you have a few hash containers that will store a very large number of values. The `non-parallel` hash containers are preferred if you have a large number of hash containers, each storing a relatively small number of values.

- The benefits of the `parallel` hash containers are:  
   a. reduced peak memory usage (when resizing), and  
   b. multithreading support (and inherent internal parallelism)



## Parallel hash containers


The four provided parallel hash containers are:
    - `gtl::parallel_flat_hash_map`
    - `gtl::parallel_flat_hash_set`
    - `gtl::parallel_node_hash_map`
    - `gtl::parallel_node_hash_set`


For a full writeup explaining the design and benefits of the parallel hash containers, [click here](https://greg7mdp.github.io/gtl/).

For more information on the implementation, usage and characteristics of the parallel hash containers, please see [gtl parallel hash containers](https://github.com/greg7mdp/gtl/tree/main/docs/phmap.md)

Here is a very basic example of using the gtl::flat_hash_map:


## Btree containers


The four provided btree containers are:
    - `gtl::btree_map`
    - `gtl::btree_set`
    - `gtl::btree_multimap`
    - `gtl::btree_multiset`

For more information on the hash containers, please see [gtl btree containers](https://github.com/greg7mdp/gtl/tree/main/docs/btree.md)

**Key decision points for btree containers:**

Btree containers are ordered containers, which can be used as alternatives to `std::map` and `std::set`. They store multiple values in each tree node, and are therefore more cache friendly and use significantly less memory.

Btree containers will usually be preferable to the default red-black trees of the STL, except when:
- pointer stability or iterator stability is required
- the value_type is large and expensive to move

When an ordering is not needed, a hash container is typically a better choice than a btree one.













All [gtl](https://github.com/greg7mdp/gtl) classes have the following characteristics: 

- **Header only**: nothing to build, just copy the `gtl` directory to your project and you are good to go.

- Containers are **drop-in replacement** for `std::unordered_map`, `std::unordered_set`, `std::map` and `std::set`

- **Very efficient**, significantly faster than your compiler's unordered map/set or Boost's, or than [sparsepp](https://github.com/greg7mdp/sparsepp)

- **Memory friendly**: low memory usage, although a little higher than [sparsepp](https://github.com/greg7mdp/sparsepp)

- Supports **heterogeneous lookup**

- Easy to **forward declare**: just include `phmap_fwd_decl.hpp` in your header files to forward declare Parallel Hashmap containers [note: this does not work currently for hash maps with pointer keys]

- **Dump/load** feature: when a `flat` hash map stores data that is `std::trivially_copyable`, the container can be dumped to disk and restored as a single array, very efficiently, and without requiring any hash computation. This is typically about 10 times faster than doing element-wise serialization to disk, but it will use 10% to 60% extra disk space. See `examples/serialize.cpp`. _(flat hash map/set only)_

- **Tested** on Windows (vs2015 & vs2017, vs2019, Intel compiler 18 and 19), linux (g++ 4.8.4, 5, 6, 7, 8, clang++ 3.9, 4.0, 5.0) and MacOS (g++ and clang++) - click on travis and appveyor icons above for detailed test status.

- Automatic support for **boost's hash_value()** method for providing the hash function (see `examples/hash_value.hpp`). Also default hash support for `std::pair` and `std::tuple`.

- **natvis** visualization support in Visual Studio _(hash map/set only)_

@byronhe kindly provided this [Chinese translation](https://byronhe.com/post/2020/11/10/parallel-hashmap-btree-fast-multi-thread-intro/) of the README.md.


## Fast *and*  memory friendly

Click here [For a full writeup explaining the design and benefits of the Parallel Hashmap](https://greg7mdp.github.io/gtl/).

The hashmaps and btree provided here are built upon those open sourced by Google in the Abseil library. The hashmaps use closed hashing, where values are stored directly into a memory array, avoiding memory indirections. By using parallel SSE2 instructions, these hashmaps are able to look up items by checking 16 slots in parallel,  allowing the implementation to remain fast even when the container is filled up to 87.5% capacity.

> **IMPORTANT:** This repository borrows code from the [abseil-cpp](https://github.com/abseil/abseil-cpp) repository, with modifications, and may behave differently from the original. This repository is an independent work, with no guarantees implied or provided by the authors. Please visit [abseil-cpp](https://github.com/abseil/abseil-cpp) for the official Abseil libraries.

## Installation

Copy the gtl directory to your project. Update your include path. That's all.

If you are using Visual Studio, you probably want to add `gtl.natvis` to your projects. This will allow for a clear display of the hash container contents in the debugger.

> A cmake configuration files (CMakeLists.txt) is provided for building the tests and examples. Command for building and running the tests is: `mkdir build && cd build && cmake -DGTL_BUILD_TESTS=ON -DGTL_BUILD_EXAMPLES=ON .. && cmake --build . && make test`

## Various hash maps and their pros and cons

The header `gtl/phmap.hpp` provides the implementation for the following eight hash containers:
- gtl::flat_hash_set
- gtl::flat_hash_map
- gtl::node_hash_set
- gtl::node_hash_map
- gtl::parallel_flat_hash_set
- gtl::parallel_flat_hash_map
- gtl::parallel_node_hash_set
- gtl::parallel_node_hash_map

The header `gtl/btree.hpp` provides the implementation for the following btree-based ordered containers:
- gtl::btree_set
- gtl::btree_map
- gtl::btree_multiset
- gtl::btree_multimap

The btree containers are direct ports from Abseil, and should behave exactly the same as the Abseil ones, modulo small differences (such as supporting std::string_view instead of absl::string_view, and being forward declarable).

When btrees are mutated, values stored within can be moved in memory. This means that pointers or iterators to values stored in btree containers can be invalidated when that btree is modified. This is a significant difference with `std::map` and `std::set`, as the std containers do offer a guarantee of pointer stability. The same is true for the 'flat' hash maps and sets.

The full types with template parameters can be found in the [gtl/phmap_fwd_decl.hpp](https://raw.githubusercontent.com/greg7mdp/gtl/main/gtl/phmap_fwd_decl.hpp) header, which is useful for forward declaring the Parallel Hashmaps when necessary.

**Key decision points for hash containers:**

- The `flat` hash maps will move the keys and values in memory. So if you keep a pointer to something inside a `flat` hash map, this pointer may become invalid when the map is mutated. The `node` hash maps don't, and should be used instead if this is a problem.

- The `flat` hash maps will use less memory, and usually be faster than the `node` hash maps, so use them if you can. the exception is when the values inserted in the hash map are large (say more than 100 bytes [*needs testing*]) and costly to move.

- The `parallel` hash maps are preferred when you have a few hash maps that will store a very large number of values. The `non-parallel` hash maps are preferred if you have a large number of hash maps, each storing a relatively small number of values.

- The benefits of the `parallel` hash maps are:  
   a. reduced peak memory usage (when resizing), and  
   b. multithreading support (and inherent internal parallelism)

**Key decision points for btree containers:**

Btree containers are ordered containers, which can be used as alternatives to `std::map` and `std::set`. They store multiple values in each tree node, and are therefore more cache friendly and use significantly less memory.

Btree containers will usually be preferable to the default red-black trees of the STL, except when:
- pointer stability or iterator stability is required
- the value_type is large and expensive to move

When an ordering is not needed, a hash container is typically a better choice than a btree one.

## Changes to Abseil's hashmaps

- The default hash framework is std::hash, not absl::Hash. However, if you prefer the default to be the Abseil hash framework, include the Abseil headers before `phmap.hpp` and define the preprocessor macro `GTL_USE_ABSL_HASH`.

- The `erase(iterator)` and `erase(const_iterator)` both return an iterator to the element following the removed element, as does the std::unordered_map. A non-standard `void _erase(iterator)` is provided in case the return value is not needed.

- No new types, such as `absl::string_view`, are provided. All types with a `std::hash<>` implementation are supported by phmap containers (including `std::string_view` of course if your compiler provides it).

- The Abseil hash containers internally randomize a hash seed, so that the container iteration order is non-deterministic. This can be useful to prevent *Denial Of Service*  attacks when a hash container is used for a customer facing web service, but it can make debugging more difficult. The *phmap* hashmaps by default do **not** implement this randomization, but it can be enabled by adding `#define GTL_NON_DETERMINISTIC 1` before including the header `phmap.hpp` (as is done in raw_hash_set_test.cpp).

- Unlike the Abseil hash maps, we do an internal mixing of the hash value provided. This prevents serious degradation of the hash container performance when the hash function provided by the user has poor entropy distribution. The cost in performance is very minimal, and this helps provide reliable performance even with *imperfect* hash functions. 


## Memory usage

|  type                 |    memory usage   | additional *peak* memory usage when resizing  |
|-----------------------|-------------------|-----------------------------------------------|
| flat tables           | ![flat_mem_usage](https://github.com/greg7mdp/gtl/blob/master/html/img/flat_mem_usage.png?raw=true) | ![flat_peak_usage](https://github.com/greg7mdp/gtl/blob/master/html/img/flat_peak.png?raw=true) |
| node tables           | ![node_mem_usage](https://github.com/greg7mdp/gtl/blob/master/html/img/node_mem_usage.png?raw=true) | ![node_peak_usage](https://github.com/greg7mdp/gtl/blob/master/html/img/node_peak.png?raw=true) |
| parallel flat tables  | ![flat_mem_usage](https://github.com/greg7mdp/gtl/blob/master/html/img/flat_mem_usage.png?raw=true) | ![parallel_flat_peak](https://github.com/greg7mdp/gtl/blob/master/html/img/parallel_flat_peak.png?raw=true) |
| parallel node tables  | ![node_mem_usage](https://github.com/greg7mdp/gtl/blob/master/html/img/node_mem_usage.png?raw=true) | ![parallel_node_peak](https://github.com/greg7mdp/gtl/blob/master/html/img/parallel_node_peak.png?raw=true) |


- *size()* is the number of values in the container, as returned by the size() method
- *load_factor()* is the ratio: `size() / bucket_count()`. It varies between 0.4375 (just after the resize) to 0.875 (just before the resize). The size of the bucket array doubles at each resize.
- the value 9 comes from `sizeof(void *) + 1`, as the *node* hash maps store one pointer plus one byte of metadata for each entry in the bucket array.
- flat tables store the values, plus one byte of metadata per value), directly into the bucket array, hence the `sizeof(C::value_type) + 1`.
- the additional peak memory usage (when resizing) corresponds the the old bucket array (half the size of the new one, hence the 0.5), which contains the values to be copied to the new bucket array, and which is freed when the values have been copied.
- the *parallel* hashmaps, when created with a template parameter N=4, create 16 submaps. When the hash values are well distributed, and in single threaded mode, only one of these 16 submaps resizes at any given time, hence the factor `0.03` roughly equal to `0.5 / 16`

## Iterator invalidation for hash containers

The rules are the same as for `std::unordered_map`, and are valid for all the phmap hash containers:


|    Operations	                            | Invalidated                |
|-------------------------------------------|----------------------------|
| All read only operations, swap, std::swap | Never                      |
| clear, rehash, reserve, operator=         | Always                     |
| insert, emplace, emplace_hint, operator[] | Only if rehash triggered   |
| erase                                     | Only to the element erased |

## Iterator invalidation for btree containers

Unlike for `std::map` and `std::set`, any mutating operation may invalidate existing iterators to btree containers.


|    Operations	                            | Invalidated                |
|-------------------------------------------|----------------------------|
| All read only operations, swap, std::swap | Never                      |
| clear, operator=                          | Always                     |
| insert, emplace, emplace_hint, operator[] | Yes                        |
| erase                                     | Yes                        |

## Example 2 - providing a hash function for a user-defined class

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

Parallel Hashmap containers follow the thread safety rules of the Standard C++ library. In Particular:

- A single phmap hash table is thread safe for reading from multiple threads. For example, given a hash table A, it is safe to read A from thread 1 and from thread 2 simultaneously.

- If a single hash table is being written to by one thread, then all reads and writes to that hash table on the same or other threads must be protected. For example, given a hash table A, if thread 1 is writing to A, then thread 2 must be prevented from reading from or writing to A. 

- It is safe to read and write to one instance of a type even if another thread is reading or writing to a different instance of the same type. For example, given hash tables A and B of the same type, it is safe if A is being written in thread 1 and B is being read in thread 2.

- The *parallel* tables can be made internally thread-safe for concurrent read and write access, by providing a synchronization type (for example [std::mutex](https://en.cppreference.com/w/cpp/thread/mutex)) as the last template argument. Because locking is performed at the *submap* level, a high level of concurrency can still be achieved. Read access can be done safely using `if_contains()`, which passes a reference value to the callback while holding the *submap* lock. Similarly, write access can be done safely using `modify_if`, `try_emplace_l` or `lazy_emplace_l`. However, please be aware that iterators or references returned by standard APIs are not protected by the mutex, so they cannot be used reliably on a hash map which can be changed by another thread.

- Examples on how to use various mutex types, including boost::mutex, boost::shared_mutex and absl::Mutex can be found in `examples/bench.cpp`


## Acknowledgements

Many thanks to the Abseil developers for implementing the swiss table and btree data structures (see [abseil-cpp](https://github.com/abseil/abseil-cpp)) upon which this work is based, and to Google for releasing it as open-source. 
