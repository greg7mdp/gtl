#include <gtl/lru_cache.hpp>
#include "gtest/gtest.h"

const int NUM_OF_TEST1_RECORDS = 100;
const int NUM_OF_TEST2_RECORDS = 100;
const int TEST2_CACHE_CAPACITY = 50;

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

TEST(CacheTest1, KeepsAllValuesWithinCapacity) {
    gtl::lru_cache<int, int> cache(TEST2_CACHE_CAPACITY);

    for (int i = 0; i < NUM_OF_TEST2_RECORDS; ++i) {
        cache.insert(i, i);
    }

    for (int i = 0; i < NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; ++i) {
        EXPECT_FALSE(cache.exists(i));
    }

    for (int i = NUM_OF_TEST2_RECORDS - TEST2_CACHE_CAPACITY; i < NUM_OF_TEST2_RECORDS; ++i) {
        EXPECT_TRUE(cache.exists(i));
        EXPECT_EQ(i, *cache.get(i));
    }

    size_t size = cache.size();
    EXPECT_EQ(TEST2_CACHE_CAPACITY, size);
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
