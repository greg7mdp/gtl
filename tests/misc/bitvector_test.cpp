// ---------------------------------------------------------------------------
// Copyright (c) 2022, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
// ---------------------------------------------------------------------------
#include "gtest/gtest.h"
#include <gtl/bit_vector.hpp>

void set_bits_naive(gtl::bit_vector& v, size_t first, size_t last)
{
    for (size_t i = first; i < last; ++i) {
        v.set(i);
        // EXPECT_TRUE(v[i]);
    }
}

void flip_bits_naive(gtl::bit_vector& v, size_t first, size_t last)
{
    for (size_t i = first; i < last; ++i)
        v.flip(i);
}

void reset_bits_naive(gtl::bit_vector& v, size_t first, size_t last)
{
    for (size_t i = first; i < last; ++i) {
        v.reset(i);
        // EXPECT_FALSE(v[i]);
    }
}

const std::vector<gtl::bit_vector>& get_test_vector()
{
    static std::vector<gtl::bit_vector> res;
    if (res.empty()) {
        res.push_back(gtl::bit_vector(0));
        res.push_back(gtl::bit_vector(1, true));
        res.push_back(gtl::bit_vector(1, false));

        {
            // generate some vectors of size 2
            // -------------------------------
            gtl::bit_vector v(2);
            for (uint64_t i = 0; i < 4; ++i) {
                v = { i };
                res.push_back(v);
            }
        }

        {
            // generate some vectors of size 17
            // -------------------------------
            gtl::bit_vector v(17);
            for (uint64_t i = 0; i < 4; ++i) {
                v = { i << 3 };
                v.set(static_cast<size_t>(i + 11));
                res.push_back(v);
            }
        }

        {
            // generate some vectors of size 307
            // ---------------------------------
            gtl::bit_vector v(307);
            for (uint64_t i = 3; i < 9; ++i) {
                uint64_t root = (i << (2 * i)) * 127;
                v             = { root + root * 7,
                                  (i << (27 - i)) + (i << (27 + i)),
                                  0x0f1f1f1f00aaaa * 7,
                                  0x0af000000000000 * 29,
                                  41 * i * i };
                v.set(static_cast<size_t>(i + 11));
                res.push_back(v);
            }
        }

        {
            // generate some vectors of size 256
            // ---------------------------------
            for (size_t i = 27; i < 36; ++i) {
                gtl::bit_vector v(256, false);
                v.view(117 + i, 237 - i).set();
                v.view(i, i + 2).set();
                res.push_back(v);

                v.flip();
                res.push_back(v);
            }
        }
    }
    return res;
}

TEST(BitVectorTest, resize)
{
    gtl::bit_vector v1(128), v2(100);
    v1.set();
    v2.set();
    v1.resize(100);
    EXPECT_TRUE(v1 == v2);

    v1.resize(128);
    v1.flip();
    v2.flip();
    v1.resize(100);
    EXPECT_TRUE(v1 == v2);

    v1.resize(128);
    v1.flip();
    v2.flip();
    v1.resize(100);
    EXPECT_TRUE(v1 == v2);
}

TEST(BitVectorTest, bit_view_change)
{
    static constexpr size_t sz = 500;
    gtl::bit_vector         tv1(sz), tv2(sz);

    auto bv_change_test = [](gtl::bit_vector& v1, gtl::bit_vector& v2, size_t lg) {
        gtl::bit_vector v3(v1.size());

        for (size_t i = 0; i < 130; ++i) {
            v1.view(i, i + lg).set();
            set_bits_naive(v2, i, i + lg);
            EXPECT_TRUE(v1 == v2);

            v1.view(i, i + lg).flip();
            flip_bits_naive(v2, i, i + lg);
            EXPECT_TRUE(v1 == v2);
            EXPECT_TRUE(v1 == v3);

            v1.view(i, i + lg).set();
            set_bits_naive(v2, i, i + lg);
            EXPECT_TRUE(v1 == v2);

            v1.view(i, i + lg).reset();
            reset_bits_naive(v2, i, i + lg);
            EXPECT_TRUE(v1 == v2);
            EXPECT_TRUE(v1 == v3);

            v1.view(i, i + lg).set().flip();
            EXPECT_TRUE(v1 == v3);
        }
    };

    bv_change_test(tv1, tv2, 11);
    bv_change_test(tv1, tv2, 69);
    bv_change_test(tv1, tv2, 155);
    bv_change_test(tv1, tv2, 277);
    bv_change_test(tv1, tv2, sz - 130);
}

TEST(BitVectorTest, bitwise_op_on_bv)
{
    {
        static constexpr size_t sz = 500;
        gtl::bit_vector         v1(sz), v2(sz), v3(sz);
        v1.set();
        for (size_t i = 0; i < 117; i += 30)
            v1.view(i, i + 11).reset();
        v1.view(200, 400).reset();

        v2.reset();
        for (size_t i = 0; i < 117; i += 30)
            v2.view(i, i + 11).set();
        v2.view(200, 400).set();
        EXPECT_TRUE(~v1 == v2);
    }

    const std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v1 : testv) {
        auto v2 = v1;
        EXPECT_TRUE((~v2 ^ v1).every());
        EXPECT_TRUE((v2 ^ v1).none());
        EXPECT_TRUE((v2 - v1).none());
        EXPECT_TRUE((v2 & v1) == v1);
        EXPECT_TRUE(~(~v1) == v1);
        for (auto v3 : testv) {
            if (v3.size() != v1.size())
                continue;
            EXPECT_TRUE(~(v3 | v1) == (~v3 & ~v1));
            EXPECT_TRUE(~(v3 & v1) == (~v3 | ~v1));
        }
    }
}

TEST(BitVectorTest, bitwise_assign_op_on_full_bit_vector)
{
    const std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v1 : testv) {
        auto v2 = v1;
        EXPECT_TRUE((~(v2 ^= v1)).every());

        v2 = v1;
        EXPECT_TRUE((v2 ^= v1).none());

        v2 = v1;
        EXPECT_TRUE((v2 -= v1).none());

        v2 = v1;
        EXPECT_TRUE((v2 &= v1) == v1);
    }
}

TEST(BitVectorTest, bit_shift)
{
    auto check = [](const gtl::bit_vector& v_orig,
                    const gtl::bit_vector& v2,
                    int                    i_shift,
                    size_t                 first,
                    size_t                 last) {
        size_t shift = (i_shift >= 0) ? (size_t)i_shift : (size_t)-i_shift;
        last         = std::min(last, v2.size());
        if (shift <= last - first) {
            if (i_shift >= 0) {
                EXPECT_TRUE(v2.view(first, first + shift).none());
                EXPECT_TRUE(v2.view(first + shift, last) == v_orig.view(first, last - shift));
            } else {
                EXPECT_TRUE(v2.view(last - shift, last).none());
                EXPECT_TRUE(v2.view(first, last - shift) == v_orig.view(first + shift, last));
            }
        }
    };

    auto bitshift_check = [&](const gtl::bit_vector& v_orig,
                              int                    shift,
                              size_t                 first = 0,
                              size_t                 last  = gtl::bit_vector::npos) {
        if (last == gtl::bit_vector::npos)
            last = v_orig.size();
        if (first == 0 && last == v_orig.size()) {
            gtl::bit_vector v = (shift >= 0) ? (v_orig >> (size_t)shift)
                                             : (v_orig << (size_t)-shift);
            check(v_orig, v, shift, first, last);
        } else {
            gtl::bit_vector v(v_orig);
            if (shift >= 0)
                v.view(first, last) >>= (size_t)shift;
            else
                v.view(first, last) <<= (size_t)-shift;
            check(v_orig, v, shift, first, last);
        }
    };

    auto check_range = [&](const gtl::bit_vector& v, int shift, size_t width) {
        if (v.size() > width) {
            for (size_t i = 0; i < v.size() - width; ++i) {
                bitshift_check(v, shift, i, i + width);
                bitshift_check(v, -shift, i, i + width);
            }
        }
    };

    const std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v : testv) {
        // only right shift supported for now

        // check fill bit_vector shift
        for (size_t i = 0; i < v.size(); ++i)
            bitshift_check(v, (int)i);

        // check gtl::bit_view shift
        check_range(v, 3, 17);
        check_range(v, 3, 128);
        check_range(v, 128, 3);
        check_range(v, 17, 33);
        check_range(v, 111, 66);
        check_range(v, 127, 31);
    }
}

TEST(BitVectorTest, view_assignment)
{
    auto check_va = [](const gtl::bit_vector& v2, size_t div, const size_t incr = 3) {
        size_t sz = v2.size();
        if (sz) {
            gtl::bit_vector v(sz, false), v1(sz);
            for (size_t i = 0; i < div; ++i)
                v.view((i * sz) / div, ((i + 1) * sz) / div) =
                    v2.view((i * sz) / div, ((i + 1) * sz) / div);
            EXPECT_TRUE(v == v2);

            for (size_t i = 0; i < sz - 1; ++i) {
                if (sz > i + incr + 2) {
                    v.view(i, i + incr) = v2.view(i + 1, i + 1 + incr);
                    i += incr - 1;
                } else
                    v.view(i, i + 1) = v2.view(i + 1, i + 2);
            }
            v.view(sz - 1, sz) = 0;
            EXPECT_TRUE(v == v2 << 1);

            v = v2;
            for (size_t i = 0; i < sz - 1; ++i) {
                if (sz > i + incr + 2) {
                    v.view(i + 1, i + 1 + incr) = v2.view(i, i + incr);
                    i += incr - 1;
                } else
                    v.view(i + 1, i + 2) = v2.view(i, i + 1);
            }
            v.view(0, 1) = 0;
            EXPECT_TRUE(v == v2 >> 1);

            v  = v2;
            v1 = v2;
            for (size_t i = 0; i < sz - 1; ++i) {
                if (sz > i + incr + 2) {
                    v.view(i + 1, i + 1 + incr) = v.view(i, i + incr);
                    v1.view(i + 1, i + 1 + incr).copy_slow(v1.view(i, i + incr));
                    EXPECT_TRUE(v == v1);
                    i += incr - 1;
                } else {
                    v.view(i + 1, i + 2) = v.view(i, i + 1);
                    v1.view(i + 1, i + 2).copy_slow(v1.view(i, i + 1));
                    EXPECT_TRUE(v == v1);
                }
            }
        }
    };

    const std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v : testv) {
        check_va(v, 3);
        check_va(v, 5, 5);
        check_va(v, 7);
        check_va(v, 9, 11);
        check_va(v, 17);
    }
}

TEST(BitVectorTest, unary_predicates_on_full_bit_vector)
{
    auto check_sz = [](size_t sz) {
        gtl::bit_vector v(sz);

        for (size_t i = 0; i < sz; ++i) {
            EXPECT_TRUE(v.none());
            v.set(i);
            EXPECT_FALSE(v.none());
            v.view().set();
            EXPECT_TRUE(v.every());
            v.reset(i);
            EXPECT_FALSE(v.every());
            v.view().reset();
        }
    };

    check_sz(199);
}

TEST(BitVectorTest, binary_predicates_on_full_bit_vector)
{
    const std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v1 : testv) {
        auto v1_copy = v1;
        EXPECT_TRUE(v1 == v1_copy);
        for (auto v2 : testv) {
            if (v2.size() != v1.size())
                continue;
            EXPECT_TRUE(v1.contains(v2) || ((v1 | v2) != v1));

            if (v1.contains(v2) && v2.contains(v1)) {
                EXPECT_TRUE(v1 == v2);
            }

            if (v1.disjoint(v2)) {
                EXPECT_TRUE((v1 & v2).none());
            }
        }
    }
}

TEST(BitVectorTest, count)
{
    auto count_naive =
        [](const gtl::bit_vector& v, size_t first = 0, size_t last = gtl::bit_vector::npos) {
            size_t n = 0;
            if (last == gtl::bit_vector::npos)
                last = v.size();
            for (size_t i = first; i < last; ++i)
                if (v[i])
                    ++n;
            return n;
        };

    const std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v : testv) {
        EXPECT_TRUE(v.count() == count_naive(v));
        for (size_t i = 0; i < v.size(); ++i) {
            v.set(i);
            EXPECT_TRUE(v.count() == count_naive(v));
            if (i % 2)
                v.reset(i);
        }
    }

    // test view count
    for (auto v : testv) {
        size_t last = v.size();
        for (size_t i = 0; i < last; ++i)
            EXPECT_TRUE(v.view(i, last).count() == count_naive(v, i, last));
        if (last) {
            for (size_t i = last - 1; i != 0; --i)
                EXPECT_TRUE(v.view(0, i).count() == count_naive(v, 0, i));
        }
    }
}

TEST(BitVectorTest, find_first)
{
    gtl::bit_vector v{ 0, 0, 0x020202 };
    EXPECT_TRUE(v.find_first() == 129);
    EXPECT_TRUE(v.view(10).find_first() == 119); // returns the index from start of view
    EXPECT_TRUE(v.view(33).find_first() == 96);
    EXPECT_TRUE(v.find_next(130) == 137);
    EXPECT_TRUE(v.find_next(138) == 145);

    gtl::bit_vector v2{ 0, 0, 0, 0, 0x020202 };
    EXPECT_TRUE(v2.find_first() == 257);
    EXPECT_TRUE(v2.view(67).find_first() == 190);
    EXPECT_TRUE(v2.view(67).find_next(191) == 198);
}

TEST(BitVectorTest, hash)
{
    gtl::bit_vector v{ 0xfafafaull, 0ull, 0x4444444444444444ull };
    auto            x = std::hash<gtl::bit_vector>()(v);

    v.set(66);
    auto y = std::hash<gtl::bit_vector>()(v);
    EXPECT_TRUE(x != y);

    v.set(191);
    auto z = std::hash<gtl::bit_vector>()(v);
    EXPECT_TRUE(z != y);
}

TEST(BitVectorTest, conversions)
{
    static constexpr size_t sz = 100;
    gtl::bit_vector         v(sz);
    for (size_t i = 0; i < sz; i += 4)
        v.view(i, i + 4) = uint64_t((i / 4) % 16); // set to hex 0 -> f

    EXPECT_TRUE((std::string)v == "0x0876543210fedcba9876543210");

    {
        // to_string
        gtl::bit_vector x(8);
        x = { 42 };
        EXPECT_TRUE(x.to_string() == "00101010");
        EXPECT_TRUE(x.to_string('*') == "**1*1*1*");
    }

    {
        // to_ullong
        gtl::bit_vector x{ 0x123456789abcdef0LL };
        EXPECT_TRUE(x.to_ullong() == 0x123456789abcdef0LL);
    }

    if (0) {
        // print test bit_vectors
        // ----------------------
        auto& testv = get_test_vector();
        for (size_t i = 0; i < testv.size(); ++i)
            std::cout << testv[i] << '\n';
    }
}
