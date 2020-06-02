#
# Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
# reserved.
#
# See the LICENSE file for more info.
#
# ABSTRACT
# Yes, this is the Makefile. What can I say? It makes stuff...
#
DEVICE=	atmega328p
ASRCS=	locore.s setled.s
CSRCS=	main.c control.c timer.c gyro.c serial.c i2c.c eeprom.c
BIN=	flight

LFUSE=0xdf
HFUSE=0xdd
EFUSE=0xff

include ../libavr/avr.mk

$(BIN):	$(OBJS)
	$(CC) -o $(BIN) $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJS): flight.h $(REGVALS)
