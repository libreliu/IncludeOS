// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#ifndef UTIL_FIXEDVEC_HPP
#define UTIL_FIXEDVEC_HPP

/**
 * High performance no-heap fixed vector
 * All data is reserved uninitialized
 **/

#include <cstdint>
#include <cstring>

template <typename T, int N>
struct fixedvector {

  enum No_init { UNINITIALIZED };
  fixedvector() : count(0) {}
  fixedvector(No_init) {}

  // add existing
  void add(const T& e) noexcept {
    (*this)[count++] = e;
  }
  // construct into
  template <typename... Args>
  void emplace(Args&&... args) noexcept {
    new (&element[count++]) T(args...);
  }

  // pop back and return last element
  T pop() {
    return (*this)[--count];
  }
  // clear whole thing
  void clear() noexcept {
    count = 0;
  }

  bool empty() const noexcept {
    return count == 0;
  }
  uint32_t size() const noexcept {
    return count;
  }

  T& operator[] (uint32_t i) noexcept {
    return *(T*) (element + i);
  }

  T* begin() noexcept {
    return (T*) &element[0];
  }
  T* end() noexcept {
    return (T*) &element[count];
  }

  constexpr int capacity() const noexcept {
    return N;
  }
  bool free_capacity() const noexcept {
    return count < N;
  }

  // overwrite this element buffer with a buffer from another
  // source of the same type T, with @size elements
  // Note: size and capacity are not related, and they don't have to match
  void copy(T* src, uint32_t size) {
    memcpy(element, src, size * sizeof(T));
    count = size;
  }

private:

  // NOTE: We can't default initialize here due to plugins / global ctors
  uint32_t count;
  typename std::aligned_storage<sizeof(T), alignof(T)>::type element[N];
};


#endif
