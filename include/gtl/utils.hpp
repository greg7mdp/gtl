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

#include <utility>

// ---------------*** U N T E S T E D ***-------------------------------------

namespace gtl {

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
template<class Unset>
class scoped_set_unset
{
public:
    template<class Set>
    scoped_set_unset(Set&& set, Unset&& unset, bool do_it = true)
        : do_it_(do_it)
    {
        if (do_it_) {
            unset = std::move(unset);
            std::forward<Set>(set)();
        }
    }

    ~scoped_set_unset()
    {
        if (do_it_)
            unset_();
    }

    scoped_set_unset(const scoped_set_unset&)             = delete;
    scoped_set_unset& operator=(const scoped_se1t_unset&) = delete;

private:
    Unset unset_;
    bool  do_it_;
};

// ---------------------------------------------------------------------------
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
// assigns val to var, and return true if the value changed
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
// assigns val to var, and return the previous value
// ---------------------------------------------------------------------------
template<class T, class V>
T replace(T& var, V&& val) noexcept
{
    T old = std::move(var);
    var   = std::forward<V>(val);
    return old;
}

} // namespace gtl

#endif // gtl_utils_hpp_guard
