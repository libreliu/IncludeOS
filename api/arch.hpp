// -*-C++-*-
// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2017 Oslo and Akershus University College of Applied Sciences
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
#ifndef INCLUDEOS_ARCH_HEADER
#define INCLUDEOS_ARCH_HEADER

#include <cstddef>
#include <cstdint>
#include <cassert>

extern void __arch_init();
extern void __arch_poweroff();
extern void __arch_reboot();
extern void __arch_enable_legacy_irq(uint8_t);
extern void __arch_disable_legacy_irq(uint8_t);
inline void __arch_hw_barrier() noexcept;
inline void __sw_barrier() noexcept;
inline uint64_t __arch_cpu_cycles() noexcept;


inline void __sw_barrier() noexcept
{
  asm volatile("" ::: "memory");
}

// Include arch specific inline implementations
#if defined(ARCH_x86_64)
#include "arch/x86_64.hpp"
#elif defined(ARCH_i686)
#include "arch/i686.hpp"
#else
#error "Unsupported arch specified"
#endif

#endif
