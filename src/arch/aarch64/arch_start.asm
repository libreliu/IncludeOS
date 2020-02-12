/*
*/
#include "macros.h"
.text
.global __arch_start
.global fast_kernel_start
.global init_serial
.global drop_el1_secure

.extern exception_vector
//linker handles this
//.extern kernel_start
//not sure if this is a sane stack location
//.set STACK_LOCATION, 0x200000 - 16
.extern __stack_top
.extern __exception_stack
.extern __setup_pagetable
//__stack_top:

.align 8
__arch_start:

  /* Turn off Dcache and MMU */
//el3 is unhandled
/*
execution_level_switch x12, 3f, 2f, 1f
3:
  b 0f
2:
  mrs	x0, sctlr_el2
  bic	x0, x0, #1 << 0	// clear SCTLR.M
  bic	x0, x0, #1 << 2	// clear SCTLR.C
//  pre_disable_mmu_workaround
  msr	sctlr_el2, x0
  isb
  b	0f
1:
  mrs	x0, sctlr_el1
  bic	x0, x0, #1 << 0	// clear SCTLR.M
  bic	x0, x0, #1 << 2	// clear SCTLR.C
  //pre_disable_mmu_workaround
  msr	sctlr_el1, x0
  isb
0:
*/
  // read cpu id, stop slave cores ?

  mrs     x1, mpidr_el1
  and     x1, x1, #3
  cbz     x1, 2f
  // cpu id > 0, stop
1:  wfe
  b       1b
2:  // cpu id == 0
    //store any boot magic if present ?
//  bl clear_bss

    //pointless on PI ?!!
//  mrs x0, s3_1_c15_c3_0
//  bl enable_fpu_simd
  //  ;; Create stack frame for backtrace

  /*  ldr w0,[x9]
    ldr w1,[x9,#0x8]
*/

  bl final_start

  //unreachable

  execution_level_switch x12, 3f, 2f, 1f
3:
  bl drop_el1_non_secure
//  bl drop_el1_secure
2:
//TODOOO
//Setup page table for EL1..
  mrs x0,ttbr0_el2
  msr ttbr0_el1,x0
/*  mov x0,xzr
  orr x0,
  msr tcr_el1,x0*/
  ldr x0, =__exception_stack
  mov sp, x30
  bl register_exception_handler
  //register L2 handlers

  //msr sp_el1, x0
  //bl register_exception_handler no point need to be in el3 to register el2 handler ?
  //switch down to EL1
  bl drop_el1_non_secure
1:

el1_start:
final_start:

//Based on the EL some magic is required to make "everything" work
  execution_level_switch x12, 3f, 2f, 1f
3:
  //TODO ?
  b 0f
2:
  //enable interrupt to EL2
  mrs x0, hcr_el2
  orr x0,x0,#0x1<<5 //async route irq to EL2
  orr x0,x0,#0x1<<4 //irq route to EL2
  msr hcr_el2,x0
  b 0f
1:
  b 0f
0:


  ldr x30, =__stack_top
  mov sp, x30
//    mrs x0, s3_1_c15_c3_0
//  bl __setup_pagetable
  bl register_exception_handler
//  bl enable_fpu_simd

  ldr x8, =__boot_magic
  ldr x0, [x8]
  //ldr x1, =__regs
  //ldr x,[x8]
  //TODO check if spsel is the issue

/*  ldr x30, =__exception_stack
  mov sp, x30*/
//el1_start:
  bl kernel_start
    //;; hack to avoid stack protector
    //;; mov DWORD [0x1014], 0x89abcdef

/*
clear_bss:
  // clear bss ?
  ldr     x1, =_BSS_START_
  ldr     x3, =_BSS_END_
  sub     x2,x3,#1 //subtract X1 from X3 ?
  //ldr     w2, =__bss_size
3:
  cbz     x2, 4f //when done jump to 4
  str     xzr, [x1], #8
  sub     x2, x2, #1
  cbnz    x2, 3b

4:
  ret
*/
register_exception_handler:
  ldr x0, =exception_vector
  execution_level_switch x12, 3f, 2f, 1f

3:
//TODO verify that this isnt needed  msr sp_el3, x1
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

    /* Drop from el3 to el2. */

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
    //drop from el2 to el1
    ldr x1,=__stack_top
    msr sp_el1 ,x1

  //  msr sctlr_el1, xzr

    /* Initialize Generic Timers */
    mrs	x0, cnthctl_el2
    orr	x0, x0, #0x3	/* Enable EL1 access to timers */
    msr	cnthctl_el2, x0
    msr	cntvoff_el2, xzr


    // Disable stage 2 translations.
    msr vttbr_el2, xzr

    /* Disable coprocessor traps */
    mov	x0, #0x33ff
    msr	cptr_el2, x0	/* Disable coprocessor traps to EL2 */

    /*=============================================================*/
    /*      Enable FP/SIMD at EL1                                  */
    /*=============================================================*/
    mov	x0, #(0x3 << 20)
    msr	cpacr_el1, x0               /* Enable FP/SIMD at EL1. */



  //  msr	hstr_el2, xzr		/* Disable coprocessor traps to EL2 */

    /* Initilize MPID/MPIDR registers */
  	/*mrs	x0, midr_el1
  	mrs	x1, mpidr_el1
  	msr	vpidr_el2, x0
  	msr	vmpidr_el2, x1
*/



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
    orr	x0,	x0, #(1 << 5)
//    msr	sctlr_el1, x0

    mov x0 , #0x0800
    movk x0, #0x30d0, lsl #16
    msr	sctlr_el1, x0
    //msr	sctlr_el1, xzr

    /*=============================================================*/
    /*      Initialize hcr_el2                                     */
    /*=============================================================*/
    mrs x0, hcr_el2
    mov x0, xzr
    orr x0, x0, #(1<<31)            /* RW=1 EL1 Execution state is AArch64. */
  //  orr x0, x0, #(1 << 1)   // SWIO hardwired on Pi3 i am going to kill someone FIXME TOOD
    msr hcr_el2, x0
  //  mrs x0, hcr_el2 //why a readback =? https://github.com/bztsrc/raspi3-tutorial/blob/master/0F_executionlevel/start.S

    //mov x0, #0b00101 //DAIF = 0000
    mov     x2, #0x3c5
    msr spsr_el2,x2

    /*=============================================================*/
    /*      Initialize spsr_el2                                    */
    /*=============================================================*/
    mov x0, xzr
    mov x0, #0b00101                /* EL1 */
    orr x0, x0, #(1 << 9)           /* Enable debug . */
    orr x0, x0, #(1 << 8)           /* Enable SError and External Abort. */
    orr x0, x0, #(1 << 7)           /* IRQ interrupt Process state mask. */
    orr x0, x0, #(1 << 6)           /* FIQ interrupt Process state mask. */
  //  msr spsr_el2, x0
//    mov x0, #0b00000
//    msr spsr_el1,x0 //set DAIF 0?
    //#define SPSR_ELX_DAIF           (0b1111 << 6)
    //#define SPSR_ELX_EL1H           (0b0101)

    /*=============================================================*/
    /*      Initialize elr_el2                                     */
    /*=============================================================*/
    adr x0, el1_start
    msr elr_el2, x0
    isb
    eret


drop_el1_non_secure:
  execution_level_switch x12, 3f, 2f, 1f

3:
  b el3_entry
2:
  b el2_entry
1:
  b el1_start
