#include <cassert>
#include <cstdio>
#include <gtl/adv_utils.hpp>

int main()
{
    {
        // For example, we can use gtl::binary_search to take an integer square root:
        auto x = gtl::binary_search(
            gtl::middle<int>(), [](int x) { return x * x >= 150; }, 0, 100);
        printf("%d < integral sqrt(150) <= %d\n", x.first, x.second);
    }

    {
        // Or floating points
        auto x = gtl::binary_search(
            gtl::middle<double>(), [](double x) { return x * x >= 150.0; }, 0.0, 100.0);
        printf("%.15f < float sqrt(150) <= %.15f\n", x.first, x.second);
    }

    return 0;
}
