// ---------------------------------------------------------------------------
// Copyright (c) 2022, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
// ---------------------------------------------------------------------------
#include <gtl/bit_vector.hpp>
#include "gtest/gtest.h"

void set_bits_naive(gtl::bit_vector &v, size_t first, size_t last) {
    for (size_t i=first; i<last; ++i) {
        v.set(i);
        // EXPECT_TRUE(v[i]);
    }
}    

void flip_bits_naive(gtl::bit_vector &v, size_t first, size_t last) {
    for (size_t i=first; i<last; ++i)
        v.flip(i);
}    

void clear_bits_naive(gtl::bit_vector &v, size_t first, size_t last) {
    for (size_t i=first; i<last; ++i) {
        v.clear(i);
        // EXPECT_FALSE(v[i]);
    }
}    

void bv_change_test(gtl::bit_vector &v1, gtl::bit_vector &v2, size_t lg) {
    gtl::bit_vector  v3(v1.size());

    for (size_t i=0; i<130; ++i) {
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
        
        v1.view(i, i + lg).clear();
        clear_bits_naive(v2, i, i + lg);
        EXPECT_TRUE(v1 == v2);
        EXPECT_TRUE(v1 == v3);

        v1.view(i, i + lg).set().flip();
        EXPECT_TRUE(v1 == v3);
    }    
}

std::vector<gtl::bit_vector>& get_test_vector() {
    static std::vector<gtl::bit_vector> res;
    if (res.empty()) {
        res.push_back(gtl::bit_vector(0));
        res.push_back(gtl::bit_vector(1, true));
        res.push_back(gtl::bit_vector(1, false));

        {
            // generate some vectors of size 2
            // -------------------------------
            gtl::bit_vector v(2);
            for (uint64_t i=0; i<4; ++i) {
                v = { i };
                res.push_back(v);
            }
        }

        {
            // generate some vectors of size 17
            // -------------------------------
            gtl::bit_vector v(17);
            for (uint64_t i=0; i<4; ++i) {
                v = { i<<3 };
                v.set(i+11);
                res.push_back(v);
            }
        }

        {
            // generate some vectors of size 307
            // ---------------------------------
            gtl::bit_vector v(307);
            for (uint64_t i=3; i<9; ++i) {
                uint64_t root = (i<<(2*i)) * 127;
                v = { root  + root*7,  (i<<(27-i)) + (i<<(27+i)), 0x0f1f1f1f00aaaa*7, 0x0af000000000000*29, 41*i*i  };
                v.set(i+11);
                res.push_back(v);
            }
        }
        
        {
            // generate some vectors of size 256
            // ---------------------------------
            gtl::bit_vector v(256, false);
            v.view(177,227).set();
            res.push_back(v);

            v.flip();
            res.push_back(v);
        }
    }
    return res;
}
        

TEST(BitVectorTest, resize) {
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

TEST(BitVectorTest, bit_view_change) {
    static constexpr size_t sz = 500;
    gtl::bit_vector v1(sz), v2(sz);
    
    bv_change_test(v1, v2, 11);
    bv_change_test(v1, v2, 69);
    bv_change_test(v1, v2, 155);
    bv_change_test(v1, v2, 277);
    bv_change_test(v1, v2, sz - 130);
}

TEST(BitVectorTest, bitwise_op_on_bv) {
    {
        static constexpr size_t sz = 500;
        gtl::bit_vector v1(sz), v2(sz), v3(sz);
        v1.set();
        for (size_t i=0; i<117; i+= 30)
            v1.view(i, i+11).clear();
        v1.view(200, 400).clear();

        v2.clear();
        for (size_t i=0; i<117; i+= 30)
            v2.view(i, i+11).set();
        v2.view(200, 400).set();
        EXPECT_TRUE(~v1 == v2);
    }

    std::vector<gtl::bit_vector>& testv = get_test_vector();
    for (auto v1 : testv) {
        auto v2 = v1;
        EXPECT_TRUE((~v2 ^ v1).every());
        EXPECT_TRUE((v2 ^ v1).none());
        EXPECT_TRUE((v2 - v1).none());
        auto v3 = v2 & v1;
        EXPECT_TRUE((v2 & v1) == v1);
    }
}

TEST(BitVectorTest, bitwise_assign_op_on_bv) {
} 

TEST(BitVectorTest, shift_operators_on_bv) {
}

TEST(BitVectorTest, unary_predicates_on_bv) {
}

TEST(BitVectorTest, binary_predicates_on_bv) {
}

TEST(BitVectorTest, popcount_on_bv) {
}

TEST(BitVectorTest, print) {
    static constexpr size_t sz = 100;
    gtl::bit_vector v(sz);
    for (size_t i=0; i<sz; i+= 4)
        v.view(i, i+4).set_uint64((i/4) % 16); // set to hex 0 -> f

    EXPECT_TRUE(v.print() == "0x0876543210fedcba9876543210");
    if (0) {
        // print test bit_vectors
        // ----------------------
        auto& testv = get_test_vector();
        for (size_t i=0; i<testv.size(); ++i)
            std::cout << testv[i] << '\n';
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}
