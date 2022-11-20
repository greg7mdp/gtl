/*
 * Make sure that the phmap.hpp header builds fine when included in two separate
 * source files
 */
#include <gtl/phmap.hpp>
#include <string>

using gtl::flat_hash_map;

int main()
{
    // Create an unordered_map of three strings (that map to strings)
    using Map = flat_hash_map<std::string, std::string>;
    Map email = {
        {"tom",   "tom@gmail.com"     },
        { "jeff", "jk@gmail.com"      },
        { "jim",  "jimg@microsoft.com"}
    };

    extern void f2(Map&);
    f2(email);

    return 0;
}
