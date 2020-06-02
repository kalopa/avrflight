/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
 *
 * ABSTRACT
 * Functions associated with the specific gyro in use. In this case,
 * an ITG-3205.
 */
#include <stdio.h>
#include <avr/io.h>

#include <libavr.h>

#include "flight.h"

/*
 * Initialize the gyro. Set up our operating parameters.
 */
void
gyro_init(struct control *cp)
{
	cp->g_roll =  cp->g_pitch = cp->g_yaw = 0;
}

/*
 * Calibrate the gyro. This is done at boot-up. The point is to
 * pull a few thousand readings and then average out the error. We
 * then use the error to correct all future readings.
 */
void
gyro_calibrate(struct control *cp)
{
}

/*
 * Read the pitch, roll & yaw data from the gyro.
 */
void
gyro_read(struct control *cp)
{
}
