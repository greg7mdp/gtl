/*
 *
 * Example of dumping a map, containing values which are phmap maps or sets
 * building this requires c++17 support
 *
 */

#include <gtl/phmap_dump.hpp>
#include <iostream>

template<class K, class V>
class MyMap : public gtl::flat_hash_map<K, gtl::flat_hash_set<V>>
{
public:
    using Set = gtl::flat_hash_set<V>;

    void dump(const std::string& filename)
    {
        gtl::BinaryOutputArchive ar_out(filename.c_str());

        ar_out.saveBinary(this->size());
        for (auto& [k, v] : *this) {
            ar_out.saveBinary(k);
            ar_out.saveBinary(v);
        }
    }

    void load(const std::string& filename)
    {
        gtl::BinaryInputArchive ar_in(filename.c_str());

        size_t size;
        ar_in.loadBinary(&size);
        this->reserve(size);

        while (size--) {
            K   k;
            Set v;

            ar_in.loadBinary(&k);
            ar_in.loadBinary(&v);

            this->insert_or_assign(std::move(k), std::move(v));
        }
    }

    void insert(K k, V v)
    {
        Set& set = (*this)[k];
        set.insert(v);
    }

    friend std::ostream& operator<<(std::ostream& os, const MyMap& map)
    {
        for (const auto& [k, m] : map) {
            os << k << ": [";
            for (const auto& x : m)
                os << x << ", ";
            os << "]\n";
        }
        return os;
    }
};

int main()
{
    MyMap<size_t, size_t> m;
    m.insert(1, 5);
    m.insert(1, 8);
    m.insert(2, 3);
    m.insert(1, 15);
    m.insert(1, 27);
    m.insert(2, 10);
    m.insert(2, 13);

    std::cout << m << "\n";

    m.dump("test_archive");
    m.clear();
    m.load("test_archive");

    std::cout << m << "\n";

    return 0;
}
