
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

- The `flat` hash containers do not provide pointer stability. This means that when the container resizes, it will move the keys and values in memory. So pointers to something inside a `flat` hash container will become invalid when the container is resized. The `node` hash containers do provide pointer stability, and should be used instead if this is an issue.

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


## Bit vector (or dynamic bitset)

[Gtl](https://github.com/greg7mdp/gtl) provides `bit_vector`, which is an alternative to `std::vector<bool>` and `std::bitset`, as it provides both dynamic resizing, and a good numberassortment of bit manipulation primitives.

I implemented this container because i often needed the functionality it provides, and didn't find an open-source implementation I like which didn't require pulling a big library. The `gtl::bit_vector` implementation is self-contained in a single header file (`gtl/bit_vector.hpp`) which can trivially  be added to any project. In addition, I wished to have the `gtl::bit_view` functionality, similar to `std::string_view` for strings, to refer and operate on a subset of a full `gtl::bit_vector`.

Here is an example demonstrating some of the capabilities of `gtl::bit_vector`:

```
#include <gtl/bit_vector.hpp>

using std::string;

int main() {
    {
        // allocate small bit_vector on the stack, do very basic operations on it
        // ----------------------------------------------------------------------
        gtl::bit_vector bv(16);
        assert((string)bv == "0x0000");  // by default, initialized to 0 (false).
        assert(bv[0] == false);          // get value of bit 0

        bv.set(0);
        assert(bv[0]);
        assert((string)bv == "0x0001");

        bv.flip();                       // flip all bits
        assert((string)bv == "0xfffe");

        bv.reset();                      // reset all the bits
        assert((string)bv == "0x0000");

        bv.set(0).set(4).set(8);         // calls can often be chained 
        assert((string)bv == "0x0111");

        // quick look at views... more on that later
        bv.reset();                      // reset all the bits
        bv.view(0, 4)  = 0x1;
        bv.view(4, 8)  = 0x2;
        bv.view(8, 12) = 0x3;
        assert((string)bv == "0x0321");
    }

    {
        // We can initialize with std::initializer_list<uint64_t>
        // each value adds 64 bit to the bit_vector
        // ------------------------------------------------------
        gtl::bit_vector bv { 0x0321 }; 
        assert(bv.size() == 64);
        assert((string)bv == "0x0000000000000321"); // when output, read bit vector right to left
                                                    // for bv[0] is the rightmost

        // shift operators
        // ---------------
        bv >>= 4;
        assert((string)bv == "0x0000000000003210");
        bv >>= 12;
        assert((string)bv == "0x0000000003210000");
        bv <<= 16;
        assert((string)bv == "0x0000000000000321"); // back to the original value

        // assigning an initializer_list work as well
        // ------------------------------------------
        bv = { 0xffffffff00000000 };
        assert((string)bv == "0xffffffff00000000");

        gtl::bit_vector  bv2 { 0x00000000ffffffff };
        assert((string)bv2 == "0x00000000ffffffff");

        // let's try bitwise operators
        // ---------------------------
        assert((bv & bv2).none());                  // binary and => no one bit remains
        assert((string)(bv & bv2) == "0x0000000000000000");
        assert((bv & bv2).count() == 0);

        assert((bv | bv2).every());                 // binary or => all bits are set
        assert((string)(bv | bv2) == "0xffffffffffffffff");
        assert((bv | bv2).count() == 64);

        assert((bv ^ bv2).every());                 // binary xor => all bits are set

        // there are more operators |=, &=, ^=, ==) and methods like
        // contains(), disjoint(), 

    }

    {
        // we also have find_first() and find_next(), allowing to iterate over set bits
        // ----------------------------------------------------------------------------
        gtl::bit_vector bv { 0, 0xf00, 0x0321 }; 
        assert(bv.size() == 192);
        assert((string)bv == "0x00000000000003210000000000000f000000000000000000");

        assert(bv.find_first() == 72);              // find_first is equivalent to std::countr_zero
        assert(bv.find_next(73) == 73);
        assert(bv.find_next(77) == 128);
    }
        

    {
        // views are very powerful. You can create a view on a part of the bitmap
        // and then query and change the bits on the view by themselves. 
        // ----------------------------------------------------------------------
        gtl::bit_vector bv { 0x0321 }; 
        assert((string)bv == "0x0000000000000321");

        bv.view(0, 4) = 0xf;                         // create a view on bv, and assign a value to its bits
        assert((string)bv == "0x000000000000032f");  // modifies underlying bit_vector

        bv.view(4, 12) = 0xde;
        assert((string)bv == "0x0000000000000def");

        bv.view(60, 64) = 0x7;
        assert((string)bv == "0x7000000000000def");

        bv.view(4, 20) >>= 8;                        // you can bit-shift a view, changing only the "viewed" bits
        assert((string)bv == "0x70000000000de00f");

        bv.view(4, 12) = bv.view(12, 20);            // or assign a view to another one - they have to be the same size
        assert((string)bv == "0x70000000000dedef");

        assert(bv.view( 0,  4).count() == 4);        // count set bits in a view
        assert(bv.view(32, 64).count() == 3);

        bv.view(56, 60) |= bv.view(60, 64);          // or a view with the content of another one
        assert((string)bv == "0x77000000000dedef");

        // anything you can do on a gtl::bit_vector also works on a gtl::bit_view. 
        gtl::bit_view view(bv.view(4, 12));
        assert((string)view == "0xde");              // it can also be converted to a string or output on a stream

    }

    return 0;
}

```

