#ifndef gtl_utils_hpp_guard
#define gtl_utils_hpp_guard

// ---------------------------------------------------------------------------
// Copyright (c) 2022, Gregory Popovitch - greg7mdp@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
// ---------------------------------------------------------------------------

#include <cstdint>
#include <utility>

// ---------------***     WARNING     ***-------------------------------------
// ---------------*** U N T E S T E D ***-------------------------------------

namespace gtl {

// ---------------------------------------------------------------------------
// An object which calls a lambda in its constructor, and another one in
// its destructor
// ---------------------------------------------------------------------------
template<class Unset>
class scoped_set_unset
{
public:
    template<class Set>
    scoped_set_unset(Set&& set, Unset&& unset, bool do_it = true)
        : do_it_(do_it)
        , unset_(std::move(unset))
    {
        if (do_it_)
            std::forward<Set>(set)();
    }

    ~scoped_set_unset()
    {
        if (do_it_)
            unset_();
    }

    scoped_set_unset(const scoped_set_unset&)            = delete;
    scoped_set_unset& operator=(const scoped_set_unset&) = delete;

private:
    Unset unset_;
    bool  do_it_;
};

// ---------------------------------------------------------------------------
// An object which assigns a value to a variable in its constructor, and resets
// the previous its destructor
// ---------------------------------------------------------------------------
template<class T>
class scoped_set_value
{
public:
    template<class V>
    scoped_set_value(T& var, V&& val, bool do_it = true)
        : v_(var)
        , do_it_(do_it)
    {
        if (do_it_) {
            old_value_ = std::move(v_);
            v_         = std::forward<V>(val);
        }
    }

    ~scoped_set_value()
    {
        if (do_it_)
            v_ = std::move(old_value_);
    }

    scoped_set_value(const scoped_set_value&)            = delete;
    scoped_set_value& operator=(const scoped_set_value&) = delete;

    T&   v_;
    T    old_value_;
    bool do_it_;
};

// ---------------------------------------------------------------------------
// assigns val to var, and returns true if the value changed
// ---------------------------------------------------------------------------
template<class T, class V>
bool change(T& var, V&& val)
{
    if (var != val) {
        var = std::forward<V>(val);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// assigns val to var, and returns the previous value
// ---------------------------------------------------------------------------
template<class T, class V>
T replace(T& var, V&& val) noexcept
{
    T old = std::move(var);
    var   = std::forward<V>(val);
    return old;
}

// ---------------------------------------------------------------------------
// A baseclass to keep track of modifications.
// Change member `x_` using `set_with_ts`
// ---------------------------------------------------------------------------
class timestamp
{
public:
    timestamp() { stamp_ = ++clock_; }

    timestamp(uint64_t stamp)
        : stamp_(stamp)
    {
    }

    void touch() { stamp_ = ++clock_; }
    void touch(const timestamp& o) { stamp_ = o.stamp_; }

    void reset() { stamp_ = 0; }
    bool is_set() const { return !!stamp_; }

    bool is_newer_than(const timestamp& o) const { return stamp_ > o.stamp_; }
    bool is_older_than(const timestamp& o) const { return stamp_ < o.stamp_; }

    bool operator==(const timestamp& o) const { return stamp_ == o.stamp_; }
    bool operator<(const timestamp& o) const { return stamp_ < o.stamp_; }
    bool operator>(const timestamp& o) const { return stamp_ > o.stamp_; }

    // returns most recent
    timestamp  operator|(const timestamp& o) const { return stamp_ > o.stamp_ ? stamp_ : o.stamp_; }
    timestamp& operator|=(const timestamp& o)
    {
        *this = *this | o;
        return *this;
    }

    uint64_t get() const { return stamp_; }

    timestamp get_timestamp() const { return *this; }

    template<class T, class V>
    bool set_with_ts(T& var, V&& val)
    {
        if (gtl::change(var, std::forward<V>(val))) {
            this->touch();
            return true;
        }
        return false;
    }

private:
    uint64_t               stamp_;
    static inline uint64_t clock_ = 0;
};

// ---------------------------------------------------------------------------
// A baseclass (using CRTP) for classes providing get_timestamp()
// ---------------------------------------------------------------------------
template<class T>
class provides_timestamp
{
public:
    template<class TS>
    bool is_newer_than(const TS& o) const
    {
        return static_cast<const T*>(this)->get_timestamp() > o.get_timestamp();
    }

    template<class TS>
    bool is_older_than(const TS& o) const
    {
        return static_cast<const T*>(this)->get_timestamp() < o.get_timestamp();
    }

    // returns most recent
    template<class TS>
    timestamp operator|(const TS& o) const
    {
        return static_cast<const T*>(this)->get_timestamp() | o.get_timestamp();
    }
};

} // namespace gtl

#endif // gtl_utils_hpp_guard
