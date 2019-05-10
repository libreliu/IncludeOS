#include "bcm2835_timer.hpp"

struct bcm2835_system_timers
{
  volatile uint32_t control_status; // CS System Timer Control/Status 32
  volatile uint32_t counter_low;//CLO System Timer Counter Lower 32 bits 32
  volatile uint32_t counter_high; //System Timer Counter Higher 32 bits 3
  volatile uint32_t compare[4]; // system timer compare 1-4
};

static struct bcm2835_system_timers *sys_timers=nullptr;

void bcm2835_system_timer_init(uintptr_t base_addr)
{
  sys_timers=(struct bcm2835_system_timers *)base_addr;
  //pre handling
  sys_timers=(struct bcm2835_system_timers *)0x3f003000;

  //sys_timers->
}


uint64_t bcm2835_system_timer_read()
{
  if (sys_timers==nullptr) return 0;

  uint32_t hi=sys_timers->counter_high;
  uint32_t low=sys_timers->counter_low;

  if (hi != sys_timers->counter_high) //failed rollower handling re read
  {
    hi=sys_timers->counter_high;
    low=sys_timers->counter_low;
  }
  return ((uint64_t)hi<<32)| low;
}

int bcm2835_system_timer_get_status(bcm2835_system_timer timer)
{
  if (sys_timers->control_status == (0x1<<(int)timer))
    return 1;
  return 0;
}

void bcm2835_system_timer_clear_status(bcm2835_system_timer timer)
{
  sys_timers->control_status=(0x1<<(int)timer);
}

void bcm2835_system_timer_set_compare(uint32_t ticks,bcm2835_system_timer timer)
{
  sys_timers->compare[(int)timer]=sys_timers->counter_low+ticks;
}
