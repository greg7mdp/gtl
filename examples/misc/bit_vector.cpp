#include <gtl/bit_vector.hpp>

using std::string;

int main() {
    {
        // allocate small bit_vector on the stack, do very basic operations on it
        // ----------------------------------------------------------------------
        gtl::bit_vector bv(16);
        assert((string)bv == "0x0000");  // by default, initialized to 0 (false).
        assert(bv[0] == false);          // get value of bit 0

        bv.set(0);
        assert(bv[0]);
        assert((string)bv == "0x0001");

        bv.flip();                       // flip all bits
        assert((string)bv == "0xfffe");

        bv.reset();                      // reset all the bits
        assert((string)bv == "0x0000");

        bv.set(0).set(4).set(8);         // calls can often be chained 
        assert((string)bv == "0x0111");

        // quick look at views... more on that later
        bv.reset();                      // reset all the bits
        bv.view(0, 4)  = 0x1;
        bv.view(4, 8)  = 0x2;
        bv.view(8, 12) = 0x3;
        assert((string)bv == "0x0321");
    }

    {
        // We can initialize with std::initializer_list<uint64_t>
        // each value adds 64 bit to the bit_vector
        // ------------------------------------------------------
        gtl::bit_vector bv { 0x0321 }; 
        assert(bv.size() == 64);
        assert((string)bv == "0x0000000000000321"); // when output, read bit vector right to left
                                                    // for bv[0] is the rightmost

        // shift operators
        // ---------------
        bv >>= 4;
        assert((string)bv == "0x0000000000003210");
        bv >>= 12;
        assert((string)bv == "0x0000000003210000");
        bv <<= 16;
        assert((string)bv == "0x0000000000000321"); // back to the original value

        // assigning an initializer_list work as well
        // ------------------------------------------
        bv = { 0xffffffff00000000 };
        assert((string)bv == "0xffffffff00000000");

        gtl::bit_vector  bv2 { 0x00000000ffffffff };
        assert((string)bv2 == "0x00000000ffffffff");

        // let's try bitwise operators
        // ---------------------------
        assert((bv & bv2).none());      // binary and => no one bit remains
        assert((string)(bv & bv2) == "0x0000000000000000");
        assert((bv & bv2).count() == 0);

        assert((bv | bv2).every());     // binary or => all bits are set
        assert((string)(bv | bv2) == "0xffffffffffffffff");
        assert((bv | bv2).count() == 64);

        assert((bv ^ bv2).every());     // binary xor => all bits are set

        // there are more operators (-, |=, &=, ^=, ==) and methods like
        // contains(), disjoint(), 
    }

    {
        // views are very powerful. You can create a view on a part of the bitmap
        // and then query and change the bits on the view by themselves. 
        // ----------------------------------------------------------------------
        gtl::bit_vector bv { 0x0321 }; 
        assert((string)bv == "0x0000000000000321");

        bv.view(0, 4) = 0xf;
        assert((string)bv == "0x000000000000032f");

        bv.view(4, 12) = 0xde;
        assert((string)bv == "0x0000000000000def");

        bv.view(60, 64) = 0x7;
        assert((string)bv == "0x7000000000000def");

        bv.view(4, 20) >>= 8;
        assert((string)bv == "0x70000000000de00f");

        bv.view(4, 12) = bv.view(12, 20);
        assert((string)bv == "0x70000000000dedef");

        assert(bv.view( 0,  4).count() == 4); // count set bits
        assert(bv.view(32, 64).count() == 3);

        bv.view(56, 60) |= bv.view(60, 64);
        assert((string)bv == "0x77000000000dedef");
        
    }

    return 0;
}
    
