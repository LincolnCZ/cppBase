#pragma once
#include <map>
#include <set>
#include "util_common.h"

UTILITY_NAMESPACE_BEGIN

// iterator for map only point to key
template<class map_type>
class key_iterator : public map_type::iterator
{
public:
    typedef typename map_type::iterator map_iterator;
    typedef typename map_iterator::value_type::first_type key_type;

    key_iterator(const map_iterator& other) : map_type::iterator(other) {} ;

    key_type& operator *()
    {
        return map_type::iterator::operator*().first;
    }
};

// helpers to create iterators easier:
template<class map_type>
key_iterator<map_type> key_begin(map_type& m)
{
    return key_iterator<map_type>(m.begin());
}
template<class map_type>
key_iterator<map_type> key_end(map_type& m)
{
    return key_iterator<map_type>(m.end());
}

template<typename T1, typename T2>
std::set<T1> map_keyset(const std::map<T1,T2> &m)
{
    std::set<T1> s;
    typename std::map<T1,T2>::const_iterator it = m.begin();
    while(it != m.end()) {
        s.insert(it->first);
        ++it;
    }
    return s;
}

UTILITY_NAMESPACE_END
