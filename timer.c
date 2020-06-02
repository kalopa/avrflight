/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
 *
 * ABSTRACT
 * All the clock timer functionality is embedded in this file.
 */
#include <stdio.h>
#include <avr/io.h>

#include <libavr.h>

#include "flight.h"

volatile uchar_t	waitf;

/*
 * Initialize the time of day clock system. Here's the run-down of the usage:
 * Timer0 - used for RTC interrupts. Set at 1/64 divider, and CTC mode, with
 * OCR0A set to 250 (for a clock frequency of 1kHz).
 */
void
clock_init()
{
	/*
	 * Set up Timer0 for our time-of-day clock. We run the clock in CTC mode
	 * with OCR0A = 249 and a prescaler of 256, for a timer frequency of 250Hz.
	 */
	TCNT0 = 0;
	OCR0A = 249;
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS02);
	TIMSK0 = (1<<OCIE0A);
	TIFR0 = (1<<OCF0B)|(1<<OCF0A)|(1<<TOV0);
	waitf = 0;
	/*
	 * Set up Timer1 to drive the ESC servo signals. We run the timer at
	 * 250kHz by dividing the I/O clock by 64. We run the clock in
	 * normal mode and interrupt every time we hit OCR1A. The ISR
	 * updates the PORTD outputs and advances OCR1A to the next edge.
	 * Push the compare interrupt way out into the future as it will
	 * be set by the control logic.
	 */
	TCNT1 = 0;
	OCR1A = 25000;
	TCCR1A = 0;
	TCCR1B = (1<<CS11)|(1<<CS10);
	TCCR1C = 0;
	TIMSK1 = (1<<OCIE1A);
}
