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

namespace gtl {

// ---------------------------------------------------------------------------
// An object which calls a lambda in its constructor, and another one in
// its destructor
//
// This object must be captured in a local variable, Otherwise, since it is a
// temporary, it will be destroyed immediately, thus calling the unset function.
//
//          scoped_set_unset rollback(...); // good
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

    void dismiss() noexcept { do_it_ = false; }

    scoped_set_unset(const scoped_set_unset&)            = delete;
    scoped_set_unset& operator=(const scoped_set_unset&) = delete;
    void*             operator new(std::size_t)          = delete;

private:
    Unset unset_;
    bool  do_it_;
};

// ---------------------------------------------------------------------------
// An object which calls a lambda in its constructor, and another one in
// its destructor
//
// This object must be captured in a local variable, Otherwise, since it is a
// temporary, it will be destroyed immediately, thus calling the function.
//
//          scoped_guard rollback(...); // good
// ---------------------------------------------------------------------------
template<class F>
class scoped_guard
{
public:
    scoped_guard(F&& unset, bool do_it = true)
        : do_it_(do_it)
        , unset_(std::move(unset))
    {
    }

    ~scoped_guard()
    {
        if (do_it_)
            unset_();
    }

    void dismiss() noexcept { do_it_ = false; }

    scoped_guard(const scoped_guard&)            = delete;
    scoped_guard& operator=(const scoped_guard&) = delete;
    void*         operator new(std::size_t)      = delete;

private:
    F    unset_;
    bool do_it_;
};

// ---------------------------------------------------------------------------
// An object which assigns a value to a variable in its constructor, and resets
// the previous its destructor
//
// This object must be captured in a local variable, Otherwise, since it is a
// temporary, it will be destroyed immediately, thus reverting to the old value
// immediately
//
//        scoped_set_value rollback(retries, 7); // good
// ---------------------------------------------------------------------------
template<class T>
class scoped_set_value
{
public:
    template<class V>
    scoped_set_value(T& var, V&& val, bool do_it = true) noexcept(
        std::is_nothrow_copy_constructible_v<T>&& std::is_nothrow_move_assignable_v<T>&&
            std::is_nothrow_move_assignable_v<V>&& std::is_nothrow_copy_assignable_v<V>)
        : v_(var)
        , do_it_(do_it)
    {
        if (do_it_) {
            old_value_ = std::move(v_);
            v_         = std::forward<V>(val);
        }
    }

    ~scoped_set_value() noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        if (do_it_)
            v_ = std::move(old_value_);
    }

    void dismiss() noexcept { do_it_ = false; }

    scoped_set_value(const scoped_set_value&)            = delete;
    scoped_set_value& operator=(const scoped_set_value&) = delete;
    void*             operator new(std::size_t)          = delete;

    T&   v_;
    T    old_value_;
    bool do_it_;
};

#if 0
// ---------------------------------------------------------------------------
// scope_guard
// derived (with modifications) from Folly (https://github.com/facebook/folly)
//  Licensed under the Apache License, Version 2.0
// ---------------------------------------------------------------------------
namespace detail {

class scope_guard_impl_base
{
public:
    void dismiss() noexcept { dismissed_ = true; }
    void rehire() noexcept { dismissed_ = false; }

protected:
    scope_guard_impl_base(bool dismissed = false) noexcept
        : dismissed_(dismissed)
    {
    }

    [[noreturn]] static void     terminate() noexcept;
    static scope_guard_impl_base make_empty_scope_guard() noexcept
    {
        return scope_guard_impl_base{};
    }

    template<typename T>
    static const T& asConst(const T& t) noexcept
    {
        return t;
    }

    bool dismissed_;
};

template<typename FunctionType>
class scope_guard_impl : public scope_guard_impl_base
{
public:
    explicit scope_guard_impl(FunctionType& fn) noexcept(
        std::is_nothrow_copy_constructible<FunctionType>::value)
        : scope_guard_impl(asConst(fn),
                           makeFailsafe(std::is_nothrow_copy_constructible<FunctionType>{}, &fn))
    {
    }

    explicit scope_guard_impl(const FunctionType& fn) noexcept(
        std::is_nothrow_copy_constructible<FunctionType>::value)
        : scope_guard_impl(fn,
                           makeFailsafe(std::is_nothrow_copy_constructible<FunctionType>{}, &fn))
    {
    }

    explicit scope_guard_impl(FunctionType&& fn) noexcept(
        std::is_nothrow_move_constructible<FunctionType>::value)
        : scope_guard_impl(std::move_if_noexcept(fn),
                           makeFailsafe(std::is_nothrow_move_constructible<FunctionType>{}, &fn))
    {
    }

    scope_guard_impl(scope_guard_impl&& other) noexcept(
        std::is_nothrow_move_constructible<FunctionType>::value)
        : function_(std::move_if_noexcept(other.function_))
    {
        // If the above line attempts a copy and the copy throws, other is
        // left owning the cleanup action and will execute it (or not) depending
        // on the value of other.dismissed_. The following lines only execute
        // if the move/copy succeeded, in which case *this assumes ownership of
        // the cleanup action and dismisses other.
        dismissed_ = std::exchange(other.dismissed_, true);
    }

    ~scope_guard_impl() noexcept
    {
        if (!dismissed_)
            execute();
    }

private:
    static scope_guard_impl_base makeFailsafe(std::true_type, const void*) noexcept
    {
        return make_empty_scope_guard();
    }

    template<typename Fn>
    static auto makeFailsafe(std::false_type, Fn* fn) noexcept
        -> scope_guard_impl<decltype(std::ref(*fn))>
    {
        return scope_guard_impl<decltype(std::ref(*fn))>{ std::ref(*fn) };
    }

    template<typename Fn>
    explicit scope_guard_impl(Fn&& fn, scope_guard_impl_base&& failsafe)
        : scope_guard_impl_base{}
        , function_(std::forward<Fn>(fn))
    {
        failsafe.dismiss();
    }

    void* operator new(std::size_t) = delete;

    void execute() noexcept
    {
        if constexpr (false) {
            using R           = decltype(function_());
            auto catcher_word = reinterpret_cast<uintptr_t>(&terminate);
            auto catcher      = reinterpret_cast<R (*)()>(catcher_word);
            catch_exception(function_, catcher);
        } else {
            function_();
        }
    }

    FunctionType function_;
};

template<typename F>
using scope_guard_impl_decay = scope_guard_impl<typename std::decay<F>::type>;

} // namespace detail

/**
 * Create a scope guard.
 *
 * The returned object has methods .dismiss() and .rehire(), which will
 * deactivate/reactivate the calling of the function upon destruction.
 *
 * The return value of this function must be captured. Otherwise, since it is a
 * temporary, it will be destroyed immediately, thus calling the function.
 *
 *     auto guard = makeScopeGuard(...); // good
 *
 *     makeScopeGuard(...); // bad
 *
 */
template<typename F>
[[nodiscard]] detail::scope_guard_impl_decay<F> make_guard(F&& f) noexcept(
    noexcept(detail::scope_guard_impl_decay<F>(static_cast<F&&>(f))))
{
    return detail::scope_guard_impl_decay<F>(static_cast<F&&>(f));
}
#endif

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
// ---------------------------------------------------------------------------
template<class... T>
struct always_false : std::false_type
{
};

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
