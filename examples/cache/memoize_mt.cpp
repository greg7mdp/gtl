#include <chrono>
#include <mutex>
#include <cmath>
#include <vector>
#include <thread>
#include <future>
#include <cstdio>
#include <gtl/stopwatch.hpp>

#define USE_CACHING 1

#if USE_CACHING
    #include <gtl/lru_cache.hpp>
#endif

using namespace std::chrono_literals;

double complexMathFunction(int a, double b)
{
    double x = 0;
    for(int i=a; i<a+5000; ++i)
        x += std::sin(double(i) / b) + std::cos((double)i) + tanh(double(i));
    return x;
}

double simulate()
{
    double res = 0;
    
#if USE_CACHING
    static auto cached_complexMathFunction =
        gtl::mt_memoize<decltype(&complexMathFunction), false>(&complexMathFunction);
    
    for(int i = 0; i < 50000; i++) 
        res += cached_complexMathFunction(i % 1000, 3.14);
#else
    for(int i = 0; i < 50000; i++) 
        res += complexMathFunction(i % 1000, 3.14);
#endif
    
    return res;
}

int main()
{
    double sum = 0;
    std::vector<std::future<double>> threads;
    gtl::stopwatch<std::milli> sw;
    
    for(int i = 0; i < 100; i++) {
        threads.emplace_back(std::async(simulate)); // new thread
    }
    
    for(auto& t: threads) {
        sum += t.get(); // sum results from all threads
    }

    printf("Computed sum = %.3f (expected 24999981374.176) in  %10.3f seconds\n", sum, sw.since_start() / 1000);
    
    return 0;
}
