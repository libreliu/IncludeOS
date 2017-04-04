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

#include <kernel/os.hpp>
#include <kernel/cpuid.hpp>
#include <boot/multiboot.h>
#include <kprint>

#define MULTIBOOT_CMDLINE_LOC 0x7000

extern "C" {
  void __init_serial1();
  void __init_sanity_checks();
  void kernel_sanity_checks();
  uintptr_t _multiboot_free_begin(uintptr_t boot_addr);
  uintptr_t _move_symbols(uintptr_t loc);
  void _init_bss();
  void _init_heap(uintptr_t);
  void _init_c_runtime();
  void _init_syscalls();
  void _init();
  extern uintptr_t _end;
}

extern "C"
void kernel_start(uintptr_t magic, uintptr_t addr)
{
  __init_serial1();
  kprint("64-bit test1\n");

  // generate checksums of read-only areas etc.
  __init_sanity_checks();

  uintptr_t free_mem_begin = reinterpret_cast<uintptr_t>(&_end);

  // Save multiboot string before symbols overwrite area after binary
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    free_mem_begin = _multiboot_free_begin(addr);
  }

  // Preserve symbols from the ELF binary
  free_mem_begin += _move_symbols(free_mem_begin);

  // Initialize zero-initialized vars
  _init_bss();

  // Initialize heap
  _init_heap(free_mem_begin);

  // Initialize stack-unwinder, call global constructors etc.
  _init_c_runtime();

  // Initialize system calls
  _init_syscalls();

  // call global constructors emitted by compiler
  _init();

  // Initialize OS including devices
  OS::start(magic, addr);

  // verify certain read-only sections in memory
  kernel_sanity_checks();

  // Starting event loop from here allows us to profile OS::start
  OS::event_loop();
}
