#include <macros.h>

.globl __boot_magic

.data
.align 8
__boot_magic:
  .dword 0

//exception.asm
.extern exception_vector

.text
.align 8
.globl _start
_start:
  b reset

.align 4
_MULTIBOOT_START_:
  // Multiboot header
  // Must be aligned, or we'll have a hard time finding the header
  .word 0x1BADB002
  .word 0x00010003
  .word 0 - 0x1BADB002 - 0x00010003
  .word _MULTIBOOT_START_
  .word _LOAD_START_
  .word _LOAD_END_
  .word _end
  .word _start

.globl reset
reset:
  //why ?
  mrs x2, S3_1_C15_C3_0 // Read CBAR_EL1 into X2

 // in case someone one day provides us with a cookie
  ldr x8 , __boot_magic
  str x0, [x8]


        //load the exception vector to x0
        //different tables for different EL's but thats a given..
        adr     x0, exception_vector
        msr     daifset, #0xF //disable all exceptions

        //do we need this switch?
        mrs     x1, CurrentEL //load current execution level
        cmp     x1, 0xc
        b.eq    3f
        cmp     x1, 0x8
        b.eq    2f
        cmp     x1, 0x4
        b.eq    1f
3:      msr     vbar_el3, x0
        mrs     x0, scr_el3
        orr     x0, x0, #0xf                    /* SCR_EL3.NS|IRQ|FIQ|EA */
        msr     scr_el3, x0
        msr     cptr_el3, xzr                   /* Enable FP/SIMD */

        b       0f
2:      msr     vbar_el2, x0
        mov     x0, #0x33ff
        msr     cptr_el2, x0                    /* Enable FP/SIMD */
        b       0f
1:      /* msr     vbar_el1, x0 */

        /* DISABLE ALIGNMENT CHECK, see P2025 on ARMv8a manual */
        mrs     x0, sctlr_el1
        ldr     x1, =0xfffffffd
        and     x0, x0, x1
        msr     sctlr_el1, x0
        mov     x0, #3 << 20
        msr     cpacr_el1, x0                   /* Enable FP/SIMD */
0:

  b __arch_start