// ------------------------
// Windows specific example
// curtesy of @kanonka
// ------------------------
#include <windows.h>

#include "gtl/phmap.hpp"
#include <cstring>
#include <ppl.h>
#include <vector>

using Map = gtl::parallel_flat_hash_map<std::string,
                                        int,
                                        gtl::priv::hash_default_hash<std::string>,
                                        gtl::priv::hash_default_eq<std::string>,
                                        std::allocator<std::pair<const std::string, int>>,
                                        8,
                                        gtl::srwlock>;

class Dict {
    Map m_stringsMap;

public:
    int addParallel(std::string&& str, volatile long* curIdx) {
        int newIndex = -1;
        m_stringsMap.lazy_emplace_l(
            std::move(str),
            [&](Map::value_type& p) { newIndex = p.second; }, // called only when key was already present
            [&](const Map::constructor& ctor)                 // construct value_type in place when key not present
            {
                newIndex = InterlockedIncrement(curIdx);
                ctor(std::move(str), newIndex);
            });

        return newIndex;
    }
};

int main() {
    size_t           totalSize = 6000000;
    std::vector<int> values(totalSize);
    Dict             dict;
    volatile long    index = 0;
    concurrency::parallel_for(size_t(0), size_t(totalSize), [&](size_t i) {
        std::string s = "ab_uu_" + std::to_string(i % 1000000);
        values[i]     = dict.addParallel(std::move(s), &index);
    });

    return 0;
}
