#include "bcm2836_irq.hpp"

#include <kprint>
#include "fdt.hpp"
#include "cpu.h"

/*
base 7e00b000;
0x200 IRQ basic pending
0x204 GPU IRQ pending 1
0x208 GPU IRQ pending 2
0x20C FIQ control
0x210 Enable IRQs 1
0x214 Enable IRQs 2
0x218 Enable Basic IRQs
0x21C Disable IRQs 1
0x220 Disable IRQs 2
0x224 Disable Basic IRQs
*/

//TODO wrap into os ?



struct bcm2835_irq_regs
{
  volatile uint32_t basic_irq_pending;
  volatile uint32_t gpu_irq_pending[2];
  volatile uint32_t fiq_control;
  volatile uint32_t gpu_irq_enable[2];
  volatile uint32_t basic_irq_enable;
  volatile uint32_t gpu_irq_disable[2];
  volatile uint32_t basic_irq_disable;
};

static struct bcm2835_irq_regs *irq_regs=nullptr;

//can we really initialize like this ? would a singleton pattern work across context switches ? ask alfred..
void bcm2835_irq_init(uintptr_t base)
{
  //convert base to 0x3F ? is this execution level memory map or just an annoyance from linux ?
//#if defined(__aarch64__)
  if ((base&0x7E000000) == 0x7E000000)
  {
    kprintf("altering base\n");
    base &=(~0x7E000000);
    base |=0x3F000000;
  }
//#endif
  kprintf("initializing interrupt handler @%zx\n",base);
  irq_regs=(struct bcm2835_irq_regs *)base;

  cpu_irq_disable();
  cpu_fiq_disable();
  kprintf("Base pending %08x\n",irq_regs->basic_irq_pending);
  kprintf("pending[0] %08x\n",irq_regs->gpu_irq_pending[0]);
  kprintf("pending[1] %08x\n",irq_regs->gpu_irq_pending[1]);
  kprintf("enable[0] %08x\n",irq_regs->gpu_irq_enable[0]);
  kprintf("enable[1] %08x\n",irq_regs->gpu_irq_enable[1]);
  kprintf("base enable %08x\n",irq_regs->basic_irq_enable);

  kprintf("Disable all irq's\n");
  irq_regs->gpu_irq_disable[0]=0xffffffff;
  irq_regs->gpu_irq_disable[1]=0xffffffff;
  irq_regs->basic_irq_disable=0xffffffff;
  //TODO disable some irq's ?
}

void bcm2836_irq_init_fdt(const char * fdt,uint32_t fdt_offset)
{
  const struct fdt_property *prop;
  int addr_cells = 1, size_cells = 1;//, interrupt_cells = 0;
  //ffs this is wrong .. doh-.
  /*size_cells = fdt_size_cells(fdt,fdt_offset);
  addr_cells = fdt_address_cells(fdt, fdt_offset);//fdt32_ld((const fdt32_t *)prop->data);
*/
  //how to get ?
  //interrupt_cells=fdt_interrupt_cells(fdt,fdt_offset);
//#ifdef DEBUG
  kprintf("size_cells %d\n",size_cells);
  kprintf("addr_cells %d\n",addr_cells);
//#endif

  int proplen;
  prop = fdt_get_property(fdt, fdt_offset, "reg", &proplen);
  kprintf("Proplen %d\r\n",proplen);
  int cellslen = (int)sizeof(uint32_t) * (addr_cells + size_cells);
  /*if (proplen/cellslen < 2)
  {
    kprintf("fdt expected two addresses");
    return;
  }*/
  int offset=0;
  uint64_t base;
/*  uint64_t gicc;
*/
  uint64_t len;
  base=fdt_load_addr(prop,&offset,addr_cells);
  kprintf("base %016lx\r\n",base);
  /*
  len=fdt_load_size(prop,&offset,size_cells);
  GIC_DEBUG("len %016lx\r\n",len);
  gicc=fdt_load_addr(prop,&offset,addr_cells);
  GIC_DEBUG("gicc %016lx\r\n",gicc);
  //not really needed'
*/
  len=fdt_load_size(prop,&offset,size_cells);
  kprintf("len %016lx\r\n",len);

  //gic_init(uint64_t gic_gicd_base,uint64_t gic_gicc_base);
  //GIC_INFO("interrupt_cells %d\n",interrupt_cells);

  bcm2835_irq_init(base);
  //*/
}

//must be called from interrupt context.. how do we do that c++ish
int bcm2835_irq_decode()
{
  //this is very complicated..!!

  uint32_t pending=irq_regs->basic_irq_pending;
  //cant switch so its if case mania..
  if (pending & ~0x3FF)
  {
    //I do not see the gain.. maybe i am blind some can be merged to a for loop
    if (pending & 0x1<<10) return 7;
    if (pending & 0x1<<11) return 9;
    if (pending & 0x1<<12) return 10;
    if (pending & 0x1<<13) return 18;
    if (pending & 0x1<<14) return 19;
    if (pending & 0x1<<15) return 53;
    if (pending & 0x1<<16) return 54;
    if (pending & 0x1<<17) return 55;
    if (pending & 0x1<<18) return 56;
    if (pending & 0x1<<19) return 57;
    if (pending & 0x1<<20) return 62;
  /*  20 R GPU IRQ 62
    19 R GPU IRQ 57
    18 R GPU IRQ 56
    17 R GPU IRQ 55
    16 R GPU IRQ 54
    15 R GPU IRQ 53
    14 R GPU IRQ 19
    13 R GPU IRQ 18
    12 R GPU IRQ 10
    11 R GPU IRQ 9
    10 R GPU IRQ 7*/
    //some higher irq's are pending in the basic_reg! return these
  }

  if (pending &0x100)
  {
    kprintf("Pending in reg 1\n");
    //written out for "readability"
    return __builtin_ffs(irq_regs->gpu_irq_pending[0])-1;
    //decode higher irq
  }
  if (pending &0x200)
  {
    kprintf("Pending in reg 2\n");
    //written out for "readability"
    return 32+__builtin_ffs(irq_regs->gpu_irq_pending[1])-1;
  }

  if (pending&0xFF)
  {
    kprintf("Pending in basic\n");
    //written out for "readability"
    return 64+__builtin_ffs(pending&0xFF)-1;
  }
  //if we reached this point serve the irq..
  /*if (basic_regs &0x1) return 64; //arm timer irq
  if (basic_regs &0x2) return 65; //arm mbox irq
  if (basic_regs &0x3)*/
  return 0x3ff; //spurious?=
}

void bcm2835_irq_disable(uint32_t irq)
{
  //this is more sane..
  if (irq >=72) return;

  uint32_t bank=irq/32;
  uint32_t pin=irq-(bank*32);
  switch(bank)
  {
    case 0:
    case 1:
      irq_regs->gpu_irq_disable[bank]=(0x1<<pin);
      break;
    case 3:
      irq_regs->basic_irq_disable=(0x1<<pin);
      break;
  }
}

void bcm2835_irq_enable(uint32_t irq)
{
  if (irq >=72) return;

  uint32_t bank=irq/32;
  uint32_t pin=irq-(bank*32);

  kprintf("enable irq %d on bank %d\n",irq,bank);
  switch(bank)
  {
    case 0:
    case 1:
      kprintf("addr %zx %08x\n",&irq_regs->gpu_irq_enable[bank],0x1<<pin);
      irq_regs->gpu_irq_enable[bank]=(0x1<<pin);
      break;
    case 3:
      irq_regs->basic_irq_enable=(0x1<<pin);
      break;
  }
}


/**
BASIC pending
31:21 - <unused>
20 R GPU IRQ 62
19 R GPU IRQ 57
18 R GPU IRQ 56
17 R GPU IRQ 55
16 R GPU IRQ 54
15 R GPU IRQ 53
14 R GPU IRQ 19
13 R GPU IRQ 18
12 R GPU IRQ 10
11 R GPU IRQ 9
10 R GPU IRQ 7
9 R One or more bits set in pending register 2
8 R One or more bits set in pending register 1
7 R Illegal access type 0 IRQ pending
6 R Illegal access type 1 IRQ pending
5 R GPU1 halted IRQ pending
4 R GPU0 halted IRQ pending
(Or GPU1 halted if bit 10 of control register 1 is set)
3 R ARM Doorbell 1 IRQ pending
2 R ARM Doorbell 0 IRQ pending
1 R ARM Mailbox IRQ pending
0 R ARM Timer IRQ pending*/

/*
FIQ ctrl
1:8 R <unused>
7 R FIQ enable. Set this bit to 1 to enable FIQ generation.
If set to 0 bits 6:0 are don't care.
6:0 R/W Select FIQ Source

fiq sources
0-63 GPU Interrupts (See GPU IRQ table)
64 ARM Timer interrupt
65 ARM Mailbox interrupt
66 ARM Doorbell 0 interrupt
67 ARM Doorbell 1 interrupt
68 GPU0 Halted interrupt (Or GPU1)
69 GPU1 Halted interrupt
70 Illegal access type-1 interrupt
71 Illegal access type-0 interrupt
72-127 Do Not Use
**/

/**
Enable /disable basic irq's
31:8 R/Wbs <Unused>
7 R/Wbs Set to enable Access error type -0 IRQ
6 R/Wbs Set to enable Access error type -1 IRQ
5 R/Wbs Set to enable GPU 1 Halted IRQ
4 R/Wbs Set to enable GPU 0 Halted IRQ
3 R/Wbs Set to enable ARM Doorbell 1 IRQ
2 R/Wbs Set to enable ARM Doorbell 0 IRQ
1 R/Wbs Set to enable ARM Mailbox IRQ
0 R/Wbs Set to enable ARM Timer IRQ
*/


/**
# IRQ 0-15 # IRQ 16-31 # IRQ 32-47 # IRQ 48-63
16 32 48 smi
1 17 33 49 gpio_int[0]
2 18 34 50 gpio_int[1]
3 19 35 51 gpio_int[2]
4 20 36 52 gpio_int[3]
5 21 37 53 i2c_int
6 22 38 54 spi_int
7 23 39 55 pcm_int
8 24 40 56
9 25 41 57 uart_int
10 26 42 58
11 27 43 i2c_spi_slv_int 59
12 28 44 60
13 29 Aux int 45 pwa0 61
14 30 46 pwa1 62
15 31 47 63

*/
