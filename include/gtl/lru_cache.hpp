#ifndef gtl_lru_cache_h_
#define gtl_lru_cache_h_

#include <cstddef>
#include <cassert>
#include <list>
#include <tuple>
#include <gtl/phmap.hpp>

namespace gtl {

// ------------------------------------------------------------------------------
// Original author: Alexander Ponomarev (see licenses/license_lamerman)
// Modified by Gregory Popovitch
// ------------------------------------------------------------------------------
template<class K, class V, class Hash = gtl::Hash<K>, class Eq = std::equal_to<K>>
class lru_cache 
{
public:
    using value_type = typename std::pair<const K, V>;
    using list_iter  = typename std::list<value_type>::iterator;

    explicit lru_cache(size_t max_size = 128) 
    {
        assert(max_size >= 1);
        set_cache_size(max_size);
    }

    void set_cache_size(size_t max_size) 
    {
        _max_size = max_size; 
        while (_cache_items_map.size() > _max_size) 
            remove_oldest();
    }

    void remove_oldest()
    {
        if (!_cache_items_map.empty())
        {
            assert(!_cache_items_list.empty());

            auto last = _cache_items_list.end();
            last--;
            _cache_items_map.erase(last->first);
            _cache_items_list.pop_back();
        }
    }
	
    template <class Val>
    V *insert(const K& key, Val&& value) 
    {
        auto it = _cache_items_map.find(key);
        
        if (it != _cache_items_map.end()) {
            _cache_items_list.erase(it->second);
            _cache_items_map.erase(it);
        }
			
        _cache_items_list.push_front(value_type(key, std::forward<Val>(value)));
        _cache_items_map[key] = _cache_items_list.begin();
		
        if (_cache_items_map.size() > _max_size) 
            remove_oldest();

        return &(_cache_items_list.begin()->second);
    }
	
    V *get(const K& key) 
    {
        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end()) 
            return nullptr;
        
        _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
        return &(it->second->second);
    }
	
    bool exists(const K& key) const noexcept {
        return _cache_items_map.find(key) != _cache_items_map.end();
    }
	
    size_t size() const noexcept {
        return _cache_items_map.size();
    }

    void clear() noexcept {
        _cache_items_map.clear();
        _cache_items_list.clear();
    }
	
private:
    std::list<value_type> _cache_items_list;
    gtl::flat_hash_map<K, list_iter, Hash, Eq> _cache_items_map;
    size_t _max_size;
};

// ------------------------------------------------------------------------------
template<typename... Ts> struct pack { };

template <class> struct pack_helper;

template <class R, class... Args> 
struct pack_helper<R(Args...)> {
    using args = pack<Args...>;
};

template <class R, class... Args> 
struct pack_helper<R(*)(Args...)> {
    using args = pack<Args...>;
};

// ---- for lambdas
template <class T> 
struct pack_helper : public pack_helper<decltype(&T::operator())>
{};

template <class LambdaClass, class R, class... Args> 
struct pack_helper<R(LambdaClass::*)(Args...) const> {
    using args = pack<Args...>;
};

// ------------------------------------------------------------------------------
// Author:  Gregory Popovitch (greg7mdp@gmail.com)
// 
// Given a callable object (often a function), this class provides a new 
// callable which either invokes the original one, caching the returned value,
// or returns the cached returned value if the arguments match a previous call.
// Of course this should be used only for pure functions without side effects.
//
// This version only keeps a limited number of results in the hash map,
// configurable with set_max_size(). default max_size = 128
// ------------------------------------------------------------------------------
template <class F, class = typename pack_helper<F>::args> 
class memoize_lru;

template <class F, class... Args>
class memoize_lru<F, pack<Args...>>
{
public:
    using key_type = std::tuple<Args...>;
    using result_type = std::invoke_result_t<F, Args...>;

    memoize_lru(F &&f) : _f(std::forward<F>(f)) {}

    result_type* cache_hit(Args... args) { 
        key_type key(args...);
        return _cache.get(key);
    }
    
    result_type operator()(Args... args) { 
        key_type key(args...);
        auto hit = _cache.get(key);
        if (hit)
            return *hit;
        auto res =  _f(args...); 
        _cache.insert(key, res);
        return res;
    }

    void set_max_size(size_t sz) const { _cache.set_cache_size(sz); }
    size_t size() const { return _cache.size(); }
    void clear() { _cache.clear(); }

private:
    F _f;
    lru_cache<key_type, result_type>  _cache;
};

// ------------------------------------------------------------------------------
// Author:  Gregory Popovitch (greg7mdp@gmail.com)
// 
// Given a callable object (often a function), this class provides a new 
// callable which either invokes the original one, caching the returned value,
// or returns the cached returned value if the arguments match a previous call.
// Of course this should be used only for pure functions without side effects.
//
// This version keeps all unique  results in the hash map.
// ------------------------------------------------------------------------------
template <class F, class = typename pack_helper<F>::args> 
class memoize;

// template <class F, class = typename pack_helper<decltype(&F::operator())>::args> 
// class memoize;

template <class F, class... Args>
class memoize<F, pack<Args...>>
{
public:
    using key_type = std::tuple<Args...>;
    using result_type =  std::invoke_result_t<F, Args...>; // or decltype(std::declval<F>()(std::declval<Args>()...));

    memoize(F &&f) : _f(std::forward<F>(f)) {}
    
    result_type* cache_hit(Args... args) { 
        key_type key(args...);
        auto it = _cache.find(key);
        if (it != _cache.end())
            return &it->second;
        return nullptr;
    }

    result_type operator()(Args... args) { 
        key_type key(args...);
        auto it = _cache.find(key);
        if (it != _cache.end())
            return it->second;
        auto res =  _f(args...); 
        _cache.emplace(key, res);
        return res;
    }

    void clear() { _cache.clear(); }

private:
    F _f;
    gtl::flat_hash_map<key_type, result_type>  _cache;
};

#if 0
// ------------------------------------------------------------------------------
// Author:  Gregory Popovitch (greg7mdp@gmail.com)
// ------------------------------------------------------------------------------
template <class T, class F> 
class lazy_list 
{
    lazy_list(T first, F next) : 
        _first(std::move(first)), 
        _next(std::move(next)),
    {}

    const T& operator[](size_t idx) {
        if (idx == 0)
            return _first;

        const T&
        
    }
        
private:
    using mem_next = T (*)(size_t idx, const T& first, F&& next);

    static T _item(size_t idx, const T& first, F&& next) {
        if (idx == 0)
            return first;

    }

    template <class F>
    static T logify_recursion(F &f, size_t start, size_t end) {
        if (start == 0) {
            auto hit = f.cache_hit(end);
            if (hit)
                return *hit;
        }

        size_t width = end - start;
        if (width > 64) {
            (void)f(start + width / 2);
            return logify_recursion(f, start + width / 2, end);
        }

        return f(end);
    }

    T   _first;
    F   _next; 
    T   _memoized_item;
};
#endif

}  // namespace gtl

#endif  // gtl_lru_cache_h_
