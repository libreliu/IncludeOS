// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
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
#ifndef INCLUDE_UDP_FD_HPP
#define INCLUDE_UDP_FD_HPP

#include "fd.hpp"
#include <net/inet4>

class UDP_FD : public FD {
public:
  using id_t = int;

  explicit UDP_FD(const int id)
    : FD(id), non_blocking_(false), broadcast_(false)
  {}

  int     read(void*, size_t) override;
  int     write(const void*, size_t) override;
  int     close() override;
  /** SOCKET */
  int     bind(const struct sockaddr *, socklen_t) override;
  ssize_t sendto(const void *, size_t, int, const struct sockaddr *, socklen_t) override;
  ssize_t recvfrom(void *__restrict__, size_t, int, struct sockaddr *__restrict__, socklen_t *__restrict__) override;

  ~UDP_FD() {}
private:
  net::UDPSocket* sock = nullptr;
  bool non_blocking_;
  bool broadcast_;
};

#endif
