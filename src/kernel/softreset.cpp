#include <os>
#include <kprint>
#include <util/crc32.hpp>

#define SOFT_RESET_MAGIC    0xFEE1DEAD
#define SOFT_RESET_LOCATION 0x7000

namespace hw {
  extern uint32_t apic_timer_get_ticks() noexcept;
  extern void     apic_timer_set_ticks(uint32_t) noexcept;
}

struct softreset_t
{
  uint32_t checksum;
  MHz      cpu_freq;
  uint32_t apic_ticks;
};

bool OS::is_softreset_magic(uintptr_t value)
{
  return value == SOFT_RESET_MAGIC;
}

void OS::resume_softreset(intptr_t addr)
{
  auto* data = (softreset_t*) addr;
  
  /// validate soft-reset data
  const uint32_t csum_copy = data->checksum;
  data->checksum = 0;
  uint32_t crc = crc32(data, sizeof(softreset_t));
  if (crc != csum_copy) {
    kprintf("[!] Failed to verify CRC of softreset data: %08x vs %08x\n",
            crc, csum_copy);
    return;
  }
  data->checksum = csum_copy;
  
  /// restore known values
  OS::cpu_mhz_ = data->cpu_freq;
  hw::apic_timer_set_ticks(data->apic_ticks);
}

extern "C"
void* __os_store_soft_reset()
{
  // store softreset data in low memory
  auto* data = (softreset_t*) SOFT_RESET_LOCATION;
  data->checksum    = 0;
  data->cpu_freq    = OS::cpu_freq();
  data->apic_ticks  = hw::apic_timer_get_ticks();
  
  uint32_t csum = crc32(data, sizeof(softreset_t));
  data->checksum = csum;
  return data;
}
