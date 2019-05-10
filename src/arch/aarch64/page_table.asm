//taken directly from http://infocenter.arm.com/help/topic/com.arm.doc.dai0527a/DAI0527A_baremetal_boot_code_for_ARMv8_A_processors.pdf
#include "macros.h"
.global __setup_pagetable

__setup_pagetable:
//execution level switch ?
  execution_level_switch x12, 3f, 2f, 1f
  // Initialize translation table control registers
3:
  LDR X1, =0x3520 // 4GB space 4KB granularity
   // Inner-shareable.
  MSR TCR_EL3, X1 // Normal Inner and Outer Cacheable.
  LDR X1, =0xFF440400 // ATTR0 Device-nGnRnE ATTR1 Device.
  MSR MAIR_EL3, X1 // ATTR2 Normal Non-Cacheable.
   // ATTR3 Normal Cacheable.
  ADR X0, ttb0_base // ttb0_base must be a 4KB-aligned address.
  MSR TTBR0_EL3, X0
  b 0f
2:
  LDR X1, =0x3520 // 4GB space 4KB granularity
   // Inner-shareable.
  MSR TCR_EL2, X1 // Normal Inner and Outer Cacheable.
  LDR X1, =0xFF440400 // ATTR0 Device-nGnRnE ATTR1 Device.
  MSR MAIR_EL2, X1 // ATTR2 Normal Non-Cacheable.
   // ATTR3 Normal Cacheable.
  ADR X0, ttb0_base // ttb0_base must be a 4KB-aligned address.
  MSR TTBR0_EL2, X0
  b 0f
1:
  //TODO FIXME!!
  LDR X1, =0x3520 // 4GB space 4KB granularity
   // Inner-shareable.
  MSR TCR_EL2, X1 // Normal Inner and Outer Cacheable.
  LDR X1, =0xFF440400 // ATTR0 Device-nGnRnE ATTR1 Device.
  MSR MAIR_EL2, X1 // ATTR2 Normal Non-Cacheable.
   // ATTR3 Normal Cacheable.
  ADR X0, ttb0_base // ttb0_base must be a 4KB-aligned address.
  MSR TTBR0_EL2, X0
  b 0f
0:
  ret

// Put a 64-bit value with little endianness.
.macro PUT_64B high, low
  .word \low
  .word \high
.endm
// Create an entry pointing to a next-level table.
.macro TABLE_ENTRY PA, ATTR
  PUT_64B \ATTR, (\PA) + 0x3
.endm
// Create an entry for a 1GB block.
.macro BLOCK_1GB PA, ATTR_HI, ATTR_LO
  PUT_64B \ATTR_HI, ((\PA) & 0xC0000000) | \ATTR_LO | 0x1
.endm
// Create an entry for a 2MB block.
.macro BLOCK_2MB PA, ATTR_HI, ATTR_LO
  PUT_64B \ATTR_HI, ((\PA) & 0xFFE00000) | \ATTR_LO | 0x1
.endm

.align 12 // 12 for 4KB granule.
ttb0_base:
  TABLE_ENTRY level2_pagetable, 0
  TABLE_ENTRY level2_pagetable2, 0
  BLOCK_1GB 0x80000000, 0, 0x740
  BLOCK_1GB 0xC0000000, 0, 0x740
  PUT_64B 0x0,0x2 //last entry

.align 12 // 12 for 4KB granule.
level2_pagetable:
  .set ADDR, 0x000 // The current page address.
  .rept 0x1F8
  BLOCK_2MB (ADDR << 20), 0, 0x74C
  .set ADDR, ADDR+2
  .endr
  .rept 0x8
  BLOCK_2MB (ADDR << 20), 0x600000, 0x401
  .set ADDR, ADDR+2
  .endr
/*
Page level2 entry 7ff3fc0 = 6000003f000401
Page level2 entry 7ff3fc8 = 6000003f200401
Page level2 entry 7ff3fd0 = 6000003f400401
Page level2 entry 7ff3fd8 = 6000003f600401
Page level2 entry 7ff3fe0 = 6000003f800401
Page level2 entry 7ff3fe8 = 6000003fa00401
Page level2 entry 7ff3ff0 = 6000003fc00401
Page level2 entry 7ff3ff8 = 6000003fe00401
*/
.align 12 // 12 for 4KB granule.
level2_pagetable2:
  .set ADDR, 0x400 // The current page address.
  .rept 0x1
  BLOCK_2MB (ADDR << 20), 0x600000, 0x401
  .set ADDR, ADDR+2
  .endr
