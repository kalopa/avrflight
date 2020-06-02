/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
 *
 * ABSTRACT
 * All serial I/O happens here.
 */
#include <stdio.h>
#include <avr/io.h>

#include <libavr.h>

#include "flight.h"

#define BAUDRATE	8			/* 115,200 baud @ 16Mhz */

/*
 * Set up the serial I/O
 */
void
serial_init()
{
	/*
	 * Set baud rate and configure the USART.
	 */
	UBRR0H = (BAUDRATE >> 8) & 0xf;
	UBRR0L = (BAUDRATE & 0xff);
	/*UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);*/
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	(void )fdevopen(sio_putc, sio_getc);
}

/*
 * Initialize the data we receive from the remote control.
 */
void
receiver_init(struct control *cp)
{
	int i;

	cp->rx_throttle = cp->rx_roll = cp->rx_pitch = cp->rx_yaw = cp->rx_gear = 1000;
	for (i = 0; i < 7; i++)
		cp->rx_aux[i] = 1000;
}

/*
 *
 */
void
show_rx_data(struct control *cp)
{
	printf("T:%dR:%dP:%dY:%d\n", cp->rx_throttle, cp->rx_roll, cp->rx_pitch, cp->rx_yaw);
	PORTD |= 0x08;
}
