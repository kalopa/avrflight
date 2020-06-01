/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Thanks to Graham Agnew for the great advice regarding the programming of the
 * ESC pulses (interleaved starts, with a single stop - trailing edge).
 *
 * ABSTRACT
 * This is where the various inputs (radio, gyro, etc) are combined and the ESC
 * values computed. ESC settings are on the scale of 0 to 250, where 0 represents
 * 1000ms and 250 is 2000ms. The code is called from the flight loop and it will
 * do the PID computation for pitch, roll & yaw, and update the ESC values accordingly.
 * It will then start each of the ESC pulses as appropriate, and set up a Timer1
 * interrupt to turn them all off.
 */
#include <stdio.h>
#include <avr/io.h>

#include <libavr.h>

#include "flight.h"

uchar_t		motors_running;
uchar_t		esc_vals[4];

/*
 * Initialize the control surfaces. We kick things off with the ESCs in a
 * STOP state (not moving) and set all four speeds to 1.0ms.
 */
void
control_init()
{
	int i;

	motors_running = 0;
	for (i = 0; i < 4; i++)
		esc_vals[i] = 0;
	esc_vals[0] = 0;		/* For testing purposes */
	esc_vals[1] = 50;
	esc_vals[2] = 100;
	esc_vals[3] = 250;
}

/*
 * Compute the end periods for each of the ESCs and then back-fit
 * the start times.
 */
void
start_esc_outputs()
{
	int i, maxval;
	uint_t stop_time, esc_starts[4];
	uchar_t port_bits;

	TIMSK1 = 0;
	PORTD &= 0x0f;
	if (motors_running == 0)
		return;
	for (i = maxval = 0; i < 4; i++) {
		if (maxval < esc_vals[i])
			maxval = esc_vals[i];
	}
	/*
	 * Calculate when we're turning off the ESCs. To figure
	 * this out, take the highest ESC value from the range. Add
	 * 250 clocks (the initial 1ms pulse) and then work backwards
	 * to find the various start times.
	 */
	stop_time = maxval + 250;
	for (i = 0; i < 4; i++)
		esc_starts[i] = stop_time - ((uint_t )esc_vals[i] + 250);
	/*
	 * Set the OCR1A time for the turn-off interrupt. Then loop
	 * through until the timer has hit each start time.
	 */
	TCNT1 = 0;
	OCR1A = stop_time;
	TIMSK1 = (1<<OCIE1A);
	for (port_bits = 0; port_bits != 0xf0;) {
		stop_time = TCNT1;
		if (esc_starts[0] <= stop_time)
			port_bits |= 0x10;
		if (esc_starts[1] <= stop_time)
			port_bits |= 0x20;
		if (esc_starts[2] <= stop_time)
			port_bits |= 0x40;
		if (esc_starts[3] <= stop_time)
			port_bits |= 0x80;
		PORTD |= port_bits;
	}
}
