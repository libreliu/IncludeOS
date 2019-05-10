
#include <cstdint>

/*
#define LOCAL_IRQ_CNTPSIRQ	0
#define LOCAL_IRQ_CNTPNSIRQ	1
#define LOCAL_IRQ_CNTHPIRQ	2
#define LOCAL_IRQ_CNTVIRQ	3
#define LOCAL_IRQ_MAILBOX0	4
#define LOCAL_IRQ_MAILBOX1	5
#define LOCAL_IRQ_MAILBOX2	6
#define LOCAL_IRQ_MAILBOX3	7
#define LOCAL_IRQ_GPU_FAST	8
#define LOCAL_IRQ_PMU_FAST	9
#define LAST_IRQ		LOCAL_IRQ_PMU_FAST
*/
enum class bcm2836_local_irq
{
  CNTPS=0,
  CNTPN=1,
  CNTHP=2,
  CNTV=3,
  MBOX0=4,
  MBOX1=5,
  MBOX2=6,
  MBOX3=7,
  GPU_FAST=8,
  PMU_FAST=9
};

void bcm2836_local_enable_pmu_routing();
void bcm2836_local_irq_regs();

void bcm2836_local_irq_init(uintptr_t base);
uint64_t bcm2836_local_timer();
void bcm2836_local_timer_start(uint32_t timeout);
void bmc2836_local_timer_wait();
void bmc2836_interrupt_source();

//void bcm2836_l1_irq_enable(bmcm2836_l1_irq_flags flags);
