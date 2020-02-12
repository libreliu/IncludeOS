// -*-C++-*-

#include <arch.hpp>
#include <kernel/memory.hpp>

#include <util/bitops.hpp>
#include <util/units.hpp>

#include <arch/aarch64/paging.hpp>

__attribute__((weak))
void __arch_init_paging()
{
  INFO("aarch64", "Paging not enabled by default on");
}

// TODO: support aarch64 "cpuid" stuff
uintptr_t os::mem::supported_page_sizes()
{
  // only needed to detect once
  static size_t res = 0;
  if (res == 0)
  {
    uint64_t result;
    __asm__ __volatile__("mrs %0, ID_AA64MMFR0_EL1"
                         : "=r"(result));
    if (result & 0xF0000000 == 0xF0000000)
    {
      res |= 4_KiB;
    }
    if (result & 0xF000000 == 0xF000000)
    {
      res |= 64_KiB;
    }
    if (result & 0xF00000 == 0x100000)
    {
      res |= 16_KiB;
    }
  }
  return res;
}

uintptr_t aarch64::paging::supported_pa_range() {
  // only needed to detect once
  static size_t res = 0;
  if (res == 0)
  {
    uint64_t result;
    __asm__ __volatile__("mrs %0, ID_AA64MMFR0_EL1"
                         : "=r"(result));
    result &= 0xF;
    switch (result) {
      case 0x0:  // 32 bits, 4   GB
        res = 4_GiB;
        break;
      case 0x1:  // 36 bits, 64  GB
        res = 64_GiB;
        break;
      case 0x2:  // 40 bits, 1   TB
        res = 1_TiB;
        break;
      case 0x3:  // 42 bits, 4   TB
        res = 4_TiB;
        break;
      case 0x4:  // 44 bits, 16  TB
        res = 16_TiB;
        break;
      case 0x5:  // 48 bits, 256 TB
        res = 256_TiB;
        break;
    }
  }
  return res;
}

template <>
const size_t os::mem::Map::any_size{supported_page_sizes()};

namespace os {
namespace mem {
  __attribute__((weak))
  Map map(Map m, const char* name) {
    return {};
  }

  // template <>
  // const size_t Mapping<os::mem::Access>::any_size = 4096;
  
} // mem
} // os

