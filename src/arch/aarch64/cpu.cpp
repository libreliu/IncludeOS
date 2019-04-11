#include <stdint.h>
#include <kprint>

#define DAIF_DBG_BIT      (1<<3)
#define DAIF_ABT_BIT      (1<<2)
#define DAIF_IRQ_BIT      (1<<1)
#define DAIF_FIQ_BIT      (1<<0)

#if defined(__cplusplus)
extern "C" {
#endif
extern "C" void drop_el1_secure(uint64_t sec);
extern "C" uintptr_t reset;
#define CURRENT_EL_MASK   0x3
#define CURRENT_EL_SHIFT  2
uint32_t cpu_current_el()
{
  uint32_t el;
  asm volatile("mrs %0, CurrentEL": "=r" (el)::);
  return ((el>>CURRENT_EL_SHIFT)&CURRENT_EL_MASK);
}

void cpu_print_current_el()
{
  uint32_t el=cpu_current_el();
  kprintf("CurrentEL %08x\r\n",el);
}

void cpu_change_el(uint8_t el)
{
    kprintf("Attempting to swap el to %d\r\n",el);
    if (el == cpu_current_el()) return;
    switch(el)
    {
      case 1:
        kprintf("Dropping down to el1");
        switch(cpu_current_el())
        {
          case 3:
          {
            //TODO make functions for reading and writing special regs
             kprintf(" from el3\n");

            /*TODO do this in the EL!! asm volatile(
              "mov x0, #3 << 20"
              "msr cpacr_el1, x0"           // Enable FP/SIMD at EL1)
              */
              uint32_t sctlr_el1=0;
              sctlr_el1|=(1 << 29);          /* Checking http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500d/CIHDIEBD.html */
              sctlr_el1|=(1 << 28);          /* Bits 29,28,23,22,20,11 should be 1 (res1 on documentation) */
              sctlr_el1|=(1 << 23);
              sctlr_el1|=(1 << 22);
              sctlr_el1|=(1 << 20);
              sctlr_el1|=(1 << 11);
              asm volatile("msr sctlr_el1, %0" ::"r"(sctlr_el1) );
              asm volatile("mrs x0, scr_el3\n\t"
              "orr x0, x0, #(1<<10)\n\t"        /* Lower EL is 64bits */
              "msr scr_el3, x0");
              kprint("Set exception handler for el3\r\n");
              uint32_t spsr_el3=0b00101; //EL1
              spsr_el3|=(1 << 8);       /* Enable SError and External Abort. */
              spsr_el3|=(1 << 7);       /* IRQ interrupt Process state mask. */
              spsr_el3|=(1 << 6);       /* FIQ interrupt Process state mask. */

              asm volatile ("msr spsr_el3, %0\n\t"::"r"(spsr_el3));

              drop_el1_secure(3);
            }
            break;
          case 2:
          {
            kprintf("from el2\n");

            asm volatile("mov x0, xzr\n\t"
            "msr cptr_el2, x0");

            /* RW=1 EL1 Execution state is AArch64. */
            asm volatile("mov x0, #(1 << 31)\n\t"
              "msr hcr_el2, x0\n\t");
           //TODO do this in the EL!!
           /*asm volatile(
             "mov x0, #3 << 20"
             "msr cpacr_el1, x0")          // Enable FP/SIMD at EL1)
*/
             uint32_t sctlr_el1=0;
             sctlr_el1|=(1 << 29);          /* Checking http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500d/CIHDIEBD.html */
             sctlr_el1|=(1 << 28);          /* Bits 29,28,23,22,20,11 should be 1 (res1 on documentation) */
             sctlr_el1|=(1 << 23);
             sctlr_el1|=(1 << 22);
             sctlr_el1|=(1 << 20);
             sctlr_el1|=(1 << 11);
             //sctlr_el1=0x30d00800;
             asm volatile("msr sctlr_el1, %0" ::"r"(sctlr_el1) );

             //woot do we need this ? i am not virtualizing
             asm volatile(
              "mov x0, #0x33ff\n\t"
              "msr cptr_el2, x0\n\t"
               "msr hstr_el2, xzr\n\t"
            );

             kprint("Set exception handler for el2\r\n");
             uint32_t spsr_el2=0b00101; //EL1
             spsr_el2|=(1 << 8);       /* Enable SError and External Abort. */
             spsr_el2|=(1 << 7);       /* IRQ interrupt Process state mask. */
             spsr_el2|=(1 << 6);       /* FIQ interrupt Process state mask. */
             //spsr_el2=0x3c5; //EL1 ?
             asm volatile ("msr spsr_el2, %0\n\t"::"r"(spsr_el2));

             drop_el1_secure(2);
             return;
          }
          break;
        }
        break;
        case 2:
          kprintf("TODO drop down from el3 to el2 ??\r\n");
          break;
        case 0:
          kprintf("TODO drop down to el0 from el3 el2 and el1\r\n");
          break;
    }

}

//extern "C" void arch_register_exception_handler(uintptr_t handler);

extern uintptr_t exception_vector;

//create an acrch class ?
void cpu_enable_fp_simd()
{
  switch(cpu_current_el())
  {
    case 3:
      asm volatile("msr cptr_el3, xzr");
      break;
    case 2:
      asm volatile("mov     x0, #0x33ff\n\t"
      "msr cptr_el2, x0");
      break;
    case 1:
      asm volatile("  mov     x0, #3 << 20\n\t"
        "msr     cpacr_el1, x0  ");
      break;
  }
}

/**
  TODO consider having more exception vectors
*/
void cpu_set_exception_handler()
{
  switch(cpu_current_el())
  {
    case 3:
      asm volatile(
        "msr vbar_el3,%0\n\t"
        "mrs x0, scr_el3\n\t"
        "orr x0, x0, #0xf\n\t"
        "msr scr_el3, x0\n\t" ::"r"(exception_vector)
      );
      break;
    case 2:
      asm volatile(
        "msr vbar_el2,%0\n\t" ::"r"(exception_vector)
      );
      break;
    case 1:
      asm volatile(
        "msr vbar_el1,%0\n\t" ::"r"(exception_vector)
      );
      break;
  }

}

char *get_tpidr()
{
  int el=cpu_current_el();
  char *self;
  switch(el)
  {
    case 0:
      __asm__ __volatile__ ("mrs %0,tpidr_el0" : "=r"(self));
      return self;
    case 1:
      __asm__ __volatile__ ("mrs %0,tpidr_el1" : "=r"(self));
      return self;
    case 2:
      __asm__ __volatile__ ("mrs %0,tpidr_el2" : "=r"(self));
      return self;
    case 3:
      __asm__ __volatile__ ("mrs %0,tpidr_el3" : "=r"(self));
      return self;
  }
  return nullptr;
}

void set_tpidr(void *self)
{
  int el=cpu_current_el();
  switch(el)
  {
    case 0:
      __asm__ __volatile__ ("msr tpidr_el0,%0" : :"r"(self));
      break;
    case 1:
      __asm__ __volatile__ ("msr tpidr_el1,%0" : :"r"(self));
      break;
    case 2:
      __asm__ __volatile__ ("msr tpidr_el2,%0" : :"r"(self));
      break;
    case 3:
      __asm__ __volatile__ ("msr tpidr_el3,%0" : :"r"(self));
      break;
  }
}

void cpu_fiq_enable()
{
  asm volatile("msr DAIFClr,%0" ::"i"(DAIF_FIQ_BIT): "memory");
}

void cpu_irq_enable()
{
  asm volatile("msr DAIFClr,%0" ::"i"(DAIF_IRQ_BIT) : "memory");
}

void cpu_serror_enable()
{
  asm volatile("msr DAIFClr,%0" ::"i"(DAIF_ABT_BIT) : "memory");
}

void cpu_debug_enable()
{
  asm volatile("msr DAIFClr,%0" ::"i"(DAIF_DBG_BIT): "memory");
}

void cpu_fiq_disable()
{
  asm volatile("msr DAIFSet,%0" ::"i"(DAIF_FIQ_BIT): "memory");
}

void cpu_irq_disable()
{
  asm volatile("msr DAIFSet,%0" ::"i"(DAIF_IRQ_BIT) : "memory");
}

void cpu_serror_disable()
{
  asm volatile("msr DAIFSet,%0" ::"i"(DAIF_ABT_BIT) : "memory");
}

void cpu_debug_disable()
{
  asm volatile("msr DAIFSet,%0" ::"i"(DAIF_DBG_BIT): "memory");
}

void cpu_disable_all_exceptions()
{
  asm volatile("msr DAIFSet,%0" :: "i"(DAIF_FIQ_BIT|DAIF_IRQ_BIT|DAIF_ABT_BIT|DAIF_DBG_BIT) : "memory");
}

void cpu_wfi()
{
  asm volatile("wfi" : : : "memory");
}

void cpu_disable_exceptions(uint32_t irq)
{
  //seting mask to 0 enables the irq
  //__asm__ __volatile__("msr DAIFClr, %0\n\t" :: "r"(irq) : "memory");
  //OLD WAY
  volatile uint32_t daif;
  asm volatile("mrs %0 , DAIF" : "=r"(daif)::"memory");
  daif &=~irq;
  asm volatile("msr DAIF, %0 " : :"r"(daif):"memory");
}

void cpu_enable_exceptions(uint32_t irq)
{
  //setting the mask to 1 disables the irq
  //asm volatile("msr daifset, %0\n\t" :: "r"(irq):"memory");
  //OLD WAY

  volatile uint32_t daif;
  asm volatile("mrs %0 , DAIF" : "=r"(daif)::"memory");
  daif |=irq;
  asm volatile("msr DAIF, %0" : :"r"(daif):"memory");
}

#if defined(__cplusplus)
}
#endif
