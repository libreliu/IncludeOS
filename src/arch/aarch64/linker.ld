ENTRY(_start)

/*
 * TODO sort out memory location. which is platform specific..
 * + 0x40000000 to elf start for aarch64 vm..
 * + 0x02000000 to elf start for rpi3.. ? unsure.. in u-boot..
 * + 0x8000 is rpi3 outside u-boot
 * bare bare metal!!
 * 32-bit kernels are loaded to address 0x8000 by default, and 64-bit kernels to address 0x80000.
**/
SECTIONS
{
  /* PROVIDE ( _ELF_START_ = . + 0x200000 + 0x40000000); */
  PROVIDE ( _ELF_START_ = . + 0x200000 + 0x20000000);
  PROVIDE ( _LOAD_START_ = _ELF_START_); /* For convenience w. multiboot */
  __boot_magic = (_ELF_START_ - 16);
  __exception_stack = (__boot_magic - 16);
  __stack_top = (__exception_stack - 4096);

  . = _ELF_START_ + SIZEOF_HEADERS;

  .text ALIGN(0x1000):
  {
  PROVIDE( _TEXT_START_ = . );
    *(.text)
    *(.text.*)
    *(.gnu.linkonce.t*)
  }
  PROVIDE( _TEXT_END_ = . );

  /* Global offset-table. For dynamic linking */
  .got ALIGN(16) : {
      *(.got*)
  }

/**
 *  .preinit_array, .init_array, .fini_array
 *  from GNU LD default linker script
 */

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }

/**
  *   Global constructors
  *   Constructors are split into groups allowing the OS to use global ctors
  *   before the OS itself is initialized, while delaying the calls to service constructors
  *   until as much of the OS / C++ runtime as possible is ready.
  */

  /* OS / stdlib constructors */
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    */lib/lib*.a:*(.init_array* .ctors*)
    */platform/lib*.a:*(.init_array* .ctors*)
    PROVIDE_HIDDEN (__init_array_end = .);
  }

  /* Stdout g constructors */
  .stdout_ctors :
  {
    PROVIDE_HIDDEN (__stdout_ctors_start = .);
    */drivers/stdout/lib*.a:*(.init_array* .ctors*)
    PROVIDE_HIDDEN (__stdout_ctors_end = .);
  }

  /* Driver g constructors */
  .driver_ctors :
  {
    PROVIDE_HIDDEN (__driver_ctors_start = .);
    */drivers/lib*.a:*(.init_array* .ctors*)
    PROVIDE_HIDDEN (__driver_ctors_end = .);
  }

  /* Plugin constructors */
  .plugin_ctors :
  {
    PROVIDE_HIDDEN (__plugin_ctors_start = .);
    */plugins/lib*.a:*(.init_array* .ctors*)
    PROVIDE_HIDDEN (__plugin_ctors_end = .);
  }

  /* All other constructors */
  .service_ctors :
  {
    PROVIDE_HIDDEN (__service_ctors_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__service_ctors_end = .);
  }

  .fini_array :
  {
   PROVIDE_HIDDEN (__fini_array_start = .);
   KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
   KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
   PROVIDE_HIDDEN (__fini_array_end = .);
  }

  _EXEC_END_ = .;
  _READONLY_START_ = .;
  .config ALIGN(0x1000) : {
    _CONFIG_JSON_START_ = .;
    KEEP(*(.config))
    _CONFIG_JSON_END_ = .;
    BYTE(0);
  }

  .rodata :
  {
    _RODATA_START_ = .;
    *(.rodata*)
    *(.gnu.linkonce.r*)
    _RODATA_END_ = .;
  }

  .tdata ALIGN(16) :
  {
    _TDATA_START_ = .;
    *(.tdata .tdata.*)
    _TDATA_END_ = .;
    . = ALIGN(16);
  }
  .tbss :
  {
    _TBSS_START_ = .;
    *(.tbss .tbss.*)
    _TBSS_END_ = .;
    . = ALIGN(16);
  }

  .memdisk :
  {
    _DISK_START_ = .;
    *(.diskdata)
    _DISK_END_ = .;
  }

  /* For stack unwinding (exception handling)  */
  .eh_frame_hdr ALIGN(16):
  {
    KEEP(*(.eh_frame_hdr*))
  }
  .eh_frame ALIGN(16):
  {
    PROVIDE (__eh_frame_start = .);
    KEEP(*(.eh_frame))
    LONG (0);
  }

  .gcc_except_table :
  {
    *(.gcc_except_table)
  }
  _READONLY_END_ = .;
  . = ALIGN(16);
  .data :
  {
    _DATA_START_ = .;
    *(.data)
    *(.data.*)
    *(.gnu.linkonce.d*)
    _DATA_END_ = .;
  }
  . = ALIGN(16);
  .elf_symbols : {
    _ELF_SYM_START_ = .;
    LONG (0);
  }

  .bss ALIGN(0x100) :
  {
    _BSS_START_ = .;
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    _BSS_END_ = .;
  }
  . = ALIGN(16);

  _end = .;

  PROVIDE (end = .);
  PROVIDE (_ELF_END_ = .);
  PROVIDE (_LOAD_END_ = .);
}
