#include <hw/serial.hpp>
#include <arch/aarch64/uart.h>
#include <stdarg.h>

//static const uint16_t port = 0x3F8; // Serial 1
static int initialized __attribute__((section(".data"))) = 0x0;

static const uint32_t UART_BASE=0x09000000;

//__attribute__((no_sanitize("all")))
static inline void init_if_needed()
{
  if (initialized == true) return;
  initialized = true;

  uart_init();
}

extern "C"
//__attribute__((no_sanitize("all")))
void __serial_print1(const char* cstr)
{
  init_if_needed();
  uart_puts((char *)cstr);
}
extern "C"
void __serial_print(const char* str, size_t len)
{
  init_if_needed();
  for (size_t i = 0; i < len; i++) {
    uart_send(str[i]);
  }
}

extern "C"
void kprint(const char* c){
  __serial_print1(c);
}

extern "C" void kprintf(const char* format, ...)
{
  char buf[8192];
  va_list aptr;
  va_start(aptr, format);
  vsnprintf(buf, sizeof(buf), format, aptr);
  __serial_print1(buf);
  va_end(aptr);
}
