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
#include <iostream>

namespace gtl {

namespace bitv {

static constexpr size_t   stride = 64;
static constexpr uint64_t all_ones = (uint64_t)-1;
static constexpr size_t   mod(size_t n) { return (n & 0x3f); }
static constexpr size_t   slot_cnt(size_t n) { return (n + 63) >> 6; }
static constexpr size_t   slot(size_t n)    { return n >> 6; }
static constexpr uint64_t bitmask(size_t n) { return (uint64_t)1 << mod(n); } // a mask for this bit in its slot
static constexpr uint64_t lowmask(size_t n) { return bitmask(n) - 1; }        // a mask for lower bits than n in slot
static constexpr uint64_t himask(size_t n)  { return ~lowmask(n); }           // a mask for higher bits than n-1 in slot

// ---------------------------------------------------------------------------
// implements bit storage class
// bits default initialized to 0
// ---------------------------------------------------------------------------
class storage {
public:
    
    storage(size_t num_bits = 0, bool val = false) { resize(num_bits, val); }
    size_t size() const { return _s.size(); }          // size in slots
    void resize(size_t num_bits, bool val = false) { 
        _sz = num_bits;
        size_t num_slots = slot_cnt(num_bits);
        _s.resize(num_slots, val ? (uint64_t)-1 : 0);
        if (mod(num_bits))
            _s[num_slots - 1] &= ~himask(num_bits); // make sure to zero the remainder bits in last slot
        _check_extra_bits();
    }

    bool operator==(const storage &o) const { return _s == o._s; }
    uint64_t operator[](size_t slot) const  { assert(slot < _s.size()); return _s[slot]; }

    // functional update by slot
    // -------------------------
    template<class F>
    void update_slot(size_t slot, F&& f) { 
        uint64_t& s = _s[slot]; 
        s = std::forward<F>(f)(s); 
        _check_extra_bits();
    }

    template<class F>
    void update_bit(size_t idx, F f) { 
        size_t slot_idx = slot(idx);
        uint64_t& s  = _s[slot_idx]; 
        uint64_t  fs = f(s); 
        uint64_t  m  = bitmask(idx);
        s &= ~m;
        s |= fs & m;
    }

    // functional update by bit range, last is 1 + last index to change
    // function f can modify the whole uint64_t, only relevant bits are copied
    // the shift passed to F moves returned value in correct location
    // (>= 0 means left shift)
    // -----------------------------------------------------------------------
    template<class F>
    void update(size_t first, size_t last, F f) { 
        if (last <= first)
            return;
        size_t first_slot = slot(first);
        size_t last_slot  = slot(last);
        if (first_slot == last_slot) {
            uint64_t& s  = _s[first_slot]; 
            uint64_t  fs = f(s, (int)mod(first)); 
            uint64_t  m  = lowmask(first) ^ lowmask(last); // m has ones on the bits we want to change
            s &= ~m;
            s |= fs & m;
        } else {
            // first slot
            // ----------
            if (mod(first)) {
                uint64_t& s  = _s[first_slot]; 
                uint64_t  fs = f(s, (int)mod(first)); 
                uint64_t  m  = lowmask(first);  // m has ones on the bits we don't want to change
                s &= m;                         // zero bits to be changed
                s |= fs & ~m;                   // copy masked new value
                ++first_slot;
            }
            
            // full slots
            // ----------
            for (size_t slot=first_slot; slot<last_slot; ++slot) {
                uint64_t& s  = _s[slot]; 
                s = f(s, 0); 
            }

            // last slot
            // ---------
            if (mod(last)) {
                uint64_t& s  = _s[last_slot]; 
                uint64_t  fs = f(s, -(int)(mod(first))); 
                uint64_t  m  = himask(last);   // m has ones on the bits we don't want to change
                s &= m;                        // zero bits to be changed
                s |= fs & ~m;                  // copy masked new value
            }
        }
        _check_extra_bits();
    }

    // -----------------------------------------------------------------------
    template<class F>
    void combine_all(size_t sz, const storage &o, F f) { 
        assert(size() == o.size());
        size_t num_slots = size();
        if (!num_slots)
            return;
        size_t slot;
        for (slot=0; slot<num_slots-1; ++slot)
            _s[slot] = f(_s[slot], o._s[slot]);
        uint64_t m  = mod(sz) ? ~himask(sz) : (uint64_t)-1;
        _s[slot] = f(_s[slot], o._s[slot]) & m; // mask last returned value so we don't set bits past end
        _check_extra_bits();
    }

    void swap(storage &o) { _s.swap(o._s); }

private:
    void _check_extra_bits() const {
#ifdef _DEBUG
        // here we make sure that the bits in the last slot past the bit_vector size() are zeroed
        if (mod(_sz))
            assert((_s[slot_cnt(_sz) - 1] & himask(_sz)) == 0);
#endif
    }

    std::vector<uint64_t> _s;
    size_t _sz;
};


// ---------------------------------------------------------------------------
// implements bit_view class
// ---------------------------------------------------------------------------
template <class S, template <class S> class BV>
class view {
public:
    static constexpr size_t end = std::numeric_limits<size_t>::max();
    using vec_type = BV<S>;

    explicit view(vec_type &bv, size_t first = 0, size_t last = end) : 
        _bv(bv), _first(first)
    {
        _last = (last == end) ? _bv.size() : last;
        assert(_last >= _first);
    }

    size_t size()  const { return _last - _first; }
    bool empty() const { return _last == _first; }

    // single bit access
    // -----------------
    bool set(size_t idx)   { _bv.set(idx + _first); }
    bool clear(size_t idx) { _bv.clear(idx + _first); }
    bool flip(size_t idx)  { _bv.flip(idx + _first); }
    bool operator[](size_t idx) const { return _bv[idx + _first]; }

    // change whole view
    // -----------------
    view& set()   { _bv.storage().update(_first, _last, [](uint64_t  , int) { return (uint64_t)-1; }); return *this; }
    view& clear() { _bv.storage().update(_first, _last, [](uint64_t  , int) { return (uint64_t)0; });  return *this; }
    view& flip()  { _bv.storage().update(_first, _last, [](uint64_t v, int) { return ~v; });           return *this; }

    view& set_uint64(uint64_t val) { 
        assert(size() <= 64);
        _bv.storage().update(_first, _last, [val](uint64_t, int shl) { return shl >= 0 ? val << shl : val >> shl; });
        return *this; 
    }

    // bitwise assignment operators
    // ----------------------------
    view& operator|=(const view &o);
    view& operator&=(const view &o);
    view& operator^=(const view &o);
    view& operator<<=(size_t cnt);
    view& operator>>=(size_t cnt);

    // unary predicates: any, every, etc...
    // ------------------------------------
    bool any()   const;
    bool every() const;
    bool none()  const { return !any(); }

    // binary predicates operator==(), contains, disjoint, ...
    // -------------------------------------------------------
    bool operator==(const view &o) const;

    // miscellaneous
    // -------------
    size_t popcnt() const;

private:
    vec_type&   _bv;
    size_t      _first;
    size_t      _last;
};

// ---------------------------------------------------------------------------
// implements bit_vector class
// ---------------------------------------------------------------------------
template <class S>
class vec {
public:
    using storage_type = S;
    using bv_type = view<S, vec<S>>;
    static constexpr size_t end = bv_type::end;
    
    explicit vec(size_t sz, bool val = false) : 
        _sz(sz), _s(sz, val) 
    {}

    explicit vec(std::initializer_list<uint64_t> vals) : vec(vals.size() * stride) {
        *this = std::move(vals);
    }

    void resize(size_t sz, bool val = false) { _sz = sz; _s.resize(_sz, val); }

    // bit access
    // ----------
    void set(size_t idx)   { assert(idx < _sz); _s.update_bit(idx, [](uint64_t  ) { return (uint64_t)-1; }); }
    void clear(size_t idx) { assert(idx < _sz); _s.update_bit(idx, [](uint64_t  ) { return (uint64_t)0; }); }
    void flip(size_t idx)  { assert(idx < _sz); _s.update_bit(idx, [](uint64_t v) { return ~v; }); }
    bool operator[](size_t idx) const { return !!(_s[slot(idx)] & bitmask(idx)); }
    unsigned char get_byte(size_t byte_idx) const { return (unsigned char)(_s[byte_idx >> 3] >> ((byte_idx & 7) << 3)); }

    // change whole bit_vector
    // -----------------------
    void set()   { view().set(); }
    void clear() { view().clear(); }
    void flip()  { view().flip(); }

    // access bit value
    // ----------------

    // bitwise operators on full bit_vector
    // ------------------------------------
    vec operator|(const vec &o) const { vec res(*this); res |= o;    return res; } 
    vec operator&(const vec &o) const { vec res(*this); res &= o;    return res; } 
    vec operator^(const vec &o) const { vec res(*this); res ^= o;    return res; } 
    vec operator-(const vec &o) const { vec res(*this); res -= o;    return res; } 
    vec operator~()             const { vec res(*this); res.flip();  return res; }
    vec operator<<(size_t cnt)  const { vec res(*this); res <<= cnt; return res; } 
    vec operator>>(size_t cnt)  const { vec res(*this); res >>= cnt; return res; } 

    // bitwise assignment operators on full bit_vector
    // -----------------------------------------------
    template <class F>
    vec& bin_assign(F &&f, const vec &o) { 
        assert(_sz == o._sz); 
        _s.combine_all(_sz, o._s, std::forward<F>(f)); 
        return *this;
    }

    vec& operator|=(const vec &o) { return bin_assign([](uint64_t a, uint64_t b) { return a | b; },  o); }
    vec& operator&=(const vec &o) { return bin_assign([](uint64_t a, uint64_t b) { return a & b; },  o); }
    vec& operator^=(const vec &o) { return bin_assign([](uint64_t a, uint64_t b) { return a ^ b; },  o); }
    vec& operator-=(const vec &o) { return bin_assign([](uint64_t a, uint64_t b) { return a & ~b; }, o); }
    vec& or_not(const vec &o)     { return bin_assign([](uint64_t a, uint64_t b) { return a | ~b; }, o); }

    vec& operator=(std::initializer_list<uint64_t> vals) {
        size_t num_vals = vals.size();
        auto v = vals.begin();
        for(size_t i=0; i<num_vals; ++i)
            view(i * stride, std::min((i+1) * stride, _sz)).set_uint64(*v++);
        return *this;
    }

    // shift operators
    // ---------------
    vec& operator<<=(size_t cnt)  { view() <<= cnt; return *this; }
    vec& operator>>=(size_t cnt)  { view() >>= cnt; return *this; }

    // unary predicates any, every, etc...
    // -----------------------------------
    bool any() const { 
        size_t num_slots = _s.size(); 
        for (size_t i=0; i<num_slots; ++i) 
            if (_s[i]) 
                return true; 
        return false; 
    }

    bool every() const { 
        size_t num_slots = _s.size(); 
        if (!num_slots) 
            return true;
        size_t slot;
        for (slot=0; slot<num_slots-1; ++slot)
            if (_s[slot] != all_ones)
                return false;
        if ((_s[slot] | himask(_sz)) != all_ones) // check only add missing ones on last slot
            return false;
        return true;
    }

    bool none()  const { return !any(); }

    // binary predicates operator==(), contains, disjoint, ...
    // support comparing bit_vectors with different storage
    // -------------------------------------------------------
    template <class S2>
    bool operator==(const vec<S2> &o) const {
        if (_sz != o._sz)
            return false;
        size_t num_slots = _s.size();
        for (size_t i=0; i<num_slots; ++i) 
            if (_s[i] != o._s[i]) 
                return false;
        return true;
    }

    template <class S2> 
    bool operator!=(const vec<S2> &o) const { return !(*this == o); }

    template <class S2>
    bool contains(const vec<S2> &o) const {
        if (_sz <= o._sz)
            return false;
        size_t num_slots = o._s.size();
        for (size_t i=0; i<num_slots; ++i) 
            if (_s[i] | o._s[i] != _s[i])  
                return false;              // o._s[i] has some bits set that are not in _s[i]
        return true;
    }

    template <class S2>
    bool disjoint(const vec<S2> &o) const {
        size_t sz = std::min(_sz, o._sz);
        size_t num_slots = slot_cnt(sz);
        for (size_t i=0; i<num_slots; ++i) 
            if (_s[i] & o._s[i])           // binary and == 0 => the two have no bit in common
                return false;
        return true;
    }

    // miscellaneous
    // -------------
    size_t   popcnt() const   { return view().popcnt(); }

    size_t   size() const     { return _sz; }
    S&       storage()        { return _s; }
    const S& storage() const  { return _s; }

    // print
    // -----
    friend std::ostream& operator<<(std::ostream &s, const vec &v) { return s << v.print(); }

    std::string print() const 
    {
        if (_sz == 0)
            return "<empty>";
        std::string res;
        size_t num_bytes = (_sz + 7) >> 3;
        res.reserve(num_bytes * 2 + 2);

        res += "0x";
        auto to_hex = [](unsigned char b) -> char { return (b > 9) ? 'a' + b - 10 : '0' + b; };

        for (int i=(int)(num_bytes-1); i>= 0; --i) {
            unsigned char val = get_byte(i);
            res +=  to_hex(val >> 4);
            res += to_hex(val & 0xf);
        }
        return res;
    }
    
    // access via gtl::bit_view
    // ------------------------
    bv_type view(size_t first = 0, size_t last = end) { return bv_type(*this, first, last); }

private:
    size_t _sz; // actual number of bits
    S      _s;
};

    
} // namespace bitv

// ---------------------------------------------------------------------------
using bit_vector = bitv::vec<bitv::storage>;
using bit_view   = bitv::view;



} // namespace gtl

#endif bit_vector_h_guard_
