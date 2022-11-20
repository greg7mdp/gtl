#include <gtl/phmap_dump.hpp>
#include <iostream>

void dump_load_uint64_uint32()
{
    gtl::flat_hash_map<uint64_t, uint32_t> mp1 = {
        {100,  99 },
        { 300, 299}
    };

    for (const auto& n : mp1)
        std::cout << n.first << "'s value is: " << n.second << "\n";

    {
        gtl::BinaryOutputArchive ar_out("./dump.data");
        mp1.phmap_dump(ar_out);
    }

    gtl::flat_hash_map<uint64_t, uint32_t> mp2;
    {
        gtl::BinaryInputArchive ar_in("./dump.data");
        mp2.phmap_load(ar_in);
    }

    for (const auto& n : mp2)
        std::cout << n.first << "'s value is: " << n.second << "\n";
}

void dump_load_parallel_flat_hash_map()
{
    gtl::parallel_flat_hash_map<uint64_t, uint32_t> mp1 = {
        {100,  99 },
        { 300, 299},
        { 101, 992}
    };

    for (const auto& n : mp1)
        std::cout << "key: " << n.first << ", value: " << n.second << "\n";

    {
        gtl::BinaryOutputArchive ar_out("./dump.data");
        mp1.phmap_dump(ar_out);
    }

    gtl::parallel_flat_hash_map<uint64_t, uint32_t> mp2;
    {
        gtl::BinaryInputArchive ar_in("./dump.data");
        mp2.phmap_load(ar_in);
    }

    for (const auto& n : mp2)
        std::cout << "key: " << n.first << ", value: " << n.second << "\n";
}

int main()
{
    dump_load_uint64_uint32();
    dump_load_parallel_flat_hash_map();
    return 0;
}
