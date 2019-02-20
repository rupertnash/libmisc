// -*- mode: c++; -*-
// 
// Copyright (c) 2019, Rupert Nash
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
#ifndef LIBMISC_ELEMENT_ITERATOR_HPP
#define LIBMISC_ELEMENT_ITERATOR_HPP

// Bare-bones iterator transformer. It wraps an iterator that when
// dereferenced can be accessed with std::get<unsigned> and will
// return the corresponding element when dereferenced.
//
// Useful to allow, for example, easy interation over the values or
// keys of a map.
//
// Base iterator B must implement LegacyIterator and LegacyForwardIterator.
//
// If B implements LegacyBidirectionalIterator then so does this.
template <typename B, std::size_t index>
struct element_iterator {
  // Support iterator_traits
  using value_type = typename std::decay<decltype(std::get<index>(*B()))>::type;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = typename std::iterator_traits<B>::difference_type;
  // This is probably wrong
  using iterator_category = typename std::iterator_traits<B>::iterator_category;

  using base_iterator = B;

  // Pre increment 
  element_iterator& operator++() {
    ++base;
    return *this;
  }
  // Post increment
  element_iterator operator++(int) {
    return element_iterator{base++};
  }

  // Pre decrement
  element_iterator& operator--() {
    static_assert(std::is_same<decltype(--B() ), B&>::value,
		  "base iterator does implement operator--() as standard requires");
    --base;
    return *this;
  }
  // Post decrement
  element_iterator operator--(int) {
    static_assert(std::is_same<decltype(--B() ), B&>::value,
		  "base iterator does implement operator--() as standard requires");
    return element_iterator{base++};
  }
  // Dereference
  reference operator*() {
    return std::get<index>(*base);
  }

  // Could make this private but I'm lazy
  base_iterator base;
};

template <typename B, std::size_t index>
bool operator!=(const element_iterator<B, index>& a, const element_iterator<B, index>& b) {
  return a.base != b.base;
}
template <typename B, std::size_t index>
bool operator==(const element_iterator<B, index>& a, const element_iterator<B, index>& b) {
  return a.base == b.base;
}

#endif
