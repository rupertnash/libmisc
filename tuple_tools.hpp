// -*- mode: c++; -*-
// 
// Copyright (C) 2018 Rupert Nash, The University of Edinburgh
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef LIBMISC_TUPLE_TOOLS_H
#define LIBMISC_TUPLE_TOOLS_H

#include <tuple>

// 
// Metaprogramming helpers for tuples
//


// Invoke the unary function object with a reference to each element
// of the tuple, discarding any result.
//
// Not a million miles from std::for_each
template <typename Tuple, typename UnaryFunction>
constexpr void tuple_for_each(Tuple&& t, UnaryFunction&& f);



// Invoke the unary function object with a const reference to each
// element of the tuple, creating a new tuple with the result of each
// invocation as the result.
template <typename TupleT, typename FuncT>
constexpr auto tuple_map(const TupleT& t, FuncT&&f);



// Get the index of the first element who's type is SoughtT.
// Result stored in static member value.
template<typename SoughtT, typename TupleT>
struct tuple_index_of;



// Metafunction map
// 
// Apply the unary metafunction to each type of the tuple in turn,
// creating a tuple of the result types (i.e. the member typedef
// `type` of each application) as our result.
//
// Resulting type is in the member `type`.
template <typename TupleT, template< typename > class MetaFuncT>
struct tuple_meta_map;



// Common specialisations of tuple_meta_map

// Metafunction for adding a reference qualifier to every type in a
// tuple.
template <typename T >
using tuple_add_ref = tuple_meta_map<T, std::add_lvalue_reference>;

// Metafunction for adding a const qualifier to every type in a
// tuple.
template <typename T >
using tuple_add_const = tuple_meta_map<T, std::add_const>;

// Metafunction for adding a reference to const qualifier to every
// type in a tuple.
template <typename T >
using tuple_add_const_ref = tuple_add_ref< typename tuple_add_const<T>::type >;



// Metafunction to compute the type of a tuple containing `N` elements
// of the same type `T`.
template <typename T, int N>
struct NTuple;

// Function to convert a tuple `t` into a tuple of references to the
// elements of `t`.
template <typename TupleT>
auto tuple_references(TupleT&& t);

// 
// Implementations begin
// 

namespace detail {
  // Implementation details

  template <typename TupleT, typename FuncT, size_t... I>
  constexpr void tuple_for_each_impl(TupleT& t, FuncT f, std::index_sequence<I...>) {
    (f(std::get<I>(t)),...);
  }

  template <typename TupleT, typename FuncT, size_t... I>
  constexpr auto tuple_map_impl(const TupleT& t, FuncT f, std::index_sequence<I...>) {
    return std::make_tuple(f(std::get<I...>(t)));
  }

  template <typename TupleT, template< typename > class MetaFuncT, size_t... I>
  auto tuple_meta_map_impl(std::index_sequence<I...>) -> std::tuple< typename MetaFuncT<std::tuple_element_t<I, TupleT> >::type... >;
  template <typename TupleRefT, typename TupleT, size_t... I>
  TupleRefT tref_impl(TupleT& t, std::index_sequence<I...>) {
    return TupleRefT{std::get<I>(t)...};
  }

}


template <typename Tuple, typename UnaryFunction>
constexpr void tuple_for_each(Tuple&& t, UnaryFunction&& f) {
  using indices = std::make_index_sequence< std::tuple_size_v<std::remove_reference_t<Tuple>>>;
  detail::tuple_for_each_impl(std::forward<Tuple>(t), std::forward<UnaryFunction>(f), indices{});
}


template <typename TupleT, typename FuncT>
constexpr auto tuple_map(const TupleT& t, FuncT&&f) {
  using indices = std::make_index_sequence< std::tuple_size_v<std::remove_reference_t<TupleT>>>;
  return detail::tuple_map_impl(t, std::forward<FuncT>(f));
}

template <typename TupleT, template< typename > class MetaFuncT>
struct tuple_meta_map {
  using indices = std::make_index_sequence<std::tuple_size_v<TupleT>>;
  using type = decltype(detail::tuple_meta_map_impl<TupleT, MetaFuncT>(indices{}));
};

// tuple_index_of implementation

// Specialisation for head type matches (i.e. success)
template<typename SoughtT, typename... ElemTs>
struct tuple_index_of<SoughtT, std::tuple<SoughtT, ElemTs...> > {
  static constexpr std::size_t value = 0;
};
// Specialisation for head type not matching (i.e. recurse into later elems)
template<typename SoughtT, typename OtherT, typename... ElemTs>
struct tuple_index_of<SoughtT, std::tuple<OtherT, ElemTs...> > {
  static constexpr std::size_t value = 1 +
    tuple_index_of< SoughtT, std::tuple<ElemTs...> >::value;
};


// NTuple implementation
// General case
template <typename T, int N>
struct NTuple {
  using type =  decltype(std::tuple_cat(typename NTuple<T, N-1>::type{}, std::tuple<T>{}));
};
// Terminating case
template <typename T>
struct NTuple<T, 1> {
  using type = std::tuple<T>;
};


template <typename TupleT>
auto tuple_references(TupleT&& t) {
  using indices = std::make_index_sequence< std::tuple_size_v<std::remove_reference_t<TupleT>>>;
  using TupleRefT = typename tuple_add_ref<TupleT>::type;
  return detail::tref_impl<TupleRefT>(std::forward<TupleT>(t), indices{});
}


#endif
