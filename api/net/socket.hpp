// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2017 Oslo and Akershus University College of Applied Sciences
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
#ifndef NET_SOCKET_HPP
#define NET_SOCKET_HPP

#include <net/addr.hpp>

namespace net {

/**
 * An IP address and port
 */
class Socket {
public:
  using Address = net::Addr;
  using port_t = uint16_t;

  /**
   * Constructor
   *
   * Intialize an empty socket <0.0.0.0:0>
   */
  constexpr Socket() noexcept
    : Socket{0}
  {}

  explicit constexpr Socket(const port_t port) noexcept
    : addr_{}, port_{port}
  {}

  /**
   * Constructor
   *
   * Create a socket with an address and port
   *
   * @param address
   *  The host's network address
   *
   * @param port
   *  The port associated with the process
   */
  Socket(Address address, const port_t port) noexcept
    : addr_{std::move(address)},
      port_{port}
  {}

  Socket(const Socket& other) noexcept = default;
  Socket(Socket&& other) noexcept
    : addr_{std::move(other.addr_)}, port_{other.port_}
  {}
  Socket& operator=(const Socket& other) noexcept = default;
  Socket& operator=(Socket&& other) noexcept
  {
    addr_ = std::move(other.addr_);
    port_ = other.port_;
    return *this;
  }

  /**
   * Get the socket's network address
   *
   * @return The socket's network address
   */
  const Address& address() const noexcept
  { return addr_; }

  /**
   * Get the socket's port value
   *
   * @return The socket's port value
   */
  constexpr port_t port() const noexcept
  { return port_; }

  /**
   * Get a string representation of this class
   *
   * @return A string representation of this class
   */
  std::string to_string() const {
    return (addr_.is_v6()) ? "[" + addr_.to_string() + "]:" + std::to_string(port_)
      : addr_.to_string() + ":" + std::to_string(port_);
  }

  /**
   * Check if this socket is empty <0.0.0.0:0>
   *
   * @return true if this socket is empty, false otherwise
   */
  bool is_empty() const noexcept
  { return (addr_.v6() == ip6::Addr::link_unspecified) and (port() == 0); }

  /**
   * Operator to check for equality relationship
   *
   * @param other
   *  The socket to check for equality relationship
   *
   * @return true if the specified socket is equal, false otherwise
   */
  bool operator==(const Socket& other) const noexcept
  {
    return addr_ == other.addr_ and port_ == other.port_;
  }

  /**
   * Operator to check for inequality relationship
   *
   * @param other
   *  The socket to check for inequality relationship
   *
   * @return true if the specified socket is not equal, false otherwise
   */
  bool operator!=(const Socket& other) const noexcept
  { return not (*this == other); }

  /**
   * Operator to check for less-than relationship
   *
   * @param other
   *  The socket to check for less-than relationship
   *
   * @return true if this socket is less-than the specified socket,
   * false otherwise
   */
  bool operator<(const Socket& other) const noexcept
  {
    return (addr_ < other.addr_)
        or ((addr_ == other.addr_) and (port_ < other.port_));
  }

  /**
   * Operator to check for greater-than relationship
   *
   * @param other
   *  The socket to check for greater-than relationship
   *
   * @return true if this socket is greater-than the specified socket,
   * false otherwise
   */
  bool operator>(const Socket& other) const noexcept
  { return not (*this < other); }

private:
  Address   addr_;
  port_t    port_;

}; //< class Socket

static_assert(std::is_move_constructible_v<Socket>);
static_assert(std::is_move_assignable_v<Socket>);
//static_assert(sizeof(Socket) == 18, "Socket not 18 byte");

/**
 * @brief      A pair of Sockets
 */
struct Quadruple {
  Socket src;
  Socket dst;

  Quadruple() = default;
  Quadruple(Socket s, Socket d)
    : src(std::move(s)), dst(std::move(d))
  {}

  bool operator==(const Quadruple& other) const noexcept
  { return src == other.src and dst == other.dst; }

  bool operator!=(const Quadruple& other) const noexcept
  { return not (*this == other); }

  bool operator<(const Quadruple& other) const noexcept
  { return src < other.src or (src == other.src and dst < other.dst); }

  bool operator>(const Quadruple& other) const noexcept
  { return src > other.src or (src == other.src and dst > other.dst); }

  bool is_reverse(const Quadruple& other) const noexcept
  { return src == other.dst and dst == other.src; }

  Quadruple& swap() {
    std::swap(src, dst);
    return *this;
  }

  std::string to_string() const
  { return src.to_string() + " " + dst.to_string(); }

}; //< struct Quadruple

} //< namespace net

namespace std {
  template<>
  struct hash<net::Socket> {
  public:
    size_t operator () (const net::Socket& key) const noexcept {
      const auto h1 = std::hash<net::ip6::Addr>{}(key.address().v6());
      const auto h2 = std::hash<net::Socket::port_t>{}(key.port());
      return h1 ^ h2;
    }
  };

  template<>
  struct hash<net::Quadruple> {
  public:
    size_t operator () (const net::Quadruple& key) const noexcept {
      const auto h1 = std::hash<net::Socket>{}(key.src);
      const auto h2 = std::hash<net::Socket>{}(key.dst);
      return h1 ^ h2;
    }
  };
} // < namespace std

#endif //< NET_SOCKET_HPP
