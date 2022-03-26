#include <gtl/stopwatch.hpp>
#include <gtl/lru_cache.hpp>
#include <cstdio>

// -------------------------------------------------------------------------
// create memoized (and very fast functions) that allows for lazy-like
// Haskell behavior. On my machine, the code below computes the first 
// 10,000 twin prime numbers pairs in 0.5 seconds.
//
// This is somewhat equivalent (and faster) than the haskell version, 
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
auto cached_nth_prime   = gtl::memoize<decltype(nth_prime)>(nth_prime);
auto cached_num_factors = gtl::memoize<decltype(num_factors)>(num_factors);
auto cached_twin_primes = gtl::memoize<decltype(twin_primes)>(twin_primes);

constexpr size_t stack_skip = 200; // avoid too deep recursion

// return the nth element in the infinite list of prime numbers
// ------------------------------------------------------------
uint64_t nth_prime(uint64_t idx) {
    if (idx <= 1)
        return idx + 2;
    
    // compute some intermediate primes to avoid too deep recursion
    for (uint64_t i=stack_skip; i < idx; i += stack_skip) 
        (void)cached_nth_prime(i);

    uint64_t cur = cached_nth_prime(idx - 1);
    while (true) {
        cur += 2;
        if (cached_num_factors(cur) == 1)
            return cur;
    }
    assert(0);
}

// returns the number of prime factors of n
// ----------------------------------------
uint64_t num_factors(uint64_t n) {
    for (uint64_t i=0; ; ++i) {
        uint64_t factor = cached_nth_prime(i);
        if (factor * factor > n)
            return 1;
        if (n % factor == 0)
            return 1 + cached_num_factors(n / factor);
    }
    assert(0);
}

// returns the index, in the infinite list of prime numbers, of the first prime 
// of the idx'th pair of twin primes.
// ----------------------------------------------------------------------------
uint64_t twin_primes(uint64_t idx) {
    if (idx == 0)
        return 1; // (3, 5) are the first twin primes

    // compute some intermediate twin primes to avoid too deep recursion
    for (uint64_t i=stack_skip; i < idx; i += stack_skip) 
        (void)cached_twin_primes(i);

    uint64_t i = cached_twin_primes(idx - 1) + 1;

    while (true) {
        uint64_t a = cached_nth_prime(i);
        uint64_t b = cached_nth_prime(i+1);
        if (b == a + 2)
            return i;
        i += 1;
    }
    assert(0);
}
        
int main()
{
    stopwatch sw;
    constexpr  uint64_t idx = 10000;
 
    auto x = cached_nth_prime(idx);
    printf("cached_nth_prime(%zu):   => %zu in %10.3f seconds\n", idx,  x, sw.since_start() / 1000);

    auto first = cached_twin_primes(idx);
    printf("cached_twin_primes(%zu): => (%zu, %zu) in %10.3f seconds\n", 
           idx, cached_nth_prime(first), cached_nth_prime(first+1), sw.since_start() / 1000);
    return 0;
}
 
