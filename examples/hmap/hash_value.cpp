#include "hash_value.hpp" // defines Person  with std::hash specialization

#include <gtl/phmap.hpp>
#include <iostream>

int main() {
    // As we have defined a specialization of std::hash() for Person,
    // we can now create sparse_hash_set or sparse_hash_map of Persons
    // ----------------------------------------------------------------
    gtl::flat_hash_set<Person> persons = {
        {"John",  "Mitchell", 35},
        { "Jane", "Smith",    32},
        { "Jane", "Smith",    30},
    };

    for (auto& p : persons)
        std::cout << p._first << ' ' << p._last << " (" << p._age << ")" << '\n';
}
