#include "gtest/gtest.h"
#include <gtl/lru_cache.hpp>

constexpr int CACHETEST1_NUM_OF_RECORDS = 100;
constexpr int CACHETEST1_CACHE_CAPACITY = 50;

TEST(CacheTest, SimplePut)
{
    gtl::lru_cache<int, int> cache;
    cache.insert(7, 777);
    EXPECT_TRUE(cache.exists(7));
    EXPECT_EQ(777, *cache.get(7));
}

TEST(CacheTest, MissingValue)
{
    gtl::lru_cache<int, int> cache;
    EXPECT_FALSE(cache.get(7));
}

TEST(CacheTest1, KeepsAllValuesWithinCapacity)
{
    gtl::lru_cache<int, int> cache(CACHETEST1_CACHE_CAPACITY);

    for (int i = 0; i < CACHETEST1_NUM_OF_RECORDS; ++i) {
        cache.insert(i, i);
    }

    for (int i = 0; i < CACHETEST1_NUM_OF_RECORDS - CACHETEST1_CACHE_CAPACITY; ++i) {
        EXPECT_FALSE(cache.exists(i));
    }

    for (int i = CACHETEST1_NUM_OF_RECORDS - CACHETEST1_CACHE_CAPACITY;
         i < CACHETEST1_NUM_OF_RECORDS;
         ++i) {
        EXPECT_TRUE(cache.exists(i));
        EXPECT_EQ(i, *cache.get(i));
    }

    size_t size = cache.size();
    EXPECT_EQ(static_cast<size_t>(CACHETEST1_CACHE_CAPACITY), size);
}

TEST(CacheTest1, mtKeepsAllValuesWithinCapacity)
{
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
