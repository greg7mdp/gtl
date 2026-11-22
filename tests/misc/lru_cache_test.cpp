#include "gtest/gtest.h"
#include <gtl/lru_cache.hpp>
#include <gtl/memoize.hpp>

constexpr int CACHETEST1_NUM_OF_RECORDS = 100;
constexpr int CACHETEST1_CACHE_CAPACITY = 50;

namespace {

template<class Cache>
void CheckReferenceArgumentValues(Cache& memoized) {
    int input = 3;
    EXPECT_EQ(9, memoized(input));
    input = 4;

    auto original = memoized.contains(3);
    ASSERT_TRUE(original.has_value());
    EXPECT_EQ(9, *original);
    EXPECT_EQ(16, memoized(input));
    EXPECT_EQ(9, memoized(3));
    EXPECT_EQ(2u, memoized.size());
}

} // namespace

TEST(MemoizeTest, CachesReferenceArgumentValues) {
    auto square = [](const int& value) { return value * value; };
    gtl::memoize<decltype(square), 0> memoized(square);
    CheckReferenceArgumentValues(memoized);

    gtl::mt_memoize<decltype(square), true, 0> recursive(square);
    CheckReferenceArgumentValues(recursive);

    gtl::mt_memoize<decltype(square), false, 0> nonrecursive(square);
    CheckReferenceArgumentValues(nonrecursive);
}

TEST(MemoizeLruTest, CachesReferenceArgumentValues) {
    auto square = [](const int& value) { return value * value; };
    gtl::memoize_lru<decltype(square), 0> memoized(square, 3);
    CheckReferenceArgumentValues(memoized);

    gtl::mt_memoize_lru<decltype(square), 0> concurrent(square, 3);
    CheckReferenceArgumentValues(concurrent);
}

TEST(CacheTest, SimplePut) {
    gtl::lru_cache<int, int> cache;
    cache.insert(7, 777);
    EXPECT_TRUE(cache.exists(7));
    EXPECT_EQ(777, *cache.get(7));
}

TEST(CacheTest, MissingValue) {
    gtl::lru_cache<int, int> cache;
    EXPECT_FALSE(cache.get(7));
}

TEST(MemoizeLruTest, EvictsLeastRecentOnlyAfterCapacity) {
    auto identity = [](int value) { return value; };
    gtl::memoize_lru<decltype(identity), 0> memoized(identity, 3);

    for (int value : {1, 2, 3}) {
        EXPECT_EQ(value, memoized(value));
    }
    EXPECT_EQ(3u, memoized.size());
    EXPECT_EQ(1, memoized.contains(1));

    EXPECT_EQ(1, memoized(1));
    EXPECT_EQ(4, memoized(4));
    EXPECT_EQ(3u, memoized.size());
    EXPECT_FALSE(memoized.contains(2));
    EXPECT_EQ(1, memoized.contains(1));
    EXPECT_EQ(3, memoized.contains(3));
    EXPECT_EQ(4, memoized.contains(4));
}

TEST(CacheTest1, KeepsAllValuesWithinCapacity) {
    gtl::lru_cache<int, int> cache(CACHETEST1_CACHE_CAPACITY);

    for (int i = 0; i < CACHETEST1_NUM_OF_RECORDS; ++i) {
        cache.insert(i, i);
    }

    for (int i = 0; i < CACHETEST1_NUM_OF_RECORDS - CACHETEST1_CACHE_CAPACITY; ++i) {
        EXPECT_FALSE(cache.exists(i));
    }

    for (int i = CACHETEST1_NUM_OF_RECORDS - CACHETEST1_CACHE_CAPACITY; i < CACHETEST1_NUM_OF_RECORDS; ++i) {
        EXPECT_TRUE(cache.exists(i));
        EXPECT_EQ(i, *cache.get(i));
    }

    size_t size = cache.size();
    EXPECT_EQ(static_cast<size_t>(CACHETEST1_CACHE_CAPACITY), size);
}

TEST(CacheTest1, mtKeepsAllValuesWithinCapacity) {
    gtl::mt_lru_cache<int, int> cache(5000); // an approximation

    for (int i = 0; i < 10000; ++i) {
        cache.insert(i, i);
    }

    for (int i = 0; i < 2000; ++i) {
        EXPECT_FALSE(cache.exists(i));
    }

    for (int i = 8000; i < 10000; ++i) {
        EXPECT_TRUE(cache.exists(i));
        EXPECT_EQ(i, *cache.get(i));
    }
}

TEST(CacheTest, BoundaryCapacityOneAndTwo) {
    // Capacity 1
    gtl::lru_cache<int, int> cache1(1);
    cache1.insert(1, 10);
    EXPECT_EQ(1u, cache1.size());
    EXPECT_TRUE(cache1.contains(1));

    cache1.insert(2, 20);
    EXPECT_EQ(1u, cache1.size());
    EXPECT_FALSE(cache1.contains(1));
    EXPECT_TRUE(cache1.contains(2));

    // Capacity 2
    gtl::lru_cache<int, int> cache2(2);
    cache2.insert(1, 10);
    cache2.insert(2, 20);
    EXPECT_EQ(2u, cache2.size());
    EXPECT_TRUE(cache2.contains(1));
    EXPECT_TRUE(cache2.contains(2));

    cache2.insert(3, 30);
    EXPECT_EQ(2u, cache2.size());
    EXPECT_FALSE(cache2.contains(1));
    EXPECT_TRUE(cache2.contains(2));
    EXPECT_TRUE(cache2.contains(3));

    // Access 2 to make it most recently used, so 3 becomes least recently used
    EXPECT_EQ(20, *cache2.get(2));
    cache2.insert(4, 40);
    EXPECT_EQ(2u, cache2.size());
    EXPECT_FALSE(cache2.contains(3));
    EXPECT_TRUE(cache2.contains(2));
    EXPECT_TRUE(cache2.contains(4));
}

TEST(CacheTest, ConstContainsAndExists) {
    gtl::lru_cache<int, int> cache(5);
    cache.insert(42, 100);

    const auto& const_cache = cache;
    EXPECT_TRUE(const_cache.contains(42));
    EXPECT_TRUE(const_cache.exists(42));
    EXPECT_FALSE(const_cache.contains(99));
    EXPECT_FALSE(const_cache.exists(99));
}

TEST(MemoizeLruTest, BoundaryCapacityOneAndTwo) {
    auto sq = [](int x) { return x * x; };
    gtl::memoize_lru<decltype(sq), 0> memoized1(sq, 1);
    EXPECT_EQ(1, memoized1(1));
    EXPECT_EQ(1u, memoized1.size());
    EXPECT_TRUE(memoized1.contains(1).has_value());

    EXPECT_EQ(4, memoized1(2));
    EXPECT_EQ(1u, memoized1.size());
    EXPECT_FALSE(memoized1.contains(1).has_value());
    EXPECT_TRUE(memoized1.contains(2).has_value());

    gtl::memoize_lru<decltype(sq), 0> memoized2(sq, 2);
    EXPECT_EQ(1, memoized2(1));
    EXPECT_EQ(4, memoized2(2));
    EXPECT_EQ(2u, memoized2.size());

    EXPECT_EQ(9, memoized2(3));
    EXPECT_EQ(2u, memoized2.size());
    EXPECT_FALSE(memoized2.contains(1).has_value());
    EXPECT_TRUE(memoized2.contains(2).has_value());
    EXPECT_TRUE(memoized2.contains(3).has_value());
}

TEST(CacheTest, MoveConstructKeepsRecencyLists) {
    gtl::lru_cache<int, int> source(2);
    source.insert(1, 1);
    source.insert(2, 2);

    gtl::lru_cache<int, int> cache(std::move(source));
    EXPECT_EQ(2u, cache.size());
    EXPECT_EQ(1, cache.get(1).value()); // 1 becomes the most recent
    cache.insert(3, 3);                 // so 2 is the one evicted

    EXPECT_EQ(2u, cache.size());
    EXPECT_TRUE(cache.exists(1));
    EXPECT_FALSE(cache.exists(2));
    EXPECT_TRUE(cache.exists(3));
}

TEST(CacheTest, MoveAssignKeepsRecencyLists) {
    gtl::lru_cache<int, int> source(2);
    source.insert(1, 1);
    source.insert(2, 2);

    gtl::lru_cache<int, int> cache(2);
    cache.insert(7, 7);
    cache = std::move(source);
    cache.insert(3, 3); // 1 is the oldest

    EXPECT_EQ(2u, cache.size());
    EXPECT_FALSE(cache.exists(1));
    EXPECT_TRUE(cache.exists(2));
    EXPECT_TRUE(cache.exists(3));
    EXPECT_FALSE(cache.exists(7));
}

TEST(CacheTest, IsMoveOnly) {
    EXPECT_FALSE((std::is_copy_constructible_v<gtl::lru_cache<int, int>>));
    EXPECT_FALSE((std::is_copy_assignable_v<gtl::lru_cache<int, int>>));
    EXPECT_TRUE((std::is_move_constructible_v<gtl::lru_cache<int, int>>));
    EXPECT_TRUE((std::is_move_assignable_v<gtl::lru_cache<int, int>>));
}
