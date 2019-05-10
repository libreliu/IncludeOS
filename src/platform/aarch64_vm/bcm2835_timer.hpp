//system timers

/*
The Physical (hardware) base address for the system timers is 0x7E003000.
12.1 System Timer Registers
ST Address Map
Address
Offset Register Name Description Size
0x0 CS System Timer Control/Status 32
0x4 CLO System Timer Counter Lower 32 bits 32
0x8 CHI System Timer Counter Higher 32 bits 32
0xc C0 System Timer Compare 0 32
0x10 C1 System Timer Compare 1 32
0x14 C2 System Timer Compare 2 32
0x18 C3 System Timer Compare 3 32
*/

#include <cstdint>

void bcm2835_system_timer_init(uintptr_t base_addr);

uint64_t bcm2835_system_timer_read();

enum class bcm2835_system_timer
{
  timer0=0,
  timer1=1,
  timer2=2,
  timer3=3,
};

void bcm2835_system_timer_set_compare(uint32_t ticks,bcm2835_system_timer timer=bcm2835_system_timer::timer0);
void bcm2835_system_timer_clear_status(bcm2835_system_timer timer);
int bcm2835_system_timer_get_status(bcm2835_system_timer timer);

/*
CLO Register
Synopsis System Timer Counter Lower bits.
*/
/*
The system timer free-running counter lower register is a read-only register that returns the current value
of the lower 32-bits of the free running counter.
Bit(s) Field Name Description Type Reset
31:0 CNT Lower 32-bits of the free running counter value. 0x0
*/
/*
CHI Register
Synopsis System Timer Counter Higher bits.
The system timer free-running counter higher register is a read-only register that returns the current value
of the higher 32-bits of the free running counter.
Bit(s) Field Name Description Type Reset
31:0 CNT Higher 32-bits of the free running counter value. 0x0
!
!
06 February 2012 Broadcom Europe Ltd. 406 Science Park Milton Road Cambridge CB4 0WW Page 174
 © 2012 Broadcom Corporation. All rights reserved
*/

/*
C0 C1 C2 C3 Register
Synopsis System Timer Compare.
The system timer compare registers hold the compare value for each of the four timer channels.
Whenever the lower 32-bits of the free-running counter matches one of the compare values the
corresponding bit in the system timer control/status register is set.
Each timer peripheral (minirun and run) has a set of four compare registers.
Bit(s) Field Name Description Type Reset
31:0 CMP Compare value for match channel n. RW 0x0
-**/


/*
Synopsis System Timer Control / Status.
This register is used to record and clear timer channel comparator matches. The system timer match bits
are routed to the interrupt controller where they can generate an interrupt.
The M0-3 fields contain the free-running counter match status. Write a one to the relevant bit to clear the
match detect status bit and the corresponding interrupt request line.
06 February 2012 Broadcom Europe Ltd. 406 Science Park Milton Road Cambridge CB4 0WW Page 173
 © 2012 Broadcom Corporation. All rights reserved
Bit(s) Field Name Description Type Reset
31:4 Reserved - Write as 0, read as don't care
3 M3 System Timer Match 3
0 = No Timer 3 match since last cleared.
1 = Timer 3 match detected.
RW 0x0
2 M2 System Timer Match 2
0 = No Timer 2 match since last cleared.
1 = Timer 2 match detected.
RW 0x0
1 M1 System Timer Match 1
0 = No Timer 1 match since last cleared.
1 = Timer 1 match detected.
RW 0x0
0 M0 System Timer Match 0
0 = No Timer 0 match since last cleared.
1 = Timer 0 match detected.
*/



//free running timer!!
/*The base address for the ARM timer register is 0x7E00B000.
Address
offset8
Description
0x400 Load
0x404 Value (Read Only)
0x408 Control
0x40C IRQ Clear/Ack (Write only)
0x410 RAW IRQ (Read Only)
0x414 Masked IRQ (Read Only)
0x418 Reload
0x41C Pre-divider (Not in real 804!)
0x420 Free running counter (Not in real 804!)
*/

/*
Name: Timer control Address: base + 0x40C Reset: 0x3E0020
Bit(s) R/W Function
31:10 - <Unused>
23:16 R/W Free running counter pre-scaler. Freq is sys_clk/(prescale+1)
These bits do not exists in a standard 804! Reset value is 0x3E
15:10 - <Unused>
9 R/W 0 : Free running counter Disabled
1 : Free running counter Enabled
This bit does not exists in a standard 804 timer!
8 R/W 0 : Timers keeps running if ARM is in debug halted mode
1 : Timers halted if ARM is in debug halted mode
This bit does not exists in a standard 804 timer!
7 R/W 0 : Timer disabled
1 : Timer enabled
6 R/W Not used, The timer is always in free running mode.
If this bit is set it enables periodic mode in a standard 804. That
mode is not supported in the BC2835M.
5 R/W 0 : Timer interrupt disabled
1 : Timer interrupt enabled
4 R/W <Not used>
3:2 R/W Pre-scale bits:
00 : pre-scale is clock / 1 (No pre-scale)
01 : pre-scale is clock / 16
10 : pre-scale is clock / 256
11 : pre-scale is clock / 1 (Undefined in 804)
1 R/W 0 : 16-bit counters
1 : 23-bit counter
0 R/W Not used, The timer is always in wrapping mode.
If this bit is set it enables one-shot mode in real 804. That mode is
not supported in the BCM2835
*/


/***
Timer Masked IRQ register:
The masked IRQ register is a read-only register. It shows the status of the interrupt signal. It is simply
a logical AND of the interrupt pending bit and the interrupt enable bit.
Name: Masked IRQ Address: base + 0x40C Reset: 0x3E0020
Bit(s) R/W Function
31:0 R 0
0 R 0 : Interrupt line not asserted.
1 :Interrupt line is asserted, (the interrupt pending and the interrupt
enable bit are set.)
*/
/*
Timer Reload register:
This register is a copy of the timer load register. The difference is that a write to this register does
not trigger an immediate reload of the timer value register. Instead the timer load register value is
only accessed if the value register has finished counting down to zero.
*/
/*
The timer pre-divider register:
Name: pre-divide Address: base + 0x41C Reset: 0x07D
Bit(s) R/W Function
31:10 - <Unused>
9:0 R/W Pre-divider value.
*/

/*The Pre-divider register is not present in the SP804.
06 February 2012 Broadcom Europe Ltd. 406 Science Park Milton Road Cambridge CB4 0WW Page 199
 © 2012 Broadcom Corporation. All rights reserved
The pre-divider register is 10 bits wide and can be written or read from. This register has been added
as the SP804 expects a 1MHz clock which we do not have. Instead the pre-divider takes the APB
clock and divides it down according to:
timer_clock = apb_clock/(pre_divider+1)
The reset value of this register is 0x7D so gives a divide by 126.
*/
/*
Free running counter
Name: Free running Address: base + 0x420 Reset: 0x000
Bit(s) R/W Function
31:0 R Counter value
*/
