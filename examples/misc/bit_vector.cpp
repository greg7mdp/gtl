#include <gtl/bit_vector.hpp>

int main() {
    gtl::bit_vector bv(128);
    bv.set(10);
    bv.clear(10);

    
    bv.set(63);
    bv.clear(63);

    
    bv.set(65);
    bv.clear(65);

    
    bv.set(130);
    bv.clear(130);

    
    bv.flip(10);
    bv.flip();
    bv.set(10);

    //bv.set(62, 66);
    return 0;
}
    
