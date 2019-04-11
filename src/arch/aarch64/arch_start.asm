/*
*/
#include "macros.h"
.text
.global __arch_start
.global fast_kernel_start
.global init_serial
.global drop_el1_secure

//linker handles this
//.extern kernel_start
//not sure if this is a sane stack location
//.set STACK_LOCATION, 0x200000 - 16
.extern __stack_top
//__stack_top:

//;; @param: eax - multiboot magic
//;; @param: ebx - multiboot bootinfo addr
.align 8
__arch_start:
    //store any boot magic if present ?

    //
    mrs x0, s3_1_c15_c3_0
    bl enable_fpu_simd
  //  ;; Create stack frame for backtrace

  /*  ldr w0,[x9]
    ldr w1,[x9,#0x8]
*/
  /*  execution_level_switch x12, 3f, 2f, 1f
3:
    b drop_el1_secure
2:
    b drop_el1_non_secure
1:
*/
el1_start:
    ldr x8,__boot_magic
    ldr x0,[x8]
    mrs x0, s3_1_c15_c3_0
    bl register_exception_handler
    bl enable_fpu_simd
    ldr x30, =__stack_top
    mov sp, x30
    //TODO check if spsel is the issue
    bl kernel_start
    //;; hack to avoid stack protector
    //;; mov DWORD [0x1014], 0x89abcdef

register_exception_handler:
  ldr x0, exception_vector
  execution_level_switch x12, 3f, 2f, 1f

3:
  msr vbar_el3,x0
  mrs x0, scr_el3
  orr x0, x0, #0xf
  msr scr_el3, x0
  b 0f
2:
  msr vbar_el2, x0
  b 0f
1:
  msr vbar_el1, x0
0:
  ret

enable_fpu_simd:
  execution_level_switch x12, 3f, 2f, 1f
3:
  msr cptr_el3, xzr
  b 0f
2:
  mov x0, #0x33ff
  msr cptr_el2, x0
  b 0f
1:
  mov x0, #3 << 20
  msr cpacr_el1, x0
  b 0f
0:
  ret

//fast_kernel_start:
    /*;; Push params on 16-byte aligned stack
    ;; sub esp, 8
    ;; and esp, -16
    ;; mov [esp], eax
    ;; mov [esp+4], ebx
*/
//    b kernel_start

  /*  ;; Restore stack frame
    ;; mov esp, ebp
    ;; pop ebp
    ;; ret
*/

/*=============================================================*/
/*      drop down to configured ex                             */
/*=============================================================*/
//TODO improve to save SP and enable return to higher EL..

drop_el1_secure:

    /* Try drop from el3 to el1 secure */

    /*=============================================================*/
    /*      Enable FP/SIMD at EL1                                  */
    /*=============================================================*/
    mov	x0, #3 << 20
    msr	cpacr_el1, x0           /* Enable FP/SIMD at EL1 */

    /*=============================================================*/
    /*      Initialize sctlr_el1                                   */
    /*=============================================================*/
    mov x0, xzr
    orr	x0,	x0, #(1 << 29)          /* Checking http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500d/CIHDIEBD.html */
    orr	x0,	x0, #(1 << 28)          /* Bits 29,28,23,22,20,11 should be 1 (res1 on documentation) */
    orr	x0,	x0, #(1 << 23)
    orr	x0,	x0, #(1 << 22)
    orr	x0,	x0, #(1 << 20)
    orr	x0,	x0, #(1 << 11)
    msr	sctlr_el1, x0

    /*=============================================================*/
    /*      Initialize scr_el3                                     */
    /*=============================================================*/
    mrs x0, scr_el3
    orr x0, x0, #(1<<10)        /* Lower EL is 64bits */
    msr scr_el3, x0

    /*=============================================================*/
    /*      Initialize spsr_el3                                    */
    /*=============================================================*/
    mov x0, xzr
    mov x0, #0b00101            /* EL1 */
    orr x0, x0, #(1 << 8)       /* Enable SError and External Abort. */
    orr x0, x0, #(1 << 7)       /* IRQ interrupt Process state mask. */
    orr x0, x0, #(1 << 6)       /* FIQ interrupt Process state mask. */
    msr spsr_el3, x0

    /*=============================================================*/
    /*      Initialize elr_el3                                     */
    /*=============================================================*/
    adr x0, el1_start
    msr elr_el3, x0

    eret

el3_entry:

    /* Drop from el3 to el2 non secure. */

    /*=============================================================*/
    /*      Initialize sctlr_el2 & hcr_el2                         */
    /*=============================================================*/
    msr sctlr_el2, xzr
    mrs x0, hcr_el2
    orr x0, x0, #(1<<19)            /* SMC instruction executed in Non-secure EL1 is trapped to EL2 */
    msr hcr_el2, x0

    /*=============================================================*/
    /*      Initialize scr_el3                                     */
    /*=============================================================*/
    mrs x0, scr_el3
    orr x0, x0, #(1<<10)            /* RW EL2 Execution state is AArch64. */
    orr x0, x0, #(1<<0)             /* NS EL1 is Non-secure world.        */
    msr scr_el3, x0

    /*=============================================================*/
    /*      Initialize spsr_el3                                    */
    /*=============================================================*/
    mov x0, #0b01001                /* DAIF=0000                               */
    msr spsr_el3, x0                /* M[4:0]=01001 EL2h must match SCR_EL3.RW */

    /*=============================================================*/
    /*      Initialize elr_el3                                     */
    /*=============================================================*/
    adr x0, el2_entry               /* el2_entry points to the first instruction */
    msr elr_el3, x0
    eret

el2_entry:

    /* Drop from el2 to el1 non secure. */

    /*=============================================================*/
    /*      Enable FP/SIMD at EL1                                  */
    /*=============================================================*/
    mov	x0, #3 << 20
    msr	cpacr_el1, x0               /* Enable FP/SIMD at EL1. */

    /*=============================================================*/
    /*      Initialize sctlr_el1                                   */
    /*=============================================================*/
    mov x0, xzr
    orr	x0, x0, #(1 << 29)              /* Checking http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0500d/CIHDIEBD.html */
    orr	x0,	x0, #(1 << 28)              /* Bits 29,28,23,22,20,11 should be 1 (res1 on documentation) */
    orr	x0,	x0, #(1 << 23)
    orr	x0,	x0, #(1 << 22)
    orr	x0,	x0, #(1 << 20)
    orr	x0,	x0, #(1 << 11)
    msr	sctlr_el1, x0

    /*=============================================================*/
    /*      Initialize hcr_el2                                     */
    /*=============================================================*/
    mrs x0, hcr_el2
    orr x0, x0, #(1<<31)            /* RW=1 EL1 Execution state is AArch64. */
    msr hcr_el2, x0

    /*=============================================================*/
    /*      Initialize spsr_el2                                    */
    /*=============================================================*/
    mov x0, xzr
    mov x0, #0b00101                /* EL1 */
    orr x0, x0, #(1 << 8)           /* Enable SError and External Abort. */
    orr x0, x0, #(1 << 7)           /* IRQ interrupt Process state mask. */
    orr x0, x0, #(1 << 6)           /* FIQ interrupt Process state mask. */
    msr spsr_el2, x0

    /*=============================================================*/
    /*      Initialize elr_el2                                     */
    /*=============================================================*/
    adr x0, el1_start
    msr elr_el2, x0

    eret


drop_el1_non_secure:
  execution_level_switch x12, 3f, 2f, 1f

3:
  b el3_entry
2:
  b el2_entry
1:
  b el1_start
/*




3:
  mov x0,sp
  msr sp_el1,x0
  adr x0, fast_start
  msr elr_el3, x0
  b 0f
2:
  adr x0, __arch_start
  msr elr_el2, x0
  b 0f
1:
  mov x0,sp
  msr sp_el1,x0
  adr x0, fast_start
  msr elr_el1, x0
0:
  eret
*/
