#include <kprint>
#include <info>
#include <service>
#include <smp>
#include <boot/multiboot.h>
#include <kernel/os.hpp>

extern "C" {
#include <solo5.h>
}

extern  void solo5_stdout_handlers();
extern  void __platform_init();


__attribute__ ((weak))
void solo5_stdout_handlers()
{
  OS::add_stdout_solo5();
}

extern "C" {
  void __init_sanity_checks();
  void kernel_sanity_checks();
  uintptr_t _move_symbols(uintptr_t loc);
  void _init_bss();
  void _init_heap(uintptr_t);
  void _init_c_runtime();
  void _init_syscalls();
  void __libc_init_array();
  uintptr_t _end;
  void __set_stack();

  void solo5_poweroff()
  {
    __asm__ __volatile__("cli; hlt");
    for(;;);
  }

  void kernel_start(char *cmdline){

    // generate checksums of read-only areas etc.
    __init_sanity_checks();

    // Determine where free memory starts
    uintptr_t free_mem_begin = reinterpret_cast<uintptr_t>(&_end);

    // Preserve symbols from the ELF binary
    free_mem_begin += _move_symbols(free_mem_begin);

    // Do not zero out all solo5 global variables!! == don't touch the BSS
    //_init_bss();

    // Initialize heap
    // XXX: this is dangerous as solo5 might be doing malloc()'s using it's own
    // idea of a heap. Luckily there is no malloc instance at solo5/kernel/[ukvm|virtio|muen],
    // so might be OK (for now).
    _init_heap(free_mem_begin);

    //Initialize stack-unwinder, call global constructors etc.
    _init_c_runtime();

    // Initialize system calls
    _init_syscalls();

    //Initialize stdout handlers
    solo5_stdout_handlers();

    // Call global ctors
    __libc_init_array();

    // Initialize OS including devices
    OS::start(0, 0);

    // Starting event loop from here allows us to profile OS::start
    OS::event_loop();

    solo5_poweroff();
  }

  int solo5_app_main(char *cmdline) {
     // solo5 sets the stack to be at the end of memory.
     __set_stack();
  }
}
