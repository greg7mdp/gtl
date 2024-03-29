#include "btree_fwd.hpp"
#include <gtl/btree.hpp>
#include <iostream>

int main() {
    // initialise map with some values using an initializer_list
    gtl::btree_map<std::string, int> map = {
        {"John",  35},
        { "Jane", 32},
        { "Joe",  30},
    };

    // add a couple more values using operator[]()
    map["lucy"]  = 18;
    map["Andre"] = 20;

    auto it = map.find("Joe");
    map.erase(it);

    map.insert(std::make_pair("Alex", 16));
    map.emplace("Emily", 18); // emplace uses pair template constructor

    for (auto& p : map)
        std::cout << p.first << ", " << p.second << '\n';

    IntString map2; // IntString is declared in btree_fwd.hpp

    map2.emplace(std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple(10, 'c'));
    map2.try_emplace(1, 10, 'a'); // gtl::btree_map supports c++17 API

    for (auto& p : map2)
        std::cout << p.first << ", " << p.second << '\n';

    // create a btree_set of tuples
    using X = std::tuple<float, std::string>;
    gtl::btree_set<X> set;

    for (int i = 0; i < 10; ++i)
        set.insert(X((float)i, std::to_string(i)));
    set.emplace(15.0f, "15");

    X x((float)1, std::to_string(1));
    set.erase(x);

    for (auto& e : set)
        std::cout << std::get<0>(e) << ", \"" << std::get<1>(e) << "\" \n";

    return 0;
}
