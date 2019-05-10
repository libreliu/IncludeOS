#include "mini_uart.h"
#include <hw/serial.hpp>

static char initialized __attribute__((section(".data"))) = 0x0;

typedef struct mini_uart_s {
  volatile uint32_t io;
  volatile uint32_t ier;
  volatile uint32_t iir;
  volatile uint32_t lcr;
  volatile uint32_t mcr;
  volatile uint32_t lsr;
  volatile uint32_t msr;
  volatile uint32_t scratch;
  volatile uint32_t cntl;
  volatile uint32_t stat;
  volatile uint32_t baud;
}mini_uart_t;


//const uint64_t mini_uart_base=
static mini_uart_t *mini_uart=(mini_uart_t *)0x3f215040;//0x00215040;
//7e in 32 bit ? WTH 0x7e215040;//0x00215040;
static inline void init_if_needed()
{
  if (initialized == true) return;
  initialized = true;
  //the following asumes that "bios" has turned on the uart in enable_uart=1 on rpi3+
  mini_uart->cntl=0;
  mini_uart->lcr=3; //8
  mini_uart->mcr=0;
  mini_uart->ier=0;
  mini_uart->iir=0xc6; //disable interrupts
  mini_uart->baud=270; //115200 afaik

  mini_uart->cntl=3; //enable tx & rx




  // properly initialize serial port
/*  hw::outb(port + 1, 0x00);    // Disable all interrupts
  hw::outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  hw::outb(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  hw::outb(port + 1, 0x00);    //                  (hi byte)
  hw::outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
  hw::outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold*/

}

void __mini_uart_print1(const char *cstr)
{
    init_if_needed();
    char prev='\0';
    while(*cstr) {
      if (*cstr == '\n' && prev !='\r')
      {
        while( not (mini_uart->lsr&0x20));
        mini_uart->io='\r';
      }
      //TODO if we did this properly we would have put it into a buffer
      //and waited for level irq
      while( not (mini_uart->lsr&0x20));
      mini_uart->io=*cstr;

      prev=*cstr;
      cstr++;
    }
}

void __mini_uart_print(const char* str, size_t len)
{
  init_if_needed();
  int extra=0;
  if (str[len-1]=='\n' && str[len-2]!='\r')
  {
    len-=1;
    extra=1;
  }
  for (size_t i = 0; i < len; i++) {
    while( not (mini_uart->lsr&0x20));
    mini_uart->io=str[i];
  }

  if (extra)
  {
    char extra[2]={'\r','\n'};
    for (size_t i = 0; i < 2; i++) {
      while( not (mini_uart->lsr&0x20));
      mini_uart->io=extra[i];
    }
  }
}

void kprint(const char* c)
{
  __mini_uart_print1(c);
}

void kprintf(const char *format, ...)
{

  char mini_uart_buffer[8096];
  va_list aptr;
  va_start(aptr, format);
  //vsnprintf(mini_uart_buffer, 4096, format, aptr);
  size_t len = vsnprintf(mini_uart_buffer, sizeof(mini_uart_buffer), format, aptr);
  __mini_uart_print(mini_uart_buffer,len);
  va_end(aptr);
}
