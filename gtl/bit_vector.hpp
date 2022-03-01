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
#include <string>
#include <cstdint>
//#include <bit>
#include <cassert>
#include <iostream>

namespace gtl {

namespace bitv {

static constexpr size_t   stride = 64;
static constexpr uint64_t ones = (uint64_t)-1;
static constexpr size_t   mod(size_t n)      { return (n & 0x3f); }
static constexpr size_t   slot(size_t n)     { return n >> 6; }
static constexpr size_t   slot_cnt(size_t n) { return slot(n + 63); }
static constexpr uint64_t bitmask(size_t n)  { return (uint64_t)1 << mod(n); } // a mask for this bit in its slot
static constexpr uint64_t lowmask (size_t n) { return bitmask(n) - 1; }       // a mask for bits lower than n in slot
static constexpr uint64_t himask(size_t n)  { return ~lowmask(n); }           // a mask for bits higher than n-1 in slot

static constexpr size_t _popcount64(uint64_t y) { // https://gist.github.com/enjoylife/4091854
    y -= ((y >> 1) & 0x5555555555555555ull);
    y = (y & 0x3333333333333333ull) + (y >> 2 & 0x3333333333333333ull);
    return ((y + (y >> 4)) & 0xf0f0f0f0f0f0f0full) * 0x101010101010101ull >> 56;
}

// De Bruijn Multiplication With separated LS1B - author Kim Walisch (2012)
unsigned countr_zero(uint64_t bb) {
    const unsigned index64[64] = {
         0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
    };
    const uint64_t debruijn64 = 0x03f79d71b4cb0a89ull;
    assert (bb != 0);
    return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
}

enum class vt { none=0, view=1, oor_ones=2, backward=4 }; // visitor flags

using vt_type = std::underlying_type<vt>::type;

constexpr bool operator&(vt a, vt b) { return      (vt_type)a & (vt_type)b; }
constexpr vt   operator|(vt a, vt b) { return (vt)((vt_type)a | (vt_type)b); }

// ---------------------------------------------------------------------------
// implements bit storage class
// bits default initialized to 0
// todo: implement sparse storage (maybe like https://github.com/thrunduil/DBS/tree/master/src/dbs
//       or like what I did before)
// ---------------------------------------------------------------------------
template <class A>
class storage {
public:
    storage(size_t num_bits = 0, bool val = false) { resize(num_bits, val); }
    size_t size() const { return _s.size(); }          // size in slots

    // -------------------------------------------------------------------------------
    void resize(size_t num_bits, bool val = false) { 
        _sz = num_bits;
        size_t num_slots = slot_cnt(num_bits);
        _s.resize(num_slots, val ? ones : 0);
        if (mod(num_bits))
            _s[num_slots - 1] &= ~himask(num_bits); // make sure to zero the remainder bits in last slot
        _check_extra_bits();
    }

    bool operator==(const storage &o) const { return _s == o._s; }
    uint64_t operator[](size_t slot) const  { assert(slot < _s.size()); return _s[slot]; }

    // returns a sequence of uint64_t simulating a view starting at a as_first index
    // bits starting at first, where the first uint64_t returned 
    // contains the first init_lg bits of the sequence, shifted shift bits.
    // -------------------------------------------------------------------------------
    class bit_sequence {
    public:
        bit_sequence(const storage &s, size_t first, size_t last, size_t as_first) :
            _s(s), _cur(first), _last(last), 
            _init_lg(mod(stride - mod(as_first))), // _init_lg will be the # of initial bits returned 
            _shift(mod(as_first))                  // as if at end of a slot, so shifted left _shift bits
        {
            assert(last <= _s._sz);
            assert(_init_lg < stride); // the number of bits to return in the first call, following ones are 64 bits till last
            assert(last >= first);
            assert(_init_lg + _shift <= stride);
        }
        
        uint64_t operator()() {
            if (_init_lg) {
                uint64_t res = get_next_bits(_init_lg);
                _init_lg = 0;
                res <<= _shift;
                return res;
            }
            return get_next_bits(stride);
        }
        
        // returns the next std::min(lg, _last - _cur) of the bit sequence
        // masked appropriately
        uint64_t get_next_bits(size_t lg) {
            lg = std::min(lg, _last - _cur);
            assert(lg);

            size_t slot_idx = slot(_cur);
            size_t offset = mod(_cur);
            uint64_t v;

            if (lg == stride && offset == 0) {
                v = _s[slot_idx];
            } else if (lg <= stride - offset) {
                // result all in this slot
                size_t last = _cur + lg;
                v = (_s[slot_idx] & (mod(last) == 0 ? ones : lowmask(last))) >> offset;
            } else {
                v = _s[slot_idx] >> offset;
                size_t lg_left = lg - (stride - offset);
                v |= (_s[slot_idx + 1] & lowmask(lg_left)) << (stride - offset);
            }
            _cur += lg;
            return v;
        }
        
    private:
        const storage &_s;
        size_t _cur;
        size_t _last;
        size_t _init_lg;
        size_t _shift;
    };

    // ------------------------------------------------------------------------------------
    template<class F>
    void update_bit(size_t idx, F f) { 
        assert(idx < _sz);
        size_t slot_idx = slot(idx);
        uint64_t& s  = _s[slot_idx]; 
        uint64_t  fs = f(s); 
        uint64_t  m  = bitmask(idx);
        s &= ~m;
        s |= fs & m;
    }

    // functional update/inspect  by bit range, last is 1 + last index to change
    // function f can modify the whole uint64_t, only relevant bits are copied
    // the shift passed to F moves returned value in correct location(>= 0 means left shift)
    // if (flags & vt::view), this fn exits early when the callback returns true.
    // ------------------------------------------------------------------------------------
    template<vt flags, class F>
    void visit(size_t first, size_t last, F f) { 
        assert(last <= _sz);
        if (last <= first)
            return;
        size_t first_slot = slot(first);
        size_t last_slot  = slot(last);

        constexpr auto oor_bits = [](uint64_t s, uint64_t m) constexpr  -> uint64_t {
            if constexpr (!(flags & vt::oor_ones))
                return  (s & ~m);
            else
                return s | m;
        };

        if (first_slot == last_slot) {
            uint64_t& s  = _s[first_slot]; 
            uint64_t  m  = ~(lowmask(first) ^ lowmask(last)); // m has ones on the bits we don't want to change
            auto fs = f(oor_bits(s, m), (int)mod(first)); 
            if constexpr (!(flags & vt::view)) {
                s &= m;
                s |= fs & ~m;
            } else if (fs)
                return;
        } else if constexpr (!(flags & vt::backward)) {
            // first slot
            // ----------
            if (mod(first)) {
                uint64_t& s  = _s[first_slot]; 
                uint64_t  m  = lowmask(first);      // m has ones on the bits we don't want to change
                auto fs = f(oor_bits(s, m), (int)mod(first)); 
                if constexpr (!(flags & vt::view)) {
                    s &= m;                         // zero bits to be changed
                    s |= fs & ~m;                   // copy masked new value
                } else if (fs)
                    return;
                ++first_slot;
            }
            
            // full slots
            // ----------
            for (size_t slot=first_slot; slot<last_slot; ++slot) {
                uint64_t& s  = _s[slot]; 
                auto fs = f(s, 0); 
                if constexpr (!(flags & vt::view))
                    s = fs; 
                else if (fs)
                    return;
            }

            // last slot
            // ---------
            if (mod(last)) {
                uint64_t& s  = _s[last_slot]; 
                uint64_t  m  = himask(last);       // m has ones on the bits we don't want to change
                auto  fs = f(oor_bits(s, m), -(int)(mod(first))); 
                if constexpr (!(flags & vt::view)) {
                    s &= m;                        // zero bits to be changed
                    s |= fs & ~m;                  // copy masked new value
                } else if (fs)
                    return;
            }
        } else {
            // last slot
            // ---------
            if (mod(last)) {
                uint64_t& s  = _s[last_slot]; 
                uint64_t  m  = himask(last);       // m has ones on the bits we don't want to change
                auto fs = f(oor_bits(s, m), -(int)(mod(first))); 
                if constexpr (!(flags & vt::view)) {
                    s &= m;                        // zero bits to be changed
                    s |= fs & ~m;                  // copy masked new value
                } else if (fs)
                    return;
            }
            --last_slot;

            // full slots
            // ----------
            for (size_t slot=last_slot; slot > first_slot; --slot) {
                uint64_t& s  = _s[slot]; 
                auto fs = f(s, 0); 
                if constexpr (!(flags & vt::view))
                    s = fs;
                else if (fs)
                    return;
            }

            // first slot
            // ----------
            uint64_t& s  = _s[first_slot]; 
            uint64_t  m  = lowmask(first);  // m has ones on the bits we don't want to change
            auto fs = f(oor_bits(s, m), (int)mod(first)); 
            if constexpr (!(flags & vt::view)) {
                s &= m;                         // zero bits to be changed
                s |= fs & ~m;                   // copy masked new value
            } else if (fs)
                return;
        }
        _check_extra_bits();
    }

    void swap(storage &o) { _s.swap(o._s); std::swap(_sz, o._sz); }

private:
    void _check_extra_bits() const {
#ifdef _DEBUG
        // here we make sure that the bits in the last slot past the bit_vector size() are zeroed
        if (mod(_sz))
            assert((_s[slot_cnt(_sz) - 1] & himask(_sz)) == 0);
#endif
    }

    std::vector<uint64_t, A> _s;
    size_t _sz;
};


// ---------------------------------------------------------------------------
// implements bit_view class
// ---------------------------------------------------------------------------
template <class S, template <class S> class BV>
class view {
public:
    static constexpr size_t npos = std::numeric_limits<size_t>::max();
    using vec_type = BV<S>;

    explicit view(vec_type &bv, size_t first = 0, size_t last = npos) : 
        _bv(bv), _first(first)
    {
        _last = (last == npos) ? _bv.size() : last;
        assert(_last >= _first);
    }

    size_t size() const { return _last - _first; }
    bool empty()  const { return _last == _first; }

    // single bit access
    // -----------------
    view& set(size_t idx)   { _bv.set(idx + _first); return *this; }
    view& reset(size_t idx) { _bv.reset(idx + _first); return *this; }
    view& flip(size_t idx)  { _bv.flip(idx + _first); return *this; }
    bool operator[](size_t idx) const { return _bv[idx + _first]; }

    view& set(size_t idx, bool val) { _bv.set(idx + _first, val); return *this; }

    // change whole view
    // -----------------
    view& set()   { _bv.storage().visit<vt::none>(_first, _last, 
                                                  [](uint64_t  , int) { return ones; }); return *this; }

    view& reset() { _bv.storage().visit<vt::none>(_first, _last, 
                                                  [](uint64_t  , int) { return (uint64_t)0; }); return *this; }

    view& flip()  { _bv.storage().visit<vt::none>(_first, _last, 
                                                  [](uint64_t v, int) { return ~v; }); return *this; }

    // compound assignment operators
    // -----------------------------
    template <class F>
    view& bin_assign(const view &o, F &&f) noexcept { 
        assert(size() == o.size()); (void)o;
        _bv.storage().visit<vt::none>(_first, _last, std::forward<F>(f)); 
        return *this;
    }

    view& operator|=(const view &o) noexcept { 
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        return bin_assign(o, [&](uint64_t a, size_t) { return a | seq(); });
    } 

    view& operator&=(const view &o) noexcept {
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        return bin_assign(o, [&](uint64_t a, size_t) { return a & seq(); });
    } 

    view& operator^=(const view &o) noexcept {
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        return bin_assign(o, [&](uint64_t a, size_t) { return a ^ seq(); });
    } 

    view& operator-=(const view &o) noexcept {
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        return bin_assign(o, [&](uint64_t a, size_t) { return a & ~seq(); });
    } 

    view& or_not(const view &o) noexcept {
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        return bin_assign(o, [&](uint64_t a, size_t) { return a | ~seq(); });
    } 

    // shift operators. Zeroes are shifted in.
    // ---------------------------------------
    view& operator<<=(size_t cnt) noexcept { 
        if (cnt >= size())
            reset();
        else if (cnt) {
            if (cnt == stride) {
                size_t carry = 0;
                _bv.storage().visit<vt::none | vt::backward>(_first, _last, [&](uint64_t v, int ) { 
                        size_t res = carry; 
                        carry = v;
                        return res;
                    });
            } else if (cnt <= stride) {
                size_t carry = 0;
                _bv.storage().visit<vt::none | vt::backward>(_first, _last, [&](uint64_t v, int ) { 
                        size_t res = (v >> cnt) | carry; // yes we have to shift the opposite way!
                        carry = (v << (stride - cnt));
                        return res;
                    });
            } else {
                while(cnt) {
                    size_t shift = std::min(cnt, stride);
                    *this <<= shift;
                    cnt -= shift;
                }
            }
        }
        return *this;
    }

    view& operator>>=(size_t cnt) noexcept { 
        if (cnt >= size())
            reset();
        else if (cnt) {
            if (cnt == stride) {
                size_t carry = 0;
                _bv.storage().visit<vt::none>(_first, _last, [&](uint64_t v, int ) { 
                        size_t res = carry; 
                        carry = v;
                        return res;
                    });
            } else if (cnt <= stride) {
                size_t carry = 0;
                _bv.storage().visit<vt::none>(_first, _last, [&](uint64_t v, int ) { 
                        size_t res = (v << cnt) | carry; // yes we have to shift the opposite way!
                        carry = (v >> (stride - cnt));
                        return res;
                    });
            } else {
                while(cnt) {
                    size_t shift = std::min(cnt, stride);
                    *this >>= shift;
                    cnt -= shift;
                }
            }
        }
        return *this; 
    }

    
    // assignment operators
    // --------------------
    view& operator=(uint64_t val) { // only works for view width <= 64 bits
        assert(size() <= stride);
        _bv.storage().visit<vt::none>(_first, _last, 
                                      [val](uint64_t, int shl) { return shl >= 0 ? val << shl : val >> shl; });
        return *this; 
    }

    view& operator=(std::initializer_list<uint64_t> vals) {
        size_t num_vals = vals.size();
        auto v = vals.begin();
        size_t start = _first;
        for(size_t i=0; i<num_vals && start <= _last; ++i) {
            size_t last = std::min(_last, start + stride);
            _bv.view(start, last) = *v++;
            start = last;
        }
        return *this;
    }

    view& operator=(const view &o) {
        assert(size() == o.size());
        if ((&_bv != &o._bv) || (_first < o._first) || (_first <= o._last)) {
            typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first); 
            _bv.storage().visit<vt::none>(_first, _last, 
                                          [&](uint64_t, int) { return seq(); }); 
        } else if (_first > o._first) {
            // both views are on same bitmap, and we are copying backward with an overlap
            typename S::bit_sequence seq(_bv.storage(), _first, _last, o._first); 
            o._bv.storage().visit<vt::none>(o._first, o._last, 
                                            [&](uint64_t, int) { return seq(); }); 
        }
        return *this; 
    }

    // debug version, do not use (instead use operator=() above)
    view& copy_slow(const view &o) {
        assert(size() == o.size());
        // ------ slow version 
        if (&_bv != &o._bv) {
            reset();
            for (size_t i=0; i<size(); ++i)
                if (o[i])
                    set(i);
        } else {
            // same bv, be careful which way we iterate in case the views overlap
            if (_first < o._first)
                for (size_t i=0; i<size(); ++i)
                    set(i, o[i]);
            else if (_first > o._first)
                for (size_t i=size(); i-- > 0; )
                    set(i, o[i]);
        }
        return *this; 
    }

    template <class View>
    bool operator==(const View &o) const {
        if (size() != o.size())
            return false;
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        bool res = true;
        _bv.storage().visit<vt::view>(_first, _last, 
                                      [&](uint64_t v, int) { if (v != seq()) res = false; return !res; }); 
        return res; 
    }
  
    // unary predicates: any, every, etc...
    // ------------------------------------
    bool any() const { 
        bool res = false;
        _bv.storage().visit<vt::view>(_first, _last,
                                      [&](uint64_t v, int) { if (v) res = true; return res; }); 
        return res; 
    }

    bool every() const { 
        bool res = true;
        _bv.storage().visit<vt::view | vt::oor_ones>(_first, _last,
                                                     [&](uint64_t v, int) { if (v != ones) res = false; return !res; }); 
        return res; 
    }

    bool none()  const { return !any(); }

    // binary predicates: contains, disjoint, ...
    // ------------------------------------------
    template <class View>
    bool contains(const View &o) const {
        assert(size() >= o.size());
        if (size() < o.size())
            return false;
        bool res = true;
        typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
        _bv.storage().visit<vt::view>(_first, _last, [&](uint64_t v, int) { 
                if ((v | seq()) != v) res = false; return !res; }); 
        return res;
    }

    template <class View>
    bool disjoint(const View &o) const {
        bool res = true;
        if (size() <= o.size()) {
            typename S::bit_sequence seq(o._bv.storage(), o._first, o._first + size(), _first);
            _bv.storage().visit<vt::view>(_first, _last, [&](uint64_t v, int) { 
                    if (v & seq()) res = false; return !res; }); 
        } else {
            typename S::bit_sequence seq(o._bv.storage(), o._first, o._last, _first);
            _bv.storage().visit<vt::view>(_first, _first + o.size(), [&](uint64_t v, int) { 
                    if (v & seq()) res = false; return !res; }); 
        }
        return res;
    }

    template <class View>
    bool intersect(const View &o) const { return !disjoint(o);  }

    // miscellaneous
    // -------------
    size_t count() const {              // we could use std::popcount in c++20
        size_t cnt = 0;
        _bv.storage().visit<vt::view>(_first, _last,
                                      [&](uint64_t v, int) { cnt += _popcount64(v); return false; }); 
        return cnt;
    }

    // find next one bit - returns npos if not found
    // ---------------------------------------------
    size_t find_first() const { 
        size_t idx = _first;
        _bv.storage().visit<vt::view>(_first, _last, [&](uint64_t v, int shift) { 
                if (v) {
                    idx += countr_zero(v) - shift; // we could use std::countr_zero in c++20
                    return true; // stop iterating
                } else {
                    idx += stride - shift;
                    return false; 
                }
            }); 
        if (idx < _last)
            return idx - _first;
        return npos; 
    }

    size_t find_next(size_t start) const { 
        if (_first + start >= _last)
            return npos; 
        size_t res = _bv.view(_first + start, _last).find_first();
        return (res == npos) ? npos : res + start;
    }

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
    static constexpr size_t npos = bv_type::npos;
    
    explicit vec(size_t sz, bool val = false) : 
        _sz(sz), _s(sz, val) 
    {}

    explicit vec(std::initializer_list<uint64_t> vals) : vec(vals.size() * stride) {
        *this = std::move(vals);
    }

    void resize(size_t sz, bool val = false) { _sz = sz; _s.resize(_sz, val); }

    // bit access
    // ----------
    vec& set(size_t idx)   { assert(idx < _sz); _s.update_bit(idx, [](uint64_t  ) { return ones; }); return *this; }
    vec& reset(size_t idx) { assert(idx < _sz); _s.update_bit(idx, [](uint64_t  ) { return 0; }); return *this; }
    vec& flip(size_t idx)  { assert(idx < _sz); _s.update_bit(idx, [](uint64_t v) { return ~v; }); return *this; }
    bool test(size_t idx) const { return (*this)[idx]; }
    bool operator[](size_t idx) const { return !!(_s[slot(idx)] & bitmask(idx)); }
    unsigned char get_byte(size_t byte_idx) const { return (unsigned char)(_s[byte_idx >> 3] >> ((byte_idx & 7) << 3)); }

    // either sets or resets the bit depending on val
    vec& set(size_t idx, bool val) {
        assert(idx < _sz); _s.update_bit(idx, [&](uint64_t  ) { return val ? ones : 0; }); return *this;
    }

    // change whole bit_vector
    // -----------------------
    vec& set()   { view().set(); return *this; }
    vec& reset() { view().reset(); return *this; }
    vec& flip()  { view().flip(); return *this; }

    // access bit value
    // ----------------

    // bitwise operators on full bit_vector
    // ------------------------------------
    vec operator|(const vec &o) const noexcept { vec res(*this); res |= o;    return res; } 
    vec operator&(const vec &o) const noexcept { vec res(*this); res &= o;    return res; } 
    vec operator^(const vec &o) const noexcept { vec res(*this); res ^= o;    return res; } 
    vec operator-(const vec &o) const noexcept { vec res(*this); res -= o;    return res; } 
    vec operator~()             const noexcept { vec res(*this); res.flip();  return res; }
    vec operator<<(size_t cnt)  const noexcept { vec res(*this); res <<= cnt; return res; } 
    vec operator>>(size_t cnt)  const noexcept { vec res(*this); res >>= cnt; return res; } 

    // compound assignment operators on full bit_vector
    // ------------------------------------------------
    vec& operator|=(const vec &o) noexcept { view() |= o.view(); return *this; }
    vec& operator&=(const vec &o) noexcept { view() &= o.view(); return *this; }
    vec& operator^=(const vec &o) noexcept { view() ^= o.view(); return *this; }
    vec& operator-=(const vec &o) noexcept { view() -= o.view(); return *this; }
    vec& or_not(const vec &o)     noexcept { view().or_not(o.view()); return *this; }

    // assignment operators on full bit_vector
    // ---------------------------------------
    vec& operator=(std::initializer_list<uint64_t> vals) {
        size_t num_vals = vals.size();
        auto v = vals.begin();
        for(size_t i=0; i<num_vals; ++i)
            view(i * stride, std::min((i+1) * stride, _sz)) = *v++;
        return *this;
    }

    // shift operators. Zeroes are shifted in.
    // ---------------------------------------
    vec& operator<<=(size_t cnt)  { view() <<= cnt; return *this; }
    vec& operator>>=(size_t cnt)  { view() >>= cnt; return *this; }

    // unary predicates any, every, etc...
    // -----------------------------------
    bool any()   const { return view().any(); }
    bool every() const { return view().every(); }
    bool none()  const { return !any(); }

    // binary predicates operator==(), contains, disjoint, ...
    // support comparing bit_vectors with different storage
    // -------------------------------------------------------
    template <class S2>
    bool operator==(const vec<S2> &o) const { return this == &o || view() == o.view(); }

    template <class S2> 
    bool operator!=(const vec<S2> &o) const { return !(*this == o); }

    template <class S2> 
    bool contains(const vec<S2> &o) const { return view().contains(o.view()); }
    
    template <class S2> 
    bool disjoint(const vec<S2> &o) const { return view().disjoint(o.view()); }

    template <class S2>
    bool intersects(const vec<S2> &o) const { return !disjoint(o); }

    // miscellaneous
    // -------------
    size_t count() const { return view().count(); }

    void swap(vec &o) {
        std::swap(_sz, o._sz);
        _s.swap(o._s);
    }

    size_t   size() const noexcept   { return _sz; }
    bool     empty() const noexcept  { return _sz == 0; }
    size_t   num_blocks() const noexcept { return slot_cnt(_sz); }
    uint64_t block(size_t idx) const noexcept { return _s[idx]; }

    S&       storage()          { return _s; }
    const S& storage() const    { return _s; }

    // find next one bit
    // -----------------
    size_t find_first() const { return view().find_first(); }
    size_t find_next(size_t pos) const { return view().find_next(pos); }

    // standard bitset conversions
    // ---------------------------
    template<class CharT = char, class Traits = std::char_traits<CharT>, class A = std::allocator<CharT>> 
    std::basic_string<CharT, Traits, A> to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const
    {
        std::basic_string<CharT, Traits, A> res(_sz, zero);
        for (size_t i=0; i<_sz; ++i)
            if (test(_sz - i - 1))
                res[i] = one;
        return res;
    }

    unsigned long long to_ullong() const { return _sz ? (unsigned long long)_s[0] : 0; }

    unsigned long to_ulong() const { (unsigned long)to_ullong(); }

    // print
    // -----
    template<class CharT = char, class Traits = std::char_traits<CharT>> 
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits> &s, const vec &v) 
    { 
        return s << (std::string)v; 
    }

    template<class CharT = char, class Traits = std::char_traits<CharT>, class A = std::allocator<CharT>> 
    void append_to_string(std::basic_string<CharT, Traits, A> &res) const 
    {
        size_t num_bytes = (_sz + 7) >> 3;
        res.reserve(res.size() + num_bytes * 2);

        auto to_hex = [](unsigned char b) -> char { return (b > 9) ? 'a' + b - 10 : '0' + b; };

        for (int i=(int)(num_bytes-1); i>= 0; --i) {
            unsigned char val = get_byte(i);
            res +=  to_hex(val >> 4);
            res += to_hex(val & 0xf);
        }
    }

    // make bit_vector convertible to std::string
    template<class CharT = char, class Traits = std::char_traits<CharT>, class A = std::allocator<CharT>> 
    operator std::basic_string<CharT, Traits, A>() const 
    {
        if (_sz == 0)
            return "<empty>";
        std::basic_string<CharT, Traits, A> res;
        size_t num_bytes = (_sz + 7) >> 3;
        res.reserve(num_bytes * 2 + 2);

        res += "0x";
        append_to_string(res);
        return res;
    }
    
    // access via gtl::bit_view
    // ------------------------
    bv_type view(size_t first = 0, size_t last = npos) { return bv_type(*this, first, last); }
    const bv_type view(size_t first = 0, size_t last = npos) const { return bv_type(const_cast<vec&>(*this), first, last); }

private:
    size_t _sz; // actual number of bits
    S      _s;
};

    
} // namespace bitv

// ---------------------------------------------------------------------------
using storage    = bitv::storage<std::allocator<uint64_t>>;
using bit_vector = bitv::vec<storage>;
using bit_view   = bitv::view<storage, bitv::vec>;


} // namespace gtl

namespace std
{
    // inject specialization of std::hash for gtl::bit_vector into namespace std
    // -------------------------------------------------------------------------
    template<> struct hash<gtl::bit_vector>
    {
        size_t operator()(gtl::bit_vector const &bv) const
        { 
            size_t h = 0;
            size_t num_blocks = bv.num_blocks();
            for (size_t i=0; i<num_blocks; ++i)
                h = h ^ (bv.block(i) + size_t(0xc6a4a7935bd1e995) + (h << 6) + (h >> 2));
            return h;
        }
    };
}

#endif bit_vector_h_guard_
