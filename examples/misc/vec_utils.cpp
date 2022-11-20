// ---------------------------------------------------------------------------
// Copyright (c) 2022, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
// ---------------------------------------------------------------------------
#include <cassert>
#if __has_include(<version>)
    #include <version>
#endif

#if defined(__cpp_concepts) && defined(__cpp_lib_concepts)

    #include <gtl/vec_utils.hpp>

int main()
{
    using vec = std::vector<char>;

    // vector concatenation
    // --------------------
    const vec a{ 'a', 'b', 'c' };
    const vec b{ 'd' };
    const vec c{ 'e', 'f', 'g', 'h' };

    auto res = gtl::cat(a, b, c);
    res[0]   = 'l';

    // slice
    // -----
    const std::vector<int> l{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    assert(gtl::slice(l) == l);
    assert(gtl::slice(l, 0, -1, 1) == l);
    assert(gtl::slice(l, 3, 9, 2) == std::vector<int>({ 4, 6, 8 }));

    assert(gtl::cat(gtl::slice(l, 0, 1), gtl::slice(l, 1, -1)) == l);

    auto v1 = gtl::slice(l, 0, 1);
    v1      = gtl::slice(l, 1, -1);

    v1 = gtl::slice(l, -1, -1);
    v1 = gtl::slice(l, -1, 1);
    v1 = gtl::slice(l, -4, -2);
    v1 = gtl::slice(l, -4, -2);
    v1 = gtl::slice(l, -6, -1, 2);

    // map
    // ---
    auto v2 = gtl::map(
        [v1](const auto& e) -> std::vector<int> {
            return gtl::cat(gtl::slice(v1, 0, 1), std::vector<int>({ e }));
        },
        l);

    assert(gtl::map([](int e) { return e + 1; }, std::vector<int>({ 1, 2, 3 })) ==
           std::vector<int>({ 2, 3, 4 }));

    return 0;
}
#else
int main() { return 0; }
#endif
