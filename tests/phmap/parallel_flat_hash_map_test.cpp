#define THIS_HASH_MAP parallel_flat_hash_map
#define THIS_TEST_NAME ParallelFlatHashMap

#include <thread>

#include "parallel_hash_map_test.cpp"

#include <shared_mutex>

template <typename K> using HashEqual = gtl::priv::hash_default_eq<K>;
template <typename V> using HashFn = gtl::priv::hash_default_hash<V>;
template <typename K> using Allocator = gtl::priv::Allocator<K>;

template <typename K, typename V, size_t N>
using parallel_flat_hash_map =
   gtl::parallel_flat_hash_map<K, V, HashFn<K>, HashEqual<K>,
                                 Allocator<gtl::priv::Pair<K, V>>, N,
                                 std::shared_mutex>;

using Table = parallel_flat_hash_map<int, int, 10>;

TEST(THIS_TEST_NAME, ConcurrencyCheck) {
   static constexpr int THREADS = 10;
   static constexpr int EPOCH = 1000;
   static constexpr int KEY = 12345;

   auto Incr = [](Table *table) {
      auto exist_fn = [](typename Table::value_type &value) { value.second += 1; };
      auto emplace_fn = [](const typename Table::constructor &ctor) {
         ctor(KEY, 1);
      };
      for (int i = 0; i < EPOCH; ++i) {
         (void)table->lazy_emplace_l(KEY, exist_fn, emplace_fn);
      }
   };

   Table table;
   std::vector<std::thread> threads;
   threads.reserve(THREADS);
   for (int i = 0; i < THREADS; ++i) {
      threads.emplace_back([&]() { Incr(&table); });
   }

   for (auto &thread : threads) {
      thread.join();
   }

   EXPECT_EQ(table[KEY], 10000);
}