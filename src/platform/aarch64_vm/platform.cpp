#include <os>
#include <kernel/events.hpp>
#include <kernel/timers.hpp>
#include <kernel/rtc.hpp>

#include <kprint>
#include <kernel.hpp>
#include <kernel/rng.hpp>
#include <timer.h>

#include <smp>

extern "C" {
  #include <libfdt.h>
}

#include <cpu.h>

#include "exception_handling.hpp"
#include "gic.h"
#include "cpu.h"
#include "mini_uart.h"
#include "bcm2835_rng.hpp"
#include "bcm2836_irq.hpp"
#include "bcm2836_local_irq.hpp"
#include "bcm2835_timer.hpp"


extern "C" void noop_eoi() {}
extern "C" void cpu_sampling_irq_handler() {}
extern "C" void blocking_cycle_irq_handler() {}

extern "C" void vm_exit();


//do this instead of vm
#define RPI3B_PLUS
/*
void (*current_eoi_mechanism)() = noop_eoi;
void (*current_intr_handler)() = nullptr;
*/

#include <util/fixed_vector.hpp>

namespace kernel {
  Fixed_vector<delegate<void()>, 64> smp_global_init(Fixedvector_Init::UNINIT);
}

/*
void find_fdt_component(fdt,char *compat)
{
  offset = fdt_node_offset_by_compatible(fdt, -1, compat);
  if (offset >= 0) {
      return fdt_get_phandle(fdt, offset);
  }
}
*/
void __platform_init(uint64_t fdt_addr)
{
  printf("PLATFORM INIT\n");
  kprintf("PLATFORM INIT\r\n");
  //belongs in platform ?
  const char *fdt=(const char *)fdt_addr;
  kprintf("fdt addr %zx\r\n",fdt_addr);



  //checks both magic and version
  if ( fdt_check_header(fdt) != 0 )
  {
    kprintf("FDT Header check failed\r\n");
    return;
  }

  int intc=-1;
  const char *name=fdt_get_alias(fdt,"intc");
  if (name) {
    kprintf("Found alias for intc %s\r\n",name);
    intc=fdt_path_offset(fdt, name);
  //  return;
  } else {
    kprintf("No alias found for intc\n");
    intc = fdt_path_offset(fdt, "/intc");

    if (intc < 0 )
      intc = fdt_path_offset(fdt, "/soc/interrupt-controller");
  }

  if (intc < 0) //interrupllert controller not found in fdt.. should never happen..
  {
    kprintf("interrupt controller not found in dtb\r\n");
    return;
  }
  //TODO add gicv3 support
  if (fdt_node_check_compatible(fdt,intc,"arm,cortex-a15-gic") == 0)
  {
    kprintf("init gic\r\n");
    gic_init_fdt(fdt,intc);
  }
  else if (fdt_node_check_compatible(fdt,intc,"brcm,bcm2836-armctrl-ic") == 0)
  {
    bcm2836_irq_init_fdt(fdt,intc);
    kprintf("brcm intc found\r\n");
  }
  else
  {
    kprintf("incompatible gic\r\n");
  }


  Events::get(0).init_local();

  struct alignas(SMP_ALIGN) timer_data
  {
    int  intr;
    bool intr_enabled = false;
  };
  static std::vector<timer_data> timerdata;
  SMP_RESIZE_EARLY_GCTOR(timerdata);

  // TODO: Figure out if smp infomation is available at this moment
  for (auto lambda : kernel::smp_global_init) {
    lambda();
  }



  //frequency is in ticks per second
  static uint32_t ticks_per_micro = timer_get_frequency()/1000000;
  /*
  Timers::init(
    [](Timers::duration_t nanos){

        //there is a better way to do this!!
        // prevent overflow
        uint64_t ticks = (nanos.count() / 1000)* ticks_per_micro;
        if (ticks > 0xFFFFFFFF)
            ticks = 0xFFFFFFFF;
        // prevent oneshots less than a microsecond
        // NOTE: when ticks == 0, the entire timer system stops
        else if (UNLIKELY(ticks < ticks_per_micro))
            ticks = ticks_per_micro;

        timer_set_virtual_compare(timer_get_virtual_countval()+ticks);
        timer_virtual_start();
    },
    [](){
      timer_virtual_stop();
    }
  );
*/
  /* TODO figure out what these are for
  PER_CPU(timerdata).intr=TIMER_IRQ;
  Events::get().subscribe(TIMER_IRQ,Timers::timers_handler);
*/
  //regiser the event handler for the irq.. ?
#if defined(RPI3B_PLUS)

  cpu_irq_disable();
  //todo fix fdt addr
  //kprintf("Enable local irq\n");
//  kprintf("Enable irq sources?? routing\n");
  //bcm2836_local_enable_pmu_routing();
  //bmc2836_interrupt_source();
  //kprintf("init system timers\n");
  kprintf("Enable rounting in local irq\n");

  bcm2836_local_irq_init(0);
  //bcm2836_local_irq_regs();
  bmc2836_interrupt_source();

  bcm2835_system_timer_init(0);


  uint64_t time_one=bcm2835_system_timer_read();
  kprintf("timeone %zu\n",time_one);
  kprintf("timetwo %zu\n",bcm2835_system_timer_read());
  kprintf("local timer %zu\n",bcm2836_local_timer());

  //timer+ irq == 0
  /*

*/
  cpu_fiq_enable();
  cpu_irq_enable();

  //lets se what happens now
  bcm2835_system_timer_clear_status(bcm2835_system_timer::timer1);
  bcm2835_system_timer_set_compare(1000000,bcm2835_system_timer::timer1);

  kprintf("IRQ_Pending %d\n",bcm2835_irq_decode());

  //is system timer irq 0 correct ?
  //TIMER 1 is IRQ1 IRQ0=timer0 which is reserved
  bcm2835_irq_enable(1);

  //cpu_wfi();


  kprintf("timer irq timeout wait\n");
  while(!bcm2835_system_timer_get_status(bcm2835_system_timer::timer1));
  kprintf("IRQ_Pending %d\n",bcm2835_irq_decode());
  kprintf("timetree %zu\n",bcm2835_system_timer_read());
  kprintf("local timer %zu\n",bcm2836_local_timer());

  kprintf("timer irq_set\n");

  //16 mill er kanskje halvt sek
  bcm2836_local_timer_start(16000000);

  bmc2836_local_timer_wait();
  //krpintf("local irq freq=%d\n",bcm2836_local_irq_get_freq())

#else
  #define TIMER_IRQ 27
  register_handler(TIMER_IRQ,&Timers::timers_handler);

  gicd_set_config(TIMER_IRQ,GICD_ICFGR_EDGE);
  //highest possible priority
  gicd_set_priority(TIMER_IRQ,0);
  //TODO get what core we are on@
  gicd_set_target(TIMER_IRQ,0x01); //target is processor 1
  gicd_irq_clear(TIMER_IRQ);
  gicd_irq_enable(TIMER_IRQ);
#endif

  kprintf("Enabling all irq types\n");
  cpu_fiq_enable();
  cpu_irq_enable();
  cpu_serror_enable();
//  cpu_debug_enable();
  kprintf("done enable all irq types\n");
/*
  kprintf("disable all irq types\n");
  cpu_fiq_disable();
  cpu_irq_disable();
  cpu_serror_disable();
  cpu_debug_disable();
  kprintf("Initialize RTC\n");
  */
//  RTC::init();

//  Timers::ready();

  kprintf("Platform init complete\n");
}


void __arch_poweroff()
{

  kprint("ARCH poweroff\n");
  vm_exit();
  //TODO check that this is sane on ARM
//  while (1) asm("hlt #0xf000;");
  __builtin_unreachable();
}

// Needed on real HW
void RNG::init()
{
#if defined(RPI3B_PLUS)
  bcm2835_rng_init(0); //missing fdt addr
  rng_reseed_init(bcm2835_rng_data, 2);
#else
  size_t random = 4; // one of the best random values
  rng_absorb(&random, sizeof(random));
#endif
}
/*
void RNG::init()
{
  #if defined RPI3

  #
  uint32_t random = rng->data;
  rng_absorb(&random, sizeof(random));
}

*/

// not supported!
uint64_t __arch_system_time() noexcept {
  //in nanoseconds..
  return ((timer_get_virtual_countval()*10)/(timer_get_frequency()/100000))*1000;
  //return 0;
}
timespec __arch_wall_clock() noexcept {
  return {0, 0};
}
// not supported!

// default to serial
void kernel::default_stdout(const char* str, const size_t len)
{
  //on rpi3 use this instead..
  __mini_uart_print(str,len);
  //__serial_print(str, len);
}

void __arch_reboot()
{
  kprint("ARCH REBOOT WHY ?\r\n");
  //TODO
}
/*
void __arch_enable_legacy_irq(unsigned char){}
void __arch_disable_legacy_irq(unsigned char){}
*/

void SMP::global_lock() noexcept {}
void SMP::global_unlock() noexcept {}
int SMP::cpu_count() noexcept { return 1; }



// default stdout/logging states
__attribute__((weak))
bool os_enable_boot_logging = false;
__attribute__((weak))
bool os_default_stdout = false;
