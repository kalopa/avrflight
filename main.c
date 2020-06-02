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

uint_t			hbeat;
struct control	control;

/*
 *
 */
int
main()
{
	uchar_t hbticks, ledf, secticks;

 	/*
	 * Do the initialization first...
	 */
	set_state(&control, STATE_INIT);
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
	if (control.state != STATE_ERROR)
		set_state(&control, STATE_CALIBRATE);
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
		 * What we do next depends on the mode of operation.
		 */
		switch (control.state) {
		case STATE_ERROR:
		case STATE_DISARMED:
			break;

		case STATE_CALIBRATE:
			gyro_read(&control);
			gyro_calibrate(&control);
			break;

		case STATE_IDLE:
		case STATE_INFLIGHT:
			/*
			 * Calculate the new ESC values accordingly, and
			 * implement them.
			 */
			gyro_read(&control);
			calculate(&control);
		}
		/*
		 * Now send the ESC outputs and wait for the next clock
		 * tick (every 4ms).
		 */
		start_esc_outputs(&control);
		_idle();
	}
}

/*
 *
 */
void
set_state(struct control *cp, uchar_t new_state)
{
	cp->state = new_state;
	printf("State: %d\n", new_state);
	switch (cp->state) {
	case STATE_INIT:
		hbeat = 0xffff;
		_setled(1);
		break;

	case STATE_ERROR:
		hbeat = 0x00ff;
		break;

	case STATE_CALIBRATE:
		hbeat = 0x0555;
		break;

	case STATE_DISARMED:
		hbeat = 0x0f0f;
		break;

	case STATE_IDLE:
	case STATE_INFLIGHT:
		hbeat = 0x00f7;
		break;
	}
}
