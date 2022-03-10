/*
 *  Copyright (c) 2014, lamerman
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  
 *  * Neither the name of lamerman nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Author: Alexander Ponomarev
 *
 * Modified by Gregory Popovitch
 *
 */

#ifndef gtl_lru_cache_h_
#define gtl_lru_cache_h_

#include <cstddef>
#include <cassert>
#include <list>
#include <gtl/phmap.hpp>

namespace gtl {

template<class K, class V, class Hash = std::hash<K>, class Eq = std::equal_to<K> >
class lru_cache 
{
public:
    using value_type = typename std::pair<const K, V>;
    using list_iter  = typename std::list<value_type>::iterator;

    explicit lru_cache(size_t max_size = 100) 
    {
        assert(max_size >= 1);
        setCacheSize(max_size);
    }

    void setCacheSize(size_t max_size) 
    {
        _max_size = max_size; 
        while (_cache_items_map.size() > _max_size) 
            removeOldest();
    }

    void removeOldest()
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
            removeOldest();

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

}  // namespace gtl

#endif  // gtl_lru_cache_h_
