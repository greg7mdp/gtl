#include <vector>

#include "gtest/gtest.h"

#include "gtl/phmap_dump.hpp"

namespace gtl {
namespace priv {
namespace {

TEST(DumpLoad, FlatHashSet_uin32)
{
    gtl::flat_hash_set<uint32_t> st1 = { 1991, 1202 };

    {
        gtl::BinaryOutputArchive ar_out("./dump.data");
        EXPECT_TRUE(st1.phmap_dump(ar_out));
    }

    gtl::flat_hash_set<uint32_t> st2;
    {
        gtl::BinaryInputArchive ar_in("./dump.data");
        EXPECT_TRUE(st2.phmap_load(ar_in));
    }
    EXPECT_TRUE(st1 == st2);
}

TEST(DumpLoad, FlatHashMap_uint64_uint32)
{
    gtl::flat_hash_map<uint64_t, uint32_t> mp1 = {
        {78731,  99 },
        { 13141, 299},
        { 2651,  101}
    };

    {
        gtl::BinaryOutputArchive ar_out("./dump.data");
        EXPECT_TRUE(mp1.phmap_dump(ar_out));
    }

    gtl::flat_hash_map<uint64_t, uint32_t> mp2;
    {
        gtl::BinaryInputArchive ar_in("./dump.data");
        EXPECT_TRUE(mp2.phmap_load(ar_in));
    }

    EXPECT_TRUE(mp1 == mp2);
}

TEST(DumpLoad, ParallelFlatHashMap_uint64_uint32)
{
    gtl::parallel_flat_hash_map<uint64_t, uint32_t> mp1 = {
        {99,   299 },
        { 992, 2991},
        { 299, 1299}
    };

    {
        gtl::BinaryOutputArchive ar_out("./dump.data");
        EXPECT_TRUE(mp1.phmap_dump(ar_out));
    }

    gtl::parallel_flat_hash_map<uint64_t, uint32_t> mp2;
    {
        gtl::BinaryInputArchive ar_in("./dump.data");
        EXPECT_TRUE(mp2.phmap_load(ar_in));
    }
    EXPECT_TRUE(mp1 == mp2);
}

}
}
}
