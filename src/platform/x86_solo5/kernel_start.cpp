#include <kprint>
#include <info>
#include <smp>
#include <kernel.hpp>
#include "../x86_pc/init_libc.hpp"

extern "C" {
#include <solo5/solo5.h>
}

extern void __platform_init();

extern "C" {
  void __init_sanity_checks();
  void kernel_sanity_checks();
  uintptr_t _move_symbols(uintptr_t loc);
  void _init_syscalls();
  void _init_elf_parser();
  uintptr_t _end;
  void set_stack();
  void* get_cpu_ebp();
}

static os::Machine* __machine = nullptr;
os::Machine& os::machine() noexcept {
  Expects(__machine != nullptr);
  return *__machine;
}

static char temp_cmdline[1024];
static uintptr_t mem_size = 0;
static uintptr_t free_mem_begin;
extern "C" void kernel_start();

extern "C"
int solo5_app_main(const struct solo5_start_info *si)
{
  // si is stored at 0x6000 by solo5 tender which is used by includeos. Move it fast.
  strncpy(temp_cmdline, si->cmdline, sizeof(temp_cmdline)-1);
  temp_cmdline[sizeof(temp_cmdline)-1] = 0;
  free_mem_begin = si->heap_start;
  mem_size = si->heap_size;

  // set the stack location to its new includeos location, and call kernel_start
  set_stack();
  return 0;
}

#include <kprint>
extern "C"
__attribute__ ((__optimize__ ("-fno-stack-protector")))
void kernel_start()
{
  // generate checksums of read-only areas etc.
  __init_sanity_checks();

  static char buffer[1024];

  // Preserve symbols from the ELF binary
  snprintf(buffer, sizeof(buffer),
          "free_mem_begin = %p\n", free_mem_begin);
  __serial_print1(buffer);
  const size_t len = _move_symbols(free_mem_begin);
  free_mem_begin += len;
  mem_size -= len;
  snprintf(buffer, sizeof(buffer),
          "free_mem_begin = %p\n", free_mem_begin);
  __serial_print1(buffer);

  // Initialize heap
  kernel::init_heap(free_mem_begin, mem_size);

  // Ze machine
  kprintf("Test1\n");
  __machine = os::Machine::create((void*)free_mem_begin, mem_size);
  kprintf("Test2\n");

  _init_elf_parser();

  // Begin portable HAL initialization
  __machine->init();

  // Initialize system calls
  _init_syscalls();

  x86::init_libc((uint32_t) (uintptr_t) temp_cmdline, 0);
}
