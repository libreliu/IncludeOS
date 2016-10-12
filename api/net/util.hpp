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

#ifndef NET_UTIL_HPP
#define NET_UTIL_HPP

#include <cstdint>

namespace net {

  /*
   * See P.49 of C programming
   * Get "n" bits from integer "x", starting from position "p"
   * e.g., getbits(x, 31, 8) -- highest byte
   *       getbits(x,  7, 8) -- lowest  byte
   */
  template<typename T>
  constexpr inline auto getbits(T&& x, T&& p, T&& n) noexcept {
    return (x >> ((p + 1) - n)) & ~(~0 << n);
  }

  constexpr inline uint16_t ntohs(const uint16_t n) noexcept {
    return __builtin_bswap16(n);
  }

  constexpr inline uint16_t htons(const uint16_t n) noexcept {
    return __builtin_bswap16(n);
  }

  constexpr inline uint32_t ntohl(const uint32_t n) noexcept {
    return __builtin_bswap32(n);
  }

  constexpr inline uint32_t htonl(const uint32_t n) noexcept {
    return __builtin_bswap32(n);
  }

  constexpr inline uint64_t ntohll(const uint64_t n) noexcept {
    return __builtin_bswap64(n);
  }

  constexpr inline uint64_t htonll(const uint64_t n) noexcept {
    return __builtin_bswap64(n);
  }
  
} //< namespace net

#endif //< NET_UTIL_HPP
