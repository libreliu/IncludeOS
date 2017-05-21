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

/*
 * Self sufficient Utility function that will copy a binary to a certain
 * location and then jump to it. The idea (from fwsgonzo) is to have this
 * function copied to an otherwise unused place in memory so that we can
 * overwrite the currently running binary with a new one.
 */
#include <cstdint>
asm(".org 0x200000");

extern "C" void* __multiboot_magic;
extern "C" void* __multiboot_addr;

extern "C" __attribute__((noreturn))
void hotswap(const char* base, int len, char* dest, void* start,
             uintptr_t magic, uintptr_t bootinfo)
{
  // Copy binary to its destination
  for (int i = 0; i < len; i++)
    dest[i] = base[i];

  // Populate multiboot regs and jump to start
  asm volatile("movl %0, %%eax" : : "a" (magic) : "eax");
  asm volatile("movl %0, %%ebx" : : "b" (bootinfo) : "ebx");
  asm volatile("jmp *%0" : : "r" (start));

  __builtin_unreachable();
}
