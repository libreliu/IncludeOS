// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2018 IncludeOS AS, Oslo, Norway
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

#include <hal/machine.hpp>
#include <util/units.hpp>
#include <kernel.hpp>
#include <os>

#ifndef INFO_MACHINE
#define MINFO(fmt, ...) kprintf("[ Machine ] " fmt, ##__VA_ARGS__)
#endif

using namespace util;

// Reserve some machine memory for e.g. devices
// (can still be used by heap as fallback).
static constexpr auto reserve_mem = 1_MiB;

// Max percent of memory reserved by machine
static constexpr int  reserve_pct_max = 10;
static_assert(reserve_pct_max > 0 and reserve_pct_max < 90);

namespace os {

  Machine::Memory& Machine::memory() noexcept {
    return impl->memory();
  }

  void Machine::init() noexcept {
    impl->init();
  }

  const char* Machine::arch() noexcept {
    return impl->arch();
  }

  // Implementation details
  Machine* Machine::create(void* mem, size_t size) noexcept {
    char* mem_begin = (char*)mem + sizeof(Machine);
    return new(mem) Machine((void*)mem_begin, size - sizeof(Machine));
  }

  Machine::Machine(void* mem, size_t size) noexcept
  : impl {nullptr} {

    Expects(mem != nullptr);
    Expects(size > sizeof(detail::Machine) + Machine::Memory::min_alloc);

    // Placement new impl
    impl = {new (mem) detail::Machine{(char*)mem + sizeof(detail::Machine),
                                      size - sizeof(detail::Machine)}};
  }

}

// Detail implementations
namespace os::detail {

  Machine::Machine(void* mem, size_t size)
    : mem_{
        (void*) bits::align(Memory::align, (uintptr_t)mem),
        size - (bits::align(Memory::align, (uintptr_t)mem) - (uintptr_t)mem)
      },
      ptr_alloc_(mem_), parts_(ptr_alloc_)
  {
    kprintf("[%s %s] constructor \n", arch(), name());
  }

  void Machine::init() {
    MINFO("Initializing heap\n");
    auto main_mem = memory().allocate_largest();
    MINFO("Main memory detected as %zu b\n", main_mem.size);

    if (memory().bytes_free() < reserve_mem) {
      if (main_mem.size > reserve_mem * (100 - reserve_pct_max)) {
        main_mem.size -= reserve_mem;
        auto back = (uintptr_t)main_mem.ptr + main_mem.size - reserve_mem;
        memory().deallocate((void*)back, reserve_mem);
        MINFO("Reserving %zu b for machine use \n", reserve_mem);
      }
    }

    kernel::init_heap((uintptr_t)main_mem.ptr, main_mem.size);
  }

  const char* Machine::arch() { return Arch::name; }
}
