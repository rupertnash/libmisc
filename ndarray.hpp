// 
// Copyright (c) 2020, Rupert Nash
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

#ifndef LIBMISC_NDARRAY_HPP
#define LIBMISC_NDARRAY_HPP

#include <array>
#include <memory>
#include <numeric>

// This supplies a class template `ndarray` for simple N-dimnesional arrays, see below. Other classes are 

namespace detail {
  // Iterator for ND arrays (below)
  //
  // Template parameter can either be the array or a const-qualified array.
  //
  // This should be a random access iterator, but isn't (yet).
  //
  // The only extra this gives you over a flat iterator is you can call
  // `index()` on an instance to get the current n-dimensional point
  // being pointed to.
  template <typename AT>
  class nd_iter_impl {
  public:
    using array_type = AT;
    using base_array_type = typename std::remove_cv<AT>::type;
    using reference = decltype(std::declval<array_type>()(std::declval<typename array_type::index_type>()));
    using pointer = decltype(&std::declval<reference>());
    using value_type = typename array_type::value_type;
    using index_type = typename array_type::index_type;
  private:
    array_type* _arr;
    int _pos;

  public:
    nd_iter_impl() = default;
    friend bool operator==(const nd_iter_impl& a, const nd_iter_impl& b) {
      return (a._arr == b._arr) && (a._pos == b._pos);
    }
    friend bool operator!=(const nd_iter_impl& a, const nd_iter_impl& b) {
      return !(a == b);
    }

    reference operator*() {
      return _arr->_data[_pos];
    }
    nd_iter_impl& operator++() {
      ++_pos;
      return *this;
    }

    index_type index() const {
      return _arr->one2n(_pos);
    }

  private:
    friend base_array_type;
    nd_iter_impl(array_type* arr, int pos) : _arr(arr), _pos(pos) {
    }
  };

  // Enumerating ND iterator
  //
  // Template parameter can either be the array or a const-qualified array.
  //
  // Has same constraints as an nd_iter_impl above.
  template <typename AT>
  struct nd_enum_iter {
    using array_type = AT;
    using index_type = typename array_type::index_type;
    using base_iter = nd_iter_impl<AT>;
    using reference = std::pair<typename array_type::index_type, typename base_iter::reference>;

    base_iter _it;

    nd_enum_iter() = default;
    nd_enum_iter(base_iter it) : _it(it) {
    }

    friend bool operator==(const nd_enum_iter& a, const nd_enum_iter& b) {
      return (a._it == b._it);
    }
    friend bool operator!=(const nd_enum_iter& a, const nd_enum_iter& b) {
      return !(a == b);
    }

    reference operator*() {
      return {_it.index(), *_it};
    }
    nd_enum_iter& operator++() {
      ++_it;
      return *this;
    }  
  };

  // An enumerator helper - mainly exists to act as a helper for
  // ranged for loops which want to enumerate an array.
  template <typename AT>
  struct nd_enumerator {
    using array_type = AT;
    using base_iterator = decltype(std::declval<AT>().nd_begin());
    using iterator = nd_enum_iter<AT>;

    AT* _arr;

    nd_enumerator(AT* arr) : _arr(arr) {
    }

    iterator begin() {
      return iterator{_arr->nd_begin()};
    }
    iterator end() {
      return iterator{_arr->nd_end()};
    }
  };
}

// Simple multidimensional array class
// - Contained type must be trivial
// - Initial contents uninitialised
// - Uniquely owns its data
// - Element access via operator()
template <typename T, int N>
class ndarray {
public:
  static constexpr int NDIMS = N;
  using value_type = T;
  using index_type = std::array<int, NDIMS>;

  using iterator = T*;
  using const_iterator = const T*;

  
  using nd_iterator = nd_iter_impl<ndarray>;
  using const_nd_iterator = nd_iter_impl<const ndarray>;

  friend nd_iterator;
  friend const_nd_iterator;
private:
  // Ensure value initialised to zero
  index_type _shape = {};
  // index_type _strides = {};
  int _size = 0;
  std::unique_ptr<T[]> _data;

  // Helper to compute the flat index from the ND index.
  int n2one(const index_type& idx) const {
    //return std::inner_product(_strides.begin(), _strides.end(), idx.begin(), 0);
    int flat = 0;
    for (int d = 0; d <NDIMS; ++d) {
      flat += idx[d] * stride(d);
    }
    return flat;
  }


  // Helper to compute the ND index from the flat.
  index_type one2n(int ijk) const {
    index_type ans;
    // Assumes C-style array layout
    for (int i = 0; i < NDIMS; ++i) {
      ans[i] = ijk / stride(i);
      ijk %= stride(i);
    }
    return ans;
  }

public:
  // Default constructor has size zero along all dimensions and has no
  // data.
  ndarray() = default;

  // Construct for given dimensions (aka an appropriately sized
  // std::array<int>).
  // 
  // Contents uninitialised.
  ndarray(const index_type& shape) : _shape{shape} {
    _size = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<int>());
    _data.reset(new T[_size]);
  }

  // Construct for given dimensions (aka an appropriately sized
  // std::array<int>).
  // 
  // Contents set to the supplied value.
  ndarray(const index_type& shape, const T& val) : ndarray(shape) {
    std::fill(_data.get(), _data.get() + _size, val);
  }

  // Move construct
  ndarray(ndarray&& source) noexcept : ndarray() {
    swap(*this, source);
  }
  // Copy construct
  ndarray(const ndarray& src)
    : _shape(src._shape), _size(src._size),
      _data(src._data ? new T[src._size] : nullptr) {
    std::copy(src._data.get(), src._data.get() + src._size, _data.get());
  }

  // Assign, usual copy/move-and-swap idiom
  ndarray& operator=(ndarray src) {
    swap(*this, src);
    return *this;
  }

  friend void swap(ndarray& a, ndarray& b) noexcept {
    // ADL for array and unique_ptr
    using std::swap;
    swap(a._shape, b._shape);
    swap(a._size, b._size);
    swap(a._data, b._data);
  }

  // Get the shape of the array
  const index_type& shape() const {
    return _shape;
  }
  // Get the size (total number of elements) of the array
  int size() const {
    return _size;
  }

  // Get the stride for the given dimension
  // 
  // Why not store? Because the optimizer can see through this and
  // produce vectorised code.
  int stride(int i) const {
    if (i == NDIMS-1)
      return 1;
    return stride(i+1) * _shape[i+1];
  }
  // Get the array of strides.
  // 
  // Why not store? Because the optimizer can see through this and
  // produce vectorised code.
  index_type strides() const {
    index_type ans;
    for (int d = 0; d <NDIMS; ++d) {
      ans[d] = stride(d);
    }
    return ans;
  }

  // Mutable element access.
  template <typename... Ints>
  auto operator()(Ints... inds) -> typename std::enable_if<sizeof...(Ints) == NDIMS, T&>::type {
    return _data[n2one(index_type{static_cast<int>(inds)...})];
  }
  auto operator()(const index_type& ind) -> T& {
    return _data[n2one(ind)];
  }

  // Constant element access.
  template <typename... Ints>
  auto operator()(Ints... inds) const -> typename std::enable_if<sizeof...(Ints) == NDIMS, const T&>::type {
    return _data[n2one(index_type{static_cast<int>(inds)...})];
  }
  auto operator()(const index_type& ind) const -> const T& {
    return _data[n2one(ind)];
  }

  // Return a new copy of this array.
  ndarray clone() const {
    return ndarray{*this};
  }

  // Flat iterator range access
  iterator begin() {
    return _data.get();
  }
  const_iterator begin() const {
    return _data.get();
  }
  const_iterator cbegin() const {
    return _data.get();
  }
  
  iterator end() {
    return _data.get() + _size;
  }
  const_iterator end() const {
    return _data.get()+ _size;
  }
  const_iterator cend() const {
    return _data.get() + _size;
  }

  // ND-iterator range access
  nd_iterator nd_begin() {
    return nd_iterator(this, 0);
  }
  const_nd_iterator nd_begin() const {
    return const_nd_iterator(this, 0);
  }
  const_nd_iterator nd_cbegin() const {
    return const_nd_iterator(this, 0);
  }
  
  nd_iterator nd_end() {
    return nd_iterator(this, _size);
  }
  const_nd_iterator nd_end() const {
    return const_nd_iterator(this, _size);
  }
  const_nd_iterator nd_cend() const {
    return const_nd_iterator(this, _size);
  }
};

// Create an ND enumerator for the given array.
//
// Particularly nice if you are using C++17 or later:
// 
// for (auto [idx, val]: nd_enumerate(array)) {
//   val = some_function(idx);
// }
//
// NOTE: you need to make sure the lifetime of the array lasts the
// loop i.e. you can't pass a temporary, until C++20 allows:
// for (auto array = ndarray_factory(); auto [idx, val]: nd_enumerate(array)) {
//   //stuff
// }
template <typename AT>
nd_enumerator<AT> nd_enumerate(AT& arr) {
  return nd_enumerator<AT>(&arr);
}

#endif
