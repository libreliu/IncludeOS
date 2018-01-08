// -*-C++-*-
// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2017 IncludeOS AS, Oslo, Norway
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

#include <cstdlib>
#include <malloc.h>
#include <kernel/memory.hpp>
#include <util/bitops.hpp>

extern "C"
int posix_memalign(void **memptr, size_t alignment, size_t size) {
  if (! util::is_pow2(alignment) or alignment < sizeof(void*))
    return EINVAL;

  auto* mem = memalign(alignment, size);
  if (mem == nullptr) {
    return errno;
  }
  *memptr = mem;
  return 0;
}
