.globl __boot_magic

.data
.align 8
__boot_magic:
  .dword 0

.text
.align 8
.globl _start
_start:
  b reset

.align 8
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
  //mrs x2, S3_1_C15_C3_0 // Read CBAR_EL1 into X2

 // in case someone one day provides us with a cookie
  ldr x8 , __boot_magic
  str x0, [x8]

  b __arch_start
