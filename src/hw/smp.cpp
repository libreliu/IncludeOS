// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
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

#include <hw/smp.hpp>
#include <hw/acpi.hpp>
#include <hw/apic.hpp>
#include <hw/apic_revenant.hpp>
#include <kernel/irq_manager.hpp>
#include <algorithm>
#include <cstring>
#if defined(__MACH__)
extern void *aligned_alloc(size_t, size_t);
#endif

extern "C" {
  extern char _binary_apic_boot_bin_start;
  extern char _binary_apic_boot_bin_end;
  void lapic_exception_handler();
  void lapic_irq_entry();
}

extern idt_loc smp_lapic_idt;
static const uintptr_t BOOTLOADER_LOCATION = 0x80000;

struct apic_boot {
  // the jump instruction at the start
  uint32_t   jump;
  // stuff we will need to modify
  void*  worker_addr;
  void*  stack_base;
  size_t stack_size;
};

union addr_union {
  uint32_t whole;
  uint16_t part[2];

  addr_union(void(*addr)()) {
    whole = (uintptr_t) addr;
  }
};

using namespace hw;

void SMP::init()
{
  // smp with only one CPU == :facepalm:
  size_t CPUcount = ACPI::get_cpus().size();
  assert (CPUcount > 1);

  // copy our bootloader to APIC init location
  const char* start = &_binary_apic_boot_bin_start;
  ptrdiff_t bootloader_size = &_binary_apic_boot_bin_end - start;
  debug("Copying bootloader from %p to 0x%x (size=%d)\n",
        start, BOOTLOADER_LOCATION, bootloader_size);
  memcpy((char*) BOOTLOADER_LOCATION, start, bootloader_size);

  // modify bootloader to support our cause
  auto* boot = (apic_boot*) BOOTLOADER_LOCATION;
  // populate IDT used with SMP LAPICs
  smp_lapic_idt.limit = 256 * sizeof(IDTDescr) - 1;
  smp_lapic_idt.base  = (uintptr_t) new IDTDescr[256];

  auto* idt = (IDTDescr*) smp_lapic_idt.base;
  for (size_t i = 0; i < 32; i++) {
    addr_union addr(lapic_exception_handler);
    idt[i].offset_1 = addr.part[0];
    idt[i].offset_2 = addr.part[1];
    idt[i].selector  = 0x8;
    idt[i].type_attr = 0x8e;
    idt[i].zero      = 0;
  }
  for (size_t i = 32; i < 48; i++) {
    addr_union addr(lapic_irq_entry);
    idt[i].offset_1 = addr.part[0];
    idt[i].offset_2 = addr.part[1];
    idt[i].selector  = 0x8;
    idt[i].type_attr = 0x8e;
    idt[i].zero      = 0;
  }

  // assign stack and main func
  boot->worker_addr = (void*) &revenant_main;
  boot->stack_base = aligned_alloc(CPUcount * REV_STACK_SIZE, 4096);
  boot->stack_size = REV_STACK_SIZE;
  debug("APIC stack base: %p  size: %u   main size: %u\n",
      boot->stack_base, boot->stack_size, sizeof(boot->worker_addr));

  // reset barrier
  smp.boot_barrier.reset(1);

  auto& apic = hw::APIC::get();
  // turn on CPUs
  INFO("APIC", "Initializing APs");
  for (auto& cpu : ACPI::get_cpus())
  {
    if (cpu.id == apic.get_id()) continue;
    debug("-> CPU %u ID %u  fl 0x%x\n",
          cpu.cpu, cpu.id, cpu.flags);
    apic.ap_init(cpu.id);
  }
  // start CPUs
  INFO("APIC", "Starting APs");
  for (auto& cpu : ACPI::get_cpus())
  {
    if (cpu.id == apic.get_id()) continue;
    // Send SIPI with start address 0x80000
    apic.ap_start(cpu.id, 0x80);
    apic.ap_start(cpu.id, 0x80);
  }

  // wait for all APs to start
  smp.boot_barrier.spin_wait(CPUcount);
  INFO("APIC", "All APs are online now\n");
}

void SMP::add_task(smp_task_func task, smp_done_func done)
{
  lock(smp.tlock);
  smp.tasks.emplace_back(task, done);
  unlock(smp.tlock);
}
void SMP::work_signal()
{
  // broadcast that we have work to do
  hw::APIC::get().bcast_ipi(0x20);
}

std::vector<SMP::smp_done_func> SMP::get_completed()
{
  std::vector<smp_done_func> done;
  lock(smp.flock);
  for (auto& func : smp.completed) done.push_back(func);
  unlock(smp.flock);
  return done;
}
