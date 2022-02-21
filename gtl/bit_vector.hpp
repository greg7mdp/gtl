#if !defined(bit_vector_h_guard_)
#define bit_vector_h_guard_

// ---------------------------------------------------------------------------
// Copyright (c) 2022, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
// ---------------------------------------------------------------------------
#include <type_traits>
#include <vector>
#include <cstdint>
#include <cassert>

namespace gtl {


// ---------------------------------------------------------------------------
// bits default initialized to 0
// ---------------------------------------------------------------------------
class bit_storage {
public:
    static constexpr size_t stride = 64;
    static constexpr size_t slot_cnt(size_t n) { return (n + 63) >> 6; }
    static constexpr size_t slot(size_t n) { return n >> 6; }

    void resize(size_t n) { _bits.resize(slot_cnt(n), 0); }

private:
    std::vector<uint64_t> _bits;
};


// ---------------------------------------------------------------------------
template <class S, template <class S> class BV>
class bit_view {
public:
    static constexpr size_t end = std::numeric_limits<size_t>::max();
    using bit_vector_type = BV<S>;

    bit_view(bit_vector_type &bv, size_t first, size_t last) : 
        _bv(bv), _first(first)
    {
        _last = (last == end) ? _bv.size() : last;
        assert(_last >= _first);
    }

    size_t size()  const { return _last - _first; }
    size_t empty() const { return _last == _first; }

    // these return true if the value was changed
    // ------------------------------------------
    bool set(size_t idx);
    bool clear(size_t idx);
    bool flip(size_t idx);

    // access bit value
    // ----------------
    bool operator[](size_t idx) const;

    // bitwise operators on full bit_vector
    // ------------------------------------
    bit_view operator|(const bit_view &o);
    bit_view operator&(const bit_view &o);
    bit_view operator^(const bit_view &o);
    bit_view operator~();
    bit_view operator<<(size_t cnt);
    bit_view operator>>(size_t cnt);

    // bitwise assignment operators
    // ----------------------------
    bit_view& operator|=(const bit_view &o) { *this = *this | o; }
    bit_view& operator&=(const bit_view &o) { *this = *this & o; }
    bit_view& operator^=(const bit_view &o) { *this = *this ^ o; }
    bit_view& operator<<=(size_t cnt) { *this = *this << cnt; }
    bit_view& operator>>=(size_t cnt) { *this = *this >> cnt; }


    // miscellaneous
    // -------------
    size_t popcnt() const;

private:
    bit_vector_type&     _bv;
    size_t               _first;
    size_t               _last;
};

// ---------------------------------------------------------------------------
template <class S>
class generic_bit_vector {
public:
    using storage_type = S;
    using bv_type = bit_view<S, generic_bit_vector<S>>;
    
    explicit generic_bit_vector(size_t sz, bool val = false);

    // these return true if the value was changed
    // ------------------------------------------
    bool set(size_t idx);
    bool clear(size_t idx);
    bool flip(size_t idx);

    // access bit value
    // ----------------
    bool operator[](size_t idx) const;

    // bitwise operators on full bit_vector
    // ------------------------------------
    generic_bit_vector operator|(const generic_bit_vector &o);
    generic_bit_vector operator&(const generic_bit_vector &o);
    generic_bit_vector operator^(const generic_bit_vector &o);
    generic_bit_vector operator~();
    generic_bit_vector operator<<(size_t cnt);
    generic_bit_vector operator>>(size_t cnt);

    // bitwise assignment operators
    // ----------------------------
    generic_bit_vector& operator|=(const generic_bit_vector &o) { *this = *this | o; }
    generic_bit_vector& operator&=(const generic_bit_vector &o) { *this = *this & o; }
    generic_bit_vector& operator^=(const generic_bit_vector &o) { *this = *this ^ o; }
    generic_bit_vector& operator<<=(size_t cnt) { *this = *this << cnt; }
    generic_bit_vector& operator>>=(size_t cnt) { *this = *this >> cnt; }


    // miscellaneous
    // -------------
    size_t    popcnt() const  { return bv().popcnt(); }

    size_t   size() const     { return _sz; }
    S&       storage()        { return _s; }
    const S& storage() const  { return _s; }
    size_t&  num_ones()       { return _num_ones; }
    size_t   num_ones() const { return _num_ones; }

        
    static constexpr size_t _unknown      = std::numeric_limits<size_t>::max();
    static constexpr size_t _at_least_one = std::numeric_limits<size_t>::max() - 1;
    
private:
    bv_type bv() { return bv_type(*this, 0, bv_type::end); }

    S      _s;
    size_t _sz;       // number of bits
    size_t _num_ones; // popcnt() if less than _at_least_one
};


// ---------------------------------------------------------------------------
using bit_vector = generic_bit_vector<bit_storage>;


} // namespace gtl

#endif bit_vector_h_guard_
