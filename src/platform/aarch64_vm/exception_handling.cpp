//TODO move to arch
#include <array>

#include <kprint>
#include "gic.h"

#include "exception_handling.hpp"
#include "debug.hpp"

/*#include <hw/gpio.hpp>

hw::GPIO act_led2(29,hw::GPIO_DIR::OUTPUT);
*/
//hmm not sure if i like this design
static std::array<irq_handler_t, 0x3ff> handlers;

void register_handler(uint16_t irq, irq_handler_t handler)
{
  handlers[irq]=handler;
}

void register_handler(uint16_t irq)
{
  handlers[irq]=nullptr;
}

/*
#include <hw/gpio.hpp>

static hw::GPIO act_led2(29,hw::GPIO_DIR::OUTPUT);
*/
//is void correct ?
#if defined(__cplusplus)
extern "C" {
#endif

#include "cpu.h"

/*
hw::GPIO act_led2(29,hw::GPIO_DIR::OUTPUT);
*/
void exception_handler_irq_el(struct stack_frame *ctx,uint64_t esr)
{
  kprint("IRQ EXCEPTION\r\n");

  kprint("Interrupt\n");
  /*
  //TODO !! generic irq handling
  int irq=gicd_decode_irq();
  gicd_irq_disable(irq);
  //is this the role of the handler or the kernel ?
  gicd_irq_clear(irq);

  if (handlers[irq] != nullptr)
    handlers[irq]();

  gicd_irq_enable(irq);
*/
  while(1);
}

struct stack_frame {
  uint64_t elr;
  uint64_t x[30];
};

static inline void _dump_regs(struct stack_frame *ctx,uint64_t esr)
{
  kprintf(" esr %016zx , ",esr);
  kprintf("elr %016zx\n",ctx->elr);
  for (int i=0;i<30;i+=2)
  {
    kprintf(" x%02d %016zx , ",i,ctx->x[i]);
    kprintf("x%02d %016zx\n",i+1,ctx->x[i+1]);
  }
  kprintf(" x%02d %016zx \n",30,ctx->x[30]);
}

const char * instruction_error(uint32_t code)
{
  switch(code&0x3F)
  {
    case 0b000000:
      return "Address size fault in TTBR0 or TTBR1.";
    case 0b000101:
      return "Translation fault, 1st level.";

    case 0b000110:
      return "Translation fault, 2nd level.";

    case 0b000111:
      return "Translation fault, 3rd level.";

    case 0b001001:
      return "Access flag fault, 1st level.";

    case 0b001010:
      return "Access flag fault, 2nd level.";

    case 0b001011:
      return "Access flag fault, 3rd level.";

    case 0b001101:
      return "Permission fault, 1st level.";

    case 0b001110:
      return "Permission fault, 2nd level.";

    case 0b001111:
      return "Permission fault, 3rd level.";

    case 0b010000:
      return "Synchronous external abort.";

    case 0b011000:
      return "Synchronous parity error on memory access.";

    case 0b010101:
      return "Synchronous external abort on translation table walk, 1st level.";

    case 0b010110:
      return "Synchronous external abort on translation table walk, 2nd level.";

    case 0b010111:
      return "Synchronous external abort on translation table walk, 3rd level.";

    case 0b011101:
      return "Synchronous parity error on memory access on translation table walk, 1st level.";

    case 0b011110:
      return "Synchronous parity error on memory access on translation table walk, 2nd level.";

    case 0b011111:
      return "Synchronous parity error on memory access on translation table walk, 3rd level.";

    case 0b100001:
      return "Alignment fault.";

    case 0b100010:
      return "Debug event.";
  }
  return "RESERVED";
}





void exception_handler_syn_el(struct stack_frame *ctx, uint64_t esr)
{
  //maybe it doesnt work if the exception is from krpintf.. but thats odd.
  kprint("EXCEPTION\n");
  uint8_t type = (esr>>26 )&0x3F;
  uint32_t iss=esr&(~(0x7F<<25));
  print_memory("EC",(const char *)&type,1);
  print_memory("ISS",(const char *)&iss,4);
  uint8_t length=16;
  if (esr& (0x1<<25))
    length=32;
  //aRM arch manual page 2160 exception classes!!
  switch(type)
  {
    case 0b000000: //unknown reason
      kprint("Unknown reason\n");
      break;
    case 0b000001: //trapped WFI or WFE
      kprintf("exception WFI WFE\n");
      break;
    //we at least ignore these 4 for now
  /*  case 0b000011: //trapped mcr mrc with coproc 0b1111 aarch32 only
    case 0b000100: //-II- mmcr mmrc aarch32 only
    case 0b000101: //trapped mcr mrc with coproc 0b1110 aarch32 only
    case 0b000110: //trapped LDC STC aarch32 only
      break;
      */
    case 0b000111: //access to simd trapped by CPACR_EL1.FPEN or CPTR_ELx
      kprint("TRAPPED SIMD ACCESS EXCEPTION\n");
      break;
  /*  case 0b001000: //trapped access to VMRS aarch32 only
      break;
*/
    case 0b001001:
      kprint("Trapped PAUTH\n");
      break;

    case 0b001110:
      kprint("Illegal execution state\n");
      break;

    //https://static.docs.arm.com/ddi0487/db/DDI0487D_b_armv8_arm.pdf?_ga=2.122314763.1841537678.1557129617-1455969210.1541414548
    case 0b010001:
      //aarch32 SVC
    case 0b010010:
      //aarch32 hvc
    case 0b010011:
      //aarch32 smc
      break;

    case 0b010101:
      kprint("SVC\n");
      kprintf("SVC command %08x\n",esr); //supervisor call -> EL1
      break;
    case 0b010110:
      kprint("HVC\n"); //hypervisor call -> EL2
      break;
    case 0b010111:
      kprint("SMC\n"); //secure monitor call -> EL3
      break;
    case 0b011000:
      kprint("Trapped MSR MRS Or system instruction\n");
      break;
    case 0b011001:
      kprint("Trapped access to SVE nor reported to EC 0b000000\n");
      break;
    case 0b011010:
      kprint("Trapped ERET or ERETAA or ERETAB\n");
      break;
    case 0b011111:
      kprint("Implementation defined exception taken to EL3.. \n");
      break;
    case 0b100000:
      kprint("Instruction abort from lower EL\n");
      break;
    case 0b100001:
      kprint("0x21");
      kprintf("Reason: %s\n",instruction_error(esr));
      _dump_regs(ctx,esr);
      break;
    case 0b100010:
      kprint("PC alignement fault\n");
      break;
    case 0b100100:
      kprint("Data abort from lower EL\n");
      break;
    case 0b100101:
      kprint("Data abort from same EL\n");
      _dump_regs(ctx,esr);
      while(1);
      break;
    case 0b100110:
      kprint("SP alignement fault\n");
      break;
    case 0b101000: //aarch32 trapped FP exception
    case 0b101100: //aarch64 trapped FP exception
      kprint("Trapped FP exception\n");
      break;

      //http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0488c/CIHIDFFE.html
    case 0x2f:
      kprintf("SError\n");
      while(1);
      break;
    //brk
    //all 3x instructions are DEBUG oriented
    case 0x3C:
      kprint("BRK\n");
      kprintf("Reason: (BRK) code (%04x)\n",(((uint32_t)esr)&0xFFFF));
      _dump_regs(ctx,esr);
      break;
    default:
      kprint("Unhandled\n");
      while(1);
      //this triggers a new exception
      kprintf("UNHANDLED SYN EXCEPTION %08x type %08x length=%d\r\n",esr,type,length);

  }
}

void exception_handler_fiq_el(struct stack_frame *ctx,uint64_t esr)
{
  kprint("FIQ EXCEPTION\r\n");

//  kprintf("FIQ EXCEPTION el=%08x\r\n",cpu_get_current_el());
  //kprintf("FIQ EXCEPTION\n");
  while(1);
}

void exception_handler_serror_el(struct stack_frame *ctx,uint64_t esr)
{
  kprint("SERROR EXCEPTION\r\n");

//  kprintf("SERROR EXCEPTION el=%08x\r\n",cpu_get_current_el());
  //kprint("SYN EXCEPTION\r\n");
  //kprintf("SERROR EXCEPTION\n");
  while(1);
}

void exception_unhandled()
{
  kprint("EXCEPTION\r\n");

  //act_led2.clear();
  kprintf("UNHANDLED EXCEPTION\n");
  while(1);
}
#if defined(__cplusplus)
}
#endif
