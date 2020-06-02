/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
 *
 * ABSTRACT
 * This is where it all kicks off. Have fun, baby!
 */
#include <stdio.h>
#include <avr/io.h>

#include <libavr.h>

#include "flight.h"

struct control	control;

/*
 *
 */
int
main()
{
	uchar_t hbticks, ledf, secticks;
	uint_t hbeat = 0x00f7;

 	/*
	 * Do the initialization first...
	 */
	control.state = STATE_DISARMED;
	eeprom_init();
	clock_init();
	serial_init();
	i2c_init();
	gyro_init(&control);
	receiver_init(&control);
	control_init(&control);
	SMCR |= 0x1;
	sei();
	/*
	 * Now calibrate the gyros.
	 */
	printf("\nQuadcopter flight controller v0.1.\n");
	gyro_calibrate(&control);
	waitf = 0;
	hbticks = secticks = 0;
	while (1) {
		/*
		 * Show the running heartbeat unless we're disarmed.
		 */
		if (++hbticks > 15) {
			hbticks = 0;
			ledf = (hbeat & 0x8000) ? 1 : 0;
			_setled(ledf);
			hbeat = (hbeat << 1) | ledf;
		}
		/*
		 * Read the gyro data, calculate the new ESC values accordingly, and
		 * implement them.
		 */
		gyro_read(&control);
		calculate(&control);
		start_esc_outputs(&control);
		/*
		 * Now wait for the next clock tick (every 4ms).
		 */
		_idle();
	}
}
