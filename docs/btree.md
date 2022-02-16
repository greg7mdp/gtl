
<img src="https://github.com/greg7mdp/gtl/blob/main/html/img/phash.png?raw=true" width="120" align="middle"> 

# Btree containers provided by [gtl](https://github.com/greg7mdp/gtl)

> The btree containers provided in  [gtl](https://github.com/greg7mdp/gtl) are derived from those from Abseil, and should behave similarly as the Abseil ones, modulo small differences (such as supporting std::string_view instead of absl::string_view, and being forward declarable). 

> **IMPORTANT:** This repository borrows code from the [abseil-cpp](https://github.com/abseil/abseil-cpp) repository, with modifications, and may behave differently from the original. This repository is an independent work, with no guarantees implied or provided by the authors. Please visit [abseil-cpp](https://github.com/abseil/abseil-cpp) for the official Abseil libraries.

When btrees are mutated, values stored within can be moved in memory. This means that pointers or iterators to values stored in btree containers can be invalidated when that btree is modified. This is a significant difference with `std::map` and `std::set`, as the std containers do offer a guarantee of pointer stability. The same is true for the 'flat' hash maps and sets.

The full types with template parameters can be found in the [gtl/phmap_fwd_decl.hpp](https://raw.githubusercontent.com/greg7mdp/gtl/main/gtl/phmap_fwd_decl.hpp) header, which is useful for forward declaring the Parallel Hashmaps when necessary.



## Iterator invalidation for btree containers

Unlike for `std::map` and `std::set`, any mutating operation may invalidate existing iterators to btree containers.


|    Operations	                            | Invalidated                |
|-------------------------------------------|----------------------------|
| All read only operations, swap, std::swap | Never                      |
| clear, operator=                          | Always                     |
| insert, emplace, emplace_hint, operator[] | Yes                        |
| erase                                     | Yes                        |


## Thread safety

[Gtl](https://github.com/greg7mdp/gtl) btree containers follow the thread safety rules of the Standard C++ library. In Particular:

- A single btree container is thread safe for reading from multiple threads. For example, given a btree container A, it is safe to read A from thread 1 and from thread 2 simultaneously.

- If a single btree container is being written to by one thread, then all reads and writes to that btree container on the same or other threads must be protected. For example, given a btree container A, if thread 1 is writing to A, then thread 2 must be prevented from reading from or writing to A. 

- It is safe to read and write to one instance of a type even if another thread is reading or writing to a different instance of the same type. For example, given btree containers A and B of the same type, it is safe if A is being written in thread 1 and B is being read in thread 2.


## Acknowledgements

Many thanks to the Abseil developers for implementing the btree containers (see [abseil-cpp](https://github.com/abseil/abseil-cpp)) upon which this work is based, and to Google for releasing it as open-source. 
