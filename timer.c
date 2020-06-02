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
	TIMSK1 = 0;
}

/*
 * Compute the end periods for each of the ESCs and then back-fit
 * the start times.
 */
void
start_esc_outputs(struct control *cp)
{
	uint_t maxval, start_fr, start_fl, start_rr, start_rl;
	uchar_t port_bits;

	TIMSK1 = 0;
	PORTD &= 0x0f;
	if (cp->state < STATE_IDLE)
		return;
	/*
	 * Calculate when we're turning off the ESCs. To figure   this
	 * out, take the highest ESC value from the range. Then figure
	 * out the starting order for each of the ESCs by subtracting
	 * that number of clocks (plus 1 to allow time for this code to
	 * settle).
	 */
	maxval = MAX(cp->esc_fr, MAX(cp->esc_fl, MAX(cp->esc_rl, cp->esc_rr))) + 1;
	start_fr = maxval - (uint_t )cp->esc_fr;
	start_fl = maxval - (uint_t )cp->esc_fl;
	start_rr = maxval - (uint_t )cp->esc_rr;
	start_rl = maxval - (uint_t )cp->esc_rl;
	/*
	 * Set the OCR1A time for the turn-off interrupt. Then loop
	 * through until the timer has hit each start time.
	 */
	TCNT1 = 0;
	OCR1A = maxval + 250;
	TIMSK1 = (1<<OCIE1A);
	for (port_bits = 0; port_bits != 0xf0;) {
		maxval = TCNT1;
		if (start_fr <= maxval)
			port_bits |= 0x10;
		if (start_rl <= maxval)
			port_bits |= 0x20;
		if (start_fl <= maxval)
			port_bits |= 0x40;
		if (start_rr <= maxval)
			port_bits |= 0x80;
		PORTD |= port_bits;
	}
}
