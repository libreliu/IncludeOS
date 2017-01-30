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

#include <hw/apic_timer.hpp>
#include <hw/apic.hpp>
#include <hw/pit.hpp>
#include <kernel/irq_manager.hpp>
#include <kernel/os.hpp>
#include <cstdio>
#include <info>

#define TIMER_ONESHOT     0x0
#define TIMER_PERIODIC    0x20000
#define TIMER_DEADLINE    0x40000

// vol 3a  10-10
#define DIVIDE_BY_16     0x3

#define CALIBRATION_MS   125

using namespace std::chrono;

namespace hw
{
  static APIC_Timer::handler_t intr_handler;
  static uint32_t ticks_per_micro = 0;
  static bool     intr_enabled    = false;

  void APIC_Timer::init(const handler_t& handler)
  {
    auto& lapic = hw::APIC::get();
    lapic.timer_init();

    if (ticks_per_micro != 0) {
      hw::PIT::instance().on_timeout_ms(milliseconds(1), handler);
      return;
    }

    // start timer (unmask)
    INFO("APIC", "Measuring APIC timer...");
    lapic.timer_begin(0xFFFFFFFF);
    
    // See: Vol3a 10.5.4.1 TSC-Deadline Mode
    // 0xFFFFFFFF --> ~68 seconds
    // 0xFFFFFF   --> ~46 milliseconds

    // ready handler
    intr_handler = handler;

    /// use PIT to measure <time> in one-shot ///
    hw::PIT::instance().on_timeout_ms(milliseconds(CALIBRATION_MS),
    [] {
      uint32_t diff = hw::APIC::get().timer_diff();
      // measure difference
      ticks_per_micro = diff / CALIBRATION_MS / 1000;

      //printf("* APIC timer: ticks %ums: %u\t 1mi: %u\n",
      //       CALIBRATION_MS, diff, ticks_per_micro);

      // signal ready to go
      intr_handler();
    });
  }

  bool APIC_Timer::ready()
  {
    return ticks_per_micro != 0;
  }

  void APIC_Timer::set_handler(const handler_t& handler)
  {
    intr_handler = handler;
    if (ready()) {
      IRQ_manager::get().subscribe(LAPIC_IRQ_TIMER, handler);
    }
  }

  void APIC_Timer::oneshot(std::chrono::microseconds micros)
  {
    // prevent overflow
    uint64_t ticks = micros.count() * ticks_per_micro;
    if (ticks > 0xFFFFFFFF) ticks = 0xFFFFFFFF;

    // set initial counter
    auto& lapic = hw::APIC::get();
    lapic.timer_begin(ticks);
    // re-enable interrupts if disabled
    if (intr_enabled == false) {
      intr_enabled = true;
      lapic.timer_interrupt(true);
    }
  }
  void APIC_Timer::stop()
  {
    hw::APIC::get().timer_interrupt(false);
    intr_enabled = false;
  }

  // used by soft-reset
  uint32_t apic_timer_get_ticks()
  {
    return ticks_per_micro;
  }
  void     apic_timer_set_ticks(uint32_t tpm)
  {
    ticks_per_micro = tpm;
  }
}
