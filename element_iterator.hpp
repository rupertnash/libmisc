#ifndef LIBMISC_ELEMENT_ITERATOR_HPP
#define LIBMISC_ELEMENT_ITERATOR_HPP

// Bare-bones iterator transformer. It wraps an iterator that when
// dereferenced can be accessed with std::get<unsigned> and will
// return the corresponding element when dereferenced.
//
// Useful to allow, for example, easy interation over the values or
// keys of a map.
template <typename B, std::size_t index>
struct element_iterator {
  // Support iterator_traits
  using value_type = typename std::decay<decltype(std::get<index>(B()))>::type;
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

  typename std::enable_if<std::is_same<decltype(--B() ), B&>::value, element_iterator&>::type
  operator--() {
    --base;
    return *this;
  }
  
  reference operator*() {
    return std::get<index>(*base);
  }

  base_iterator base;
};

template <typename B>
bool operator!=(const val_iter<B>& a, const val_iter<B>& b) {
  return a.base != b.base;
}
template <typename B>
bool operator==(const val_iter<B>& a, const val_iter<B>& b) {
  return a.base == b.base;
}

#endif
