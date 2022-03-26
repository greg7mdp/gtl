#include <gtl/stopwatch.hpp>
#include <gtl/lru_cache.hpp>
#include <cstdio>

using stopwatch = gtl::stopwatch<std::milli>;

#if 1

// use gtl::memoize_lru => very fast

uint64_t fib(uint64_t n)    // cached fibonacci
{
    static auto cached_fib = gtl::memoize<decltype(&fib)>(&fib);
    if (n <= 1)
        return n;
    return cached_fib(n-1) + cached_fib(n-2);
}

#else

uint64_t fib(uint64_t n)
{
    if (n <= 1)
        return n;
    return fib(n-1) + fib(n-2);
}

#endif

int main()
{
    stopwatch sw;
    constexpr  uint64_t val = 42;
 
    uint64_t x = fib(val);
    sw.snap();
    printf("fib(%zu):         => %zu in %10.3f seconds\n", val, x, sw.since_start() / 1000);
    return 0;
}
 
