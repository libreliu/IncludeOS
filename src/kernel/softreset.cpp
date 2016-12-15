#include <os>
#include <kprint>
#include <hw/apic_timer.hpp>
#include <util/crc32.hpp>

namespace hw {
  uint32_t apic_timer_get_ticks();
  void     apic_timer_set_ticks(uint32_t);
}

struct softreset_t
{
  uint32_t checksum;
  MHz      cpu_freq;
  uint32_t apic_ticks;
};

void OS::resume_softreset(intptr_t addr)
{
  auto* data = (softreset_t*) addr;
  
  /// validate soft-reset data
  const uint32_t csum_copy = data->checksum;
  data->checksum = 0;
  uint32_t crc = crc32(data, sizeof(softreset_t));
  if (crc != csum_copy) {
    kprintf("Failed to verify CRC of softreset data: %08x vs %08x\n",
            crc, csum_copy);
    assert(false);
  }
  data->checksum = csum_copy;
  
  kprint("[!] Soft resetting OS\n");
  /// restore known values
  OS::cpu_mhz_ = data->cpu_freq;
  hw::apic_timer_set_ticks(data->apic_ticks);
}

extern "C"
void* __os_store_soft_reset()
{
  // store softreset data in low memory
  auto* data = (softreset_t*) 0x7000;
  data->checksum    = 0;
  data->cpu_freq    = OS::cpu_freq();
  data->apic_ticks  = hw::apic_timer_get_ticks();
  
  uint32_t csum = crc32(data, sizeof(softreset_t));
  data->checksum = csum;
  return data;
}
