#include <gtl/phmap.hpp>
#include <gtl/btree.hpp>
#include <gtl/vector.hpp>
#include <gtl/bit_vector.hpp>
#include <iostream>
#include <string>

using gtl::flat_hash_map;
using namespace std::string_literals;

template<class C>
void check_map() {
    C c;
    for (size_t i = 0; i < 100; ++i)
        c.emplace(std::to_string(i), i);
    for (size_t i = 1; i < 100; i += 2)
        c.erase(std::to_string(i));
    std::cout << c.size() << " elements\n";
}

template<class C>
void check_set() {
   C c;
    for (size_t i = 0; i < 100; ++i)
        c.emplace(i);
    for (size_t i = 1; i < 100; i += 2)
        c.erase(i);
    std::cout << c.size() << " elements\n";
}

void check_vector() {
   gtl::vector<int> c;
    for (size_t i = 0; i < 200; ++i)
        c.emplace_back(i);
    for (size_t i = 1; i < 100; i++)
       c.erase(c.begin() + 1);
    std::cout << c.size() << " elements\n";
}



int main() {
    // Works just fine
    check_map<gtl::flat_hash_map<std::string, int>>();
    check_set<gtl::flat_hash_set<int>>();
    check_set<gtl::node_hash_set<int>>();

    // Works partially
    check_map<gtl::parallel_flat_hash_map<std::string, int>>();
    check_map<gtl::parallel_node_hash_map<std::string, int>>();

    check_set<gtl::parallel_flat_hash_set<int>>();
    check_set<gtl::parallel_node_hash_set<int>>();

    // Does not work
    check_map<gtl::btree_map<std::string, int>>();
    check_map<gtl::btree_multimap<std::string, int>>();
    check_set<gtl::btree_set<int>>();
    check_set<gtl::btree_multiset<int>>();

    // Works just fine
    check_vector();
    return 0;
}
