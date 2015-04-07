// Copyright (C) 2013 The University of Michigan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors - Chun-Hung Hsiao (chhsiao@umich.edu)
//

#ifndef UTILITY_H
#define UTILITY_H

#include <functional>
#include <iterator>
#include <functional>
#include <utility>

namespace std { //namespace tr1 {

template<class T>
struct hash<pair<T,T> > : unary_function<pair<T,T>,size_t> {
    inline size_t operator() (const pair<T,T>& x) const { return hash<T>()(x.first) * 2654435761 + hash<T>()(x.second) * 0x6b43a9b5; }
};

} //}

template <class Map>
struct key_iterator : std::iterator<std::bidirectional_iterator_tag,const typename Map::key_type> {
    key_iterator(const typename Map::iterator& i = typename Map::iterator()) : current_(i) { }
    key_iterator(const typename Map::const_iterator& i = typename Map::const_iterator()) : current_(i) { }
    inline const typename Map::key_type& operator* () const { return current_->first; }
    inline const typename Map::key_type* operator-> () const { return &current_->first; }
    inline key_iterator<Map>& operator++ () { ++current_; return *this; }
    inline key_iterator<Map> operator++ (int) { return key_iterator<Map>(current_++); }
    inline key_iterator<Map>& operator-- () { --current_; return *this; }
    inline key_iterator<Map> operator-- (int) { return key_iterator<Map>(current_--); }
    private: typename Map::const_iterator current_;
    friend inline bool operator== (const key_iterator<Map>& lhs, const key_iterator<Map>& rhs) { return lhs.current_ == rhs.current_; }
    friend inline bool operator!= (const key_iterator<Map>& lhs, const key_iterator<Map>& rhs) { return lhs.current_ != rhs.current_; }
};

template <class Map>
struct value_iterator : std::iterator<std::bidirectional_iterator_tag,typename Map::mapped_type> {
    value_iterator(const typename Map::iterator& i = typename Map::iterator()) : current_(i) { }
    inline typename Map::mapped_type& operator* () const { return current_->second; }
    inline typename Map::mapped_type* operator-> () const { return &current_->second; }
    inline value_iterator<Map>& operator++ () { ++current_; return *this; }
    inline value_iterator<Map> operator++ (int) { return value_iterator<Map>(current_++); }
    inline value_iterator<Map>& operator-- () { --current_; return *this; }
    inline value_iterator<Map> operator-- (int) { return value_iterator<Map>(current_--); }
    private: typename Map::iterator current_;
    friend inline bool operator== (const value_iterator<Map>& lhs, const value_iterator<Map>& rhs) { return lhs.current_ == rhs.current_; }
    friend inline bool operator!= (const value_iterator<Map>& lhs, const value_iterator<Map>& rhs) { return lhs.current_ != rhs.current_; }
};

template <class Base, class T>
struct mem_fun_compare_t : std::binary_function<T,T,int> {
    explicit mem_fun_compare_t(Base* b, int (Base::*m)(const T&, const T&) const) : base_(b), mem_fun_(m) { }
    inline int operator() (const T& x, const T& y) const { return (base_->*mem_fun_)(x, y); }
    private: Base* base_; int (Base::*mem_fun_)(const T&, const T&) const;
};

template <class Op1, class Op2>
struct binary_compose : std::binary_function<typename Op1::first_argument_type, typename Op1::second_argument_type, typename Op2::result_type> {
    explicit binary_compose(const Op1& f, const Op2& g) : op1(f), op2(g) { }
    inline typename Op2::result_type operator() (const typename Op1::first_argument_type& x, const typename Op1::second_argument_type& y) const { return op2(op1(x, y)); }
    private: Op1 op1; Op2 op2;
};

template <class Base, class T>
inline binary_compose<mem_fun_compare_t<Base,T>,std::binder2nd<std::less<int> > > mem_fun_less(Base* b, int (Base::*m)(const T&, const T&) const) {
    return binary_compose<mem_fun_compare_t<Base,T>,std::binder2nd<std::less<int> > >(mem_fun_compare_t<Base,T>(b, m), std::bind2nd(std::less<int>(), 0));
}

template <class Tag>
struct mem_accessor {
    static typename Tag::type ptr;
};

template <class Tag>
typename Tag::type mem_accessor<Tag>::ptr;

template <class Tag, typename Tag::type p>
struct mem_tagger {
    mem_tagger() { mem_accessor<Tag>::ptr = p; }
    static mem_tagger tagger;
};

template <class Tag, typename Tag::type p>
mem_tagger<Tag, p> mem_tagger<Tag, p>::tagger;

#endif
