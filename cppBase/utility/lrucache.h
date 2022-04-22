#pragma once

#include <list>
#include <unordered_map>
#include "util_common.h"
UTILITY_NAMESPACE_BEGIN

// 代码来源：https://github.com/lamerman/cpp-lru-cache
template<typename key_t, typename value_t>
class lru_cache {
public:
    typedef typename std::pair<key_t, value_t> key_value_pair_t;
    typedef typename std::list<key_value_pair_t>::iterator iterator;
    typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;
    typedef typename std::unordered_map<key_t, list_iterator_t> map_t;
    typedef typename map_t::iterator map_iterator_t;

    lru_cache(size_t capacity) : _capacity(capacity) {}

    void put(const key_t &key, const value_t &value) {
        map_iterator_t it = _map.find(key);
        _list.push_front(key_value_pair_t(key, value));
        if (it != _map.end()) {
            _list.erase(it->second);
            _map.erase(it);
        }
        _map[key] = _list.begin();

        if (_map.size() > _capacity) {
            list_iterator_t last = _list.end();
            last--;
            _map.erase(last->first);
            _list.pop_back();
        }
    }

    const value_t &get(const key_t &key) {
        map_iterator_t it = _map.find(key);
        if (it == _map.end()) {
            throw std::range_error("There is no such key in cache");
        } else {
            _list.splice(_list.begin(), _list, it->second);
            return it->second->second;
        }
    }

    iterator get_iter(const key_t &key) {
        map_iterator_t it = _map.find(key);
        if (it == _map.end()) {
            return _list.end();
        } else {
            _list.splice(_list.begin(), _list, it->second);
            return it->second;
        }
    }

    iterator find(const key_t &key) {
        map_iterator_t it = _map.find(key);
        if (it == _map.end()) {
            return _list.end();
        } else {
            return it->second;
        }
    }

    bool exists(const key_t &key) const {
        return _map.find(key) != _map.end();
    }

    size_t erase(const key_t &key) {
        map_iterator_t it = _map.find(key);
        if (it == _map.end()) {
            return 0;
        } else {
            _map.erase(it);
            _list.erase(it->second);
            return 1;
        }
    }

    void clear() {
        _map.clear();
        _list.clear();
    }

    size_t size() const {
        return _map.size();
    }

    iterator begin() {
        return _list.begin();
    }

    iterator end() {
        return _list.end();
    }

private:
    std::list<key_value_pair_t> _list;
    map_t _map;
    const size_t _capacity;
};

UTILITY_NAMESPACE_END