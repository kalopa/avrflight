/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
 *
 * ABSTRACT
 * Generic I2C interface functions.
 */
#include <stdio.h>
#include <avr/io.h>

#include <libavr.h>

#include "flight.h"

/*
 * Initialize the I2C circuit in the Atmel chip.
 */
void
i2c_init()
{
	TWBR = 12;
}

/*
 * Read a value from the I2C bus.
 */
uchar_t
i2c_read()
{
	return(0);
}

/*
 * Write a value to the I2C bus.
 */
uchar_t
i2c_write()
{
	return(0);
}
