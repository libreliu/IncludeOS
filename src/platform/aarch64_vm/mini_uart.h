//rpi mini uart..
#ifndef MINI_UART_H
#define MINI_UART_H

#include <cstddef>

#if defined(__cplusplus)
extern "C" {
#endif

void __mini_uart_print1(const char *cstr);

void __mini_uart_print(const char* str, size_t len);

void kprint(const char* c);

void kprintf(const char *format, ...);

#if defined(__cplusplus)
} //extern "C" {
#endif

#endif
