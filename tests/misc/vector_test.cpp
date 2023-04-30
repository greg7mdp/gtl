// ---------------------------------------------------------------------------
// Copyright (c) 2023, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Derived from Folly open source library (https://github.com/facebook/folly)
// with modifications by Gregory Popovitch.
// Folly license redistributed in the "licenses" directory of this repository
//
// original author:  andrei.alexandrescu@fb.com
// ---------------------------------------------------------------------------
#include "gtest/gtest.h"
#include <gtl/vector.hpp>

#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <cstdlib>
#include <random>

using namespace std;
using namespace gtl;

inline uint32_t randomNumberSeed() { return rand(); }

auto static const seed = randomNumberSeed();
using RandomT          = std::mt19937;
static RandomT rng;

template<class Integral1, class Integral2>
Integral2 random(Integral1 low, Integral2 up)
{
    std::uniform_int_distribution<unsigned long long> range(low, up);
    return static_cast<Integral2>(range(rng));
}

template<class String>
void randomString(String* toFill, unsigned int maxSize = 1000)
{
    assert(toFill);
    toFill->resize(random(0, maxSize));
    for (auto& c : *toFill) {
        c = random('a', 'z');
    }
}

template<class String, class Integral>
void Num2String(String& str, Integral /* n */)
{
    str.resize(10, '\0');
    sprintf(&str[0], "%ul", 10);
    str.resize(strlen(str.c_str()));
}

std::list<char> RandomList(unsigned int maxSize)
{
    std::list<char>           lst(random(0u, maxSize));
    std::list<char>::iterator i = lst.begin();
    for (; i != lst.end(); ++i) {
        *i = random('a', 'z');
    }
    return lst;
}

template<class T>
T randomObject();

template<>
int randomObject<int>();

template<>
std::string randomObject<std::string>();

template<>
int randomObject<int>()
{
    return random(0, 1024);
}

template<>
std::string randomObject<std::string>()
{
    std::string result;
    randomString(&result);
    return result;
}

#define FOR_EACH_RANGE(i, begin, end) for (auto i = (begin); i < (end); ++i)

using IntVector    = gtl::vector<int>;
using StringVector = gtl::vector<std::string>;

#define VECTOR IntVector
#include "./vector_test.cpp.h" // nolint
#undef VECTOR
#define VECTOR StringVector
#include "./vector_test.cpp.h" // nolint
#undef VECTOR

TEST(vector, clause_23_3_6_1_3_ambiguity)
{
    gtl::vector<int> v(10, 20);
    EXPECT_EQ(v.size(), 10);
    for (const auto& i : v) {
        EXPECT_EQ(i, 20);
    }
}

TEST(vector, clause_23_3_6_1_11_ambiguity)
{
    gtl::vector<int> v;
    v.assign(10, 20);
    EXPECT_EQ(v.size(), 10);
    for (const auto& i : v) {
        EXPECT_EQ(i, 20);
    }
}

TEST(vector, clause_23_3_6_2_6)
{
    gtl::vector<int> v;
    auto const       n = random(0U, 10000U);
    v.reserve(n);
    auto const n1  = random(0U, 10000U);
    auto const obj = randomObject<int>();
    v.assign(n1, obj);
    v.shrink_to_fit();
    // Nothing to verify except that the call made it through
}

TEST(vector, clause_23_3_6_4_ambiguity)
{
    gtl::vector<int>                 v;
    gtl::vector<int>::const_iterator it = v.end();
    v.insert(it, 10, 20);
    EXPECT_EQ(v.size(), 10);
    for (auto i : v) {
        EXPECT_EQ(i, 20);
    }
}

TEST(vector, composition)
{
    gtl::vector<gtl::vector<double>> matrix(100, gtl::vector<double>(100));
}

TEST(vector, works_with_std_string)
{
    gtl::vector<std::string> v(10, "hello");
    EXPECT_EQ(v.size(), 10);
    v.push_back("world");
}

namespace {
class UserDefinedType
{
public:
    int whatevs_;
};
} // namespace

TEST(vector, works_with_user_defined_type)
{
    gtl::vector<UserDefinedType> v(10);
    EXPECT_EQ(v.size(), 10);
    v.push_back(UserDefinedType());
}

TEST(vector, move_construction)
{
    gtl::vector<int> v1(100, 100);
    gtl::vector<int> v2;
    EXPECT_EQ(v1.size(), 100);
    EXPECT_EQ(v1.front(), 100);
    EXPECT_EQ(v2.size(), 0);
    v2 = std::move(v1);
    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(v2.size(), 100);
    EXPECT_EQ(v2.front(), 100);

    v1.assign(100, 100);
    auto other = std::move(v1);
    EXPECT_EQ(v1.size(), 0);
    EXPECT_EQ(other.size(), 100);
    EXPECT_EQ(other.front(), 100);
}

TEST(vector, emplace)
{
    gtl::vector<std::string> s(12, "asd");
    EXPECT_EQ(s.size(), 12);
    EXPECT_EQ(s.front(), "asd");
    const auto& emplaced = s.emplace_back("funk");
    EXPECT_EQ(emplaced, "funk");
    EXPECT_EQ(s.back(), "funk");
    EXPECT_EQ(std::addressof(emplaced), std::addressof(s.back()));
}

TEST(vector, initializer_lists)
{
    gtl::vector<int> vec = { 1, 2, 3 };
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);

    vec = { 0, 0, 12, 16 };
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 0);
    EXPECT_EQ(vec[2], 12);
    EXPECT_EQ(vec[3], 16);

    vec.insert(vec.begin() + 1, { 23, 23 });
    EXPECT_EQ(vec.size(), 6);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 23);
    EXPECT_EQ(vec[2], 23);
    EXPECT_EQ(vec[3], 0);
    EXPECT_EQ(vec[4], 12);
    EXPECT_EQ(vec[5], 16);
}

TEST(vector, unique_ptr)
{
    gtl::vector<std::unique_ptr<int>> v(12);
    std::unique_ptr<int>              p(new int(12));
    v.push_back(std::move(p));
    EXPECT_EQ(*v.back(), 12);

    v[0] = std::move(p);
    EXPECT_FALSE(v[0].get());
    v[0] = std::make_unique<int>(32);
    std::unique_ptr<int> somePtr;
    v.insert(v.begin(), std::move(somePtr));
    EXPECT_EQ(*v[1], 32);
}

TEST(vector, task858056)
{
    gtl::vector<std::string> cycle;
    cycle.push_back("foo");
    cycle.push_back("bar");
    cycle.push_back("baz");
    std::string message("Cycle detected: ");
    for (auto i = cycle.rbegin(); i != cycle.rend(); ++i) {
        message += "[";
        message += *i;
        message += "] ";
    }
    EXPECT_EQ("Cycle detected: [baz] [bar] [foo] ", message);
}

TEST(vector, move_iterator)
{
    gtl::vector<int> base = { 0, 1, 2 };

    auto             cp1 = base;
    gtl::vector<int> fbvi1(std::make_move_iterator(cp1.begin()),
                           std::make_move_iterator(cp1.end()));
    EXPECT_EQ(fbvi1, base);

    auto             cp2 = base;
    gtl::vector<int> fbvi2;
    fbvi2.assign(std::make_move_iterator(cp2.begin()), std::make_move_iterator(cp2.end()));
    EXPECT_EQ(fbvi2, base);

    auto             cp3 = base;
    gtl::vector<int> fbvi3;
    fbvi3.insert(
        fbvi3.end(), std::make_move_iterator(cp3.begin()), std::make_move_iterator(cp3.end()));
    EXPECT_EQ(fbvi3, base);
}

TEST(vector, reserve_consistency)
{
    struct S
    {
        int64_t a, b, c, d;
    };

    gtl::vector<S> fb1;
    for (size_t i = 0; i < 1000; ++i) {
        fb1.reserve(1);
        EXPECT_EQ(fb1.size(), 0);
        fb1.shrink_to_fit();
    }
}

TEST(vector, vector_of_maps)
{
    gtl::vector<std::map<std::string, std::string>> v;

    v.push_back(std::map<std::string, std::string>());
    v.push_back(std::map<std::string, std::string>());

    EXPECT_EQ(2, v.size());

    v[1]["hello"] = "world";
    EXPECT_EQ(0, v[0].size());
    EXPECT_EQ(1, v[1].size());

    v[0]["foo"] = "bar";
    EXPECT_EQ(1, v[0].size());
    EXPECT_EQ(1, v[1].size());
}

TEST(vector, shrink_to_fit_after_clear)
{
    gtl::vector<int> fb1;
    fb1.push_back(42);
    fb1.push_back(1337);
    fb1.clear();
    fb1.shrink_to_fit();
    EXPECT_EQ(fb1.size(), 0);
    EXPECT_EQ(fb1.capacity(), 0);
}

TEST(vector, zero_len)
{
    gtl::vector<int> fb1(0);
    gtl::vector<int> fb2(0, 10);
    gtl::vector<int> fb3(std::move(fb1));
    gtl::vector<int> fb4;
    fb4                  = std::move(fb2);
    gtl::vector<int> fb5 = fb3;
    gtl::vector<int> fb6;
    fb6                           = fb4;
    std::initializer_list<int> il = {};
    fb6                           = il;
    gtl::vector<int> fb7(fb6.begin(), fb6.end());
}

#if __cpp_deduction_guides >= 201703
TEST(vector, deduction_guides)
{
    gtl::vector<int> v(3);

    gtl::vector x(v.begin(), v.end());
    EXPECT_TRUE((std::is_same_v<gtl::vector<int>, decltype(x)>));

    gtl::vector y{ v.begin(), v.end() };
    EXPECT_TRUE((std::is_same_v<gtl::vector<gtl::vector<int>::iterator>, decltype(y)>));
}
#endif

TEST(vector, erase)
{
    gtl::vector<int> v(3);
    std::iota(v.begin(), v.end(), 1);
    v.push_back(2);
    erase(v, 2);
    ASSERT_EQ(2u, v.size());
    EXPECT_EQ(1u, v[0]);
    EXPECT_EQ(3u, v[1]);
}

TEST(vector, erase_if)
{
    gtl::vector<int> v(6);
    std::iota(v.begin(), v.end(), 1);
    erase_if(v, [](const auto& x) { return x % 2 == 0; });
    ASSERT_EQ(3u, v.size());
    EXPECT_EQ(1u, v[0]);
    EXPECT_EQ(3u, v[1]);
    EXPECT_EQ(5u, v[2]);
}

// gtl extension
TEST(vector, stealing_constructor)
{
    using Alloc = std::allocator<int>;
    using vec_t = gtl::vector<int, Alloc>;
    Alloc alloc;

    int *x;
    if constexpr (std::is_same_v<Alloc, std::allocator<vec_t::value_type>>) // always the case here, but left as an example of what should be done
       x = (int *)gtl::checkedMalloc(4 * sizeof(vec_t::value_type));
    else
       x = std::allocator_traits<Alloc>::allocate(alloc, 4);
    
    for (int i = 0; i < 3; ++i)
        *(x + i) = i;
    vec_t v(std::unique_ptr<int>(x), 3, 4);
    ASSERT_EQ(3u, v.size());
    EXPECT_EQ(0u, v[0]);
    EXPECT_EQ(1u, v[1]);
    EXPECT_EQ(2u, v[2]);
}
