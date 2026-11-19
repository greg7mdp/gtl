#include "gtest/gtest.h"
#include <gtl/vec_utils.hpp>

#include <vector>

namespace {

using ivec = std::vector<int>;

const ivec l{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

} // namespace

TEST(VecUtilsTest, SliceWholeRange) {
    EXPECT_EQ(gtl::slice(l), l);
    EXPECT_EQ(gtl::slice(l, 0, -1, 1), l);
    EXPECT_EQ(gtl::slice(l, 2, 5), ivec({ 3, 4, 5 }));
    EXPECT_EQ(gtl::slice(l, -4, -2), ivec({ 7, 8 }));
}

TEST(VecUtilsTest, SliceStrideDividesRange) {
    EXPECT_EQ(gtl::slice(l, 3, 9, 2), ivec({ 4, 6, 8 }));
    EXPECT_EQ(gtl::slice(l, 0, 9, 3), ivec({ 1, 4, 7 }));
}

TEST(VecUtilsTest, SliceStrideKeepsLastElement) {
    // like python's l[0::2] and l[1::3]: the last element in range is kept
    // even when the range length is not a multiple of the stride
    EXPECT_EQ(gtl::slice(l, 0, -1, 2), ivec({ 1, 3, 5, 7, 9 }));
    EXPECT_EQ(gtl::slice(l, 1, -1, 3), ivec({ 2, 5, 8 }));
    EXPECT_EQ(gtl::slice(l, 0, 8, 3), ivec({ 1, 4, 7 }));
    EXPECT_EQ(gtl::slice(l, -6, -1, 2), ivec({ 5, 7, 9 }));
    EXPECT_EQ(gtl::slice(l, 0, -1, 4), ivec({ 1, 5, 9 }));
}

TEST(VecUtilsTest, SliceStrideLargerThanRange) {
    EXPECT_EQ(gtl::slice(l, 2, 4, 5), ivec({ 3 }));
    EXPECT_EQ(gtl::slice(l, 0, -1, 100), ivec({ 1 }));
}

TEST(VecUtilsTest, SliceEmptyRange) {
    EXPECT_TRUE(gtl::slice(l, 4, 4).empty());
    EXPECT_TRUE(gtl::slice(l, 5, 2).empty());
    EXPECT_TRUE(gtl::slice(ivec{}).empty());
}
