#ifndef GPIO_HPP_INCLUDED // This is different from "GPIO_H_INCLUDED", which is from <arch/aarch64/gpio.h>
#define GPIO_HPP_INCLUDED

#include <stdio.h>

typedef unsigned int __uint32_t;
typedef unsigned char __uint8_t;
namespace hw
{
	class GPIO
	{
		public:
			/* Read 32 bit value from a peripheral address. */
			__uint32_t read_peri(volatile __uint32_t *paddr);

			/*  Write 32 bit value to a peripheral address. */ 
			void write_peri(volatile __uint32_t *paddr, __uint32_t value);

			/* Set bits in the address. */
			void gpio_set_bits(volatile __uint32_t *paddr, __uint32_t mask, __uint32_t value);

			/* Select the function of GPIO. */
			void gpio_func_select(const __uint8_t pin, __uint8_t mode);

			/* Set output pin. */
			void gpio_set(const __uint8_t pin);

			/* Clear output pin. */
			void gpio_clr(const __uint8_t pin);

			/* Read the current level of input pin. */
			int gpio_read_level(const __uint8_t pin);

			/* Get the time. */
			__uint32_t gpio_timer(void);
			
			/* Delay count. */
			int gpio_delay(int count);
	};
};

#include <arch/aarch64/gpio.h>

#define HIGH 1
#define LOW 0

#define GPIO_FSEL_INPT 0x00  /*!< Input 0b000 */
#define GPIO_FSEL_OUTP 0x01  /*!< Output 0b001 */
#define GPIO_FSEL_ALT0 0x04  /*!< Alternate function 0 0b100 */
#define GPIO_FSEL_ALT1 0x05  /*!< Alternate function 1 0b101 */
#define GPIO_FSEL_ALT2 0x06  /*!< Alternate function 2 0b110, */
#define GPIO_FSEL_ALT3 0x07  /*!< Alternate function 3 0b111 */
#define GPIO_FSEL_ALT4 0x03  /*!< Alternate function 4 0b011 */
#define GPIO_FSEL_ALT5 0x02  /*!< Alternate function 5 0b010 */
#define GPIO_FSEL_MASK 0x07  /*!< Function select bits mask 0b111 */

#endif