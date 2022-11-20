#include <array>
#include <gtl/phmap_utils.hpp> // minimal header providing gtl::HashState()
#include <string>
#include <tuple>
#include <utility>
#include <vector>
//#include <string_view>
#include <iostream>

using std::pair;
using std::string;
using std::tuple;

using groupid_t = std::array<uint16_t, 4>;

namespace std {
template<>
struct hash<groupid_t>
{
    std::size_t operator()(groupid_t const& g) const
    {
        return gtl::Hash<decltype(std::tuple_cat(g))>()(std::tuple_cat(g));
    }
};
}

int main()
{
    std::vector<groupid_t> groups = {
        {17,  75, 82, 66},
        { 22, 88, 54, 42},
        { 11, 55, 77, 99}
    };

    for (const auto& g : groups)
        std::cout << std::hash<groupid_t>()(g) << '\n';

    return 0;
}
