#include "bcm2836_local_irq.hpp"
#include <kprint>

struct bmc2836_local_irq_regs
{
  volatile uint32_t ctrl; //0x0
  volatile uint32_t reserved1; //0x4
  volatile uint32_t prescaler; //0x8
  volatile uint32_t gpu_routing; //0x0C low 2 bits identiy the cpu that irq goes to the next two identify the fiq
  volatile uint32_t pmu_routing_set; //0x10 bits 0-3 enables gpu routing on that cpu
  volatile uint32_t pmu_routing_clear;//0x14 bits 0-3 disables gpu routing on that cpu
  volatile uint32_t reserved1_1; //0x18
  volatile uint32_t core_timer_lsr; //0x1C
  volatile uint32_t core_timer_msr; //0x20
  volatile uint32_t local_timer_routing; //0x24
  volatile uint32_t reserved2[3]; //0x18-0x3C
  volatile uint32_t local_timer_ctrl; //0x34
  volatile uint32_t local_timer_clear_reload; //0x38
  volatile uint32_t reserved3;
  /*
 * The low 4 bits of this are the CPU's timer IRQ enables, and the
 * next 4 bits are the CPU's timer FIQ enables (which override the IRQ
 * bits).
 */
  volatile uint32_t timer_control[4]; //0x40
  /*
 * The low 4 bits of this are the CPU's per-mailbox IRQ enables, and
 * the next 4 bits are the CPU's per-mailbox FIQ enables (which
 * override the IRQ bits).
 */
  volatile uint32_t mailbox_control[4]; //0x50

  volatile uint32_t irq_pending[4]; //0x60
  /*
 * The CPU's interrupt status register.  Bits are defined by the the bcm2836_local_irq
 */
  volatile uint32_t fiq_pending[4]; //0x70

  volatile uint32_t mailbox_set[4*4]; //0x80
  volatile uint32_t mailbox_clear[4*4]; //0xc0
};

static struct bmc2836_local_irq_regs *local_regs=nullptr;



void bcm2836_local_irq_regs()
{
  kprintf("Local_regs %zx\n",local_regs);
  uint32_t *ptr=(uint32_t*)local_regs;
//  for (int i=0;i<0x80/4;i++)
//  {
  for (int i=0;i < 4;i++)
  {
    kprintf("reg %16zx = %08x\n",&ptr[i],ptr[i]);
  }
}

void bmc2836_interrupt_source()
{
  kprintf("Setting %016zx to %08x\r\n",&local_regs->irq_pending[0],0xFFF);
  local_regs->irq_pending[0]=0xFFF;
  kprintf("Setting %016zx to %08x\r\n",&local_regs->timer_control[0],0x7);
  local_regs->timer_control[0]=0x7;
}
//performance monitoring

void bcm2836_local_enable_pm_routing()
{
  //todo get cpu id.. etc.
//  local_regs->pmu_routing_set=0x1<<0x0;
}
uint64_t bcm2836_local_timer()
{
  uint32_t high=local_regs->core_timer_msr;
  uint32_t low=local_regs->core_timer_lsr;
  //check if wrapped
  if (high!=local_regs->core_timer_msr)
  {
    high=local_regs->core_timer_msr;
    low=local_regs->core_timer_lsr;
  }
  return high<<32|low;
}

void bcm2836_local_timer_start(uint32_t timer)
{
  //clear
  if (local_regs->local_timer_ctrl&0x80000000)
  {
    printf("clearing irq flag\n");
    local_regs->local_timer_clear_reload=0x80000000;
  }
  uint32_t val=timer&0x0fffffff | 0x1<<28 | 0x1<<29;
  local_regs->local_timer_ctrl=val;
  if (local_regs->local_timer_ctrl&0x80000000)
  {
    printf("clearing irq flag\n");
    local_regs->local_timer_clear_reload=0xC0000000;
  }
  kprintf("local_timer_ctrl %zx = %x\n",&local_regs->local_timer_ctrl,val);

}

void bmc2836_local_timer_wait()
{
  kprintf("Wait for timer\n");
  while(!local_regs->local_timer_ctrl&0x80000000)
  {
    kprintf("local_timer_ctrl %zx = %x\n",&local_regs->local_timer_ctrl,local_regs->local_timer_ctrl);
  }
}
/*
uint32_t bcm2836_local_irq_get_freq()
{
  return local_regs->
}*/

void bcm2836_local_irq_init(uintptr_t base)
{
  printf("LOCAL IRQ INIT\n");
  if (base != 0)
  {
    local_regs=(struct bmc2836_local_irq_regs *)base;
  }
  else
  {
    local_regs=(struct bmc2836_local_irq_regs *)0x40000000;
  }
  uint32_t *ptr=(uint32_t *)local_regs;
  for (int i=0;i<64;i++)
  {
    printf("Addr %zx = %x\n",&ptr[i],ptr[i]);
  }
}
