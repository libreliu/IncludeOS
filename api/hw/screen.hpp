#ifndef SCREEN_HPP_INCLUDED
#define SCREEN_HPP_INCLUDED

#include <arch/aarch64/gpio.h>
#include <arch/aarch64/mbox.h>

/* Initialize framebuffer. */
int screen_init();

/* Print string on the screen */
void screen_print(const char *s);

#endif