#include <cinttypes>
#include <cstdio>
#include <gtl/memoize.hpp>
#include <gtl/stopwatch.hpp>

// -------------------------------------------------------------------------
// create memoized (and very fast) functions which allow for lazy-like
// Haskell behavior. On my machine, the code below computes the first
// 10,000 twin prime numbers pairs in 0.3 seconds.
//
// This is somewhat equivalent to (and faster than) the haskell version,
// which you can try in https://replit.com/languages/haskell
//
//   primes = 2 : (filter ( (==1) . length .  primeFactors) [3, 5 ..])
//
//   primeFactors n = _pf n primes
//      where _pf n (x:xs) | x*x > n = [n]
//                         | n `mod` x == 0 = x : (_pf (n `div` x) (x:xs))
//                         | otherwise = _pf n xs
//
//   twinPrimes = _tp primes
//      where _tp (a:b:rest) | b == a + 2 = (a, b) : _tp (b:rest)
//                           | otherwise = _tp (b:rest)
//
//   main = putStrLn $ show $ twinPrimes !! 10000
// -------------------------------------------------------------------------

using stopwatch = gtl::stopwatch<std::milli>;

// declare function prototypes
// ---------------------------
uint64_t nth_prime(uint64_t index);
uint64_t num_factors(uint64_t n);
uint64_t twin_primes(uint64_t idx);

// create memoized functions from prototypes
// -----------------------------------------
auto cached_nth_prime   = gtl::memoize<decltype(&nth_prime)>(&nth_prime);
auto cached_twin_primes = gtl::memoize<decltype(&twin_primes)>(&twin_primes);

// returns f(end), but avoid recursing one by one
// ---------------------------------------------------------
template<class F>
void avoid_deep_recursion(F& f, uint64_t end)
{
    constexpr uint64_t incr = 512;
    if (end > incr && f.contains(end - incr))
        return;

    for (uint64_t i = incr; i < end; i += incr)
        (void)f(i);
}

// return the nth element in the infinite list of prime numbers
// ------------------------------------------------------------
uint64_t nth_prime(uint64_t idx)
{
    if (idx == 0)
        return 2;

    avoid_deep_recursion(cached_nth_prime, idx - 1);
    uint64_t cur = cached_nth_prime(idx - 1);
    while (true) {
        cur += idx > 1 ? 2 : 1;
        if (num_factors(cur) == 1)
            return cur;
    }
    assert(0);
}

// returns the number of prime factors of n
// ----------------------------------------
uint64_t num_factors(uint64_t n)
{
    for (uint64_t i = 0;; ++i) {
        uint64_t factor = cached_nth_prime(i);
        if (factor * factor > n)
            return 1;
        // printf("%d %d\n", i,  factor);
        if (n % factor == 0)
            return 1 + num_factors(n / factor); // should restart from n
    }
    assert(0);
}

// returns the index, in the infinite list of prime numbers, of the first prime
// of the idx'th pair of twin primes.
// ----------------------------------------------------------------------------
uint64_t twin_primes(uint64_t idx)
{
    if (idx == 0)
        return 1; // (3, 5) are the first twin primes

    avoid_deep_recursion(cached_twin_primes, idx - 1);
    uint64_t i = cached_twin_primes(idx - 1) + 1;

    while (true) {
        uint64_t a = cached_nth_prime(i);
        uint64_t b = cached_nth_prime(i + 1);
        if (b == a + 2)
            return i;
        i += 1;
    }
    assert(0);
}

int main()
{
    auto x1 = [](int i) -> int { return i + 1; };
    auto y  = gtl::memoize<decltype(x1)>(x1);
    printf("---- %d\n", y(6));

    stopwatch          sw;
    constexpr uint64_t idx = 10000;

    auto x = cached_nth_prime(idx);
    printf("cached_nth_prime(%" PRIu64 "):   => %" PRIu64 " in %10.3f seconds\n",
           idx,
           x,
           sw.since_start() / 1000);

    auto first = cached_twin_primes(idx);
    printf("cached_twin_primes(%" PRIu64 "): => (%" PRIu64 ", %" PRIu64 ") in %10.3f seconds\n",
           idx,
           cached_nth_prime(first),
           cached_nth_prime(first + 1),
           sw.since_start() / 1000);

#if 0
    // lazy list - not working
    auto primes = gtl::lazy_list(uint64_t(2), [](auto p, size_t idx) -> uint64_t { 
            uint64_t cur = (*p)[idx - 1]; 
            while (true) {
                cur += idx > 1 ? 2 : 1;
                if (num_factors(cur) == 1) 
                    return cur;
            }
            assert(0); return 0;
        });

    assert(primes[100] == 547);
    assert(primes[10000] == 104743);
#endif

    return 0;
}
