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
ASRCS=	locore.s
CSRCS=	main.c control.c timer.c gyro.c i2c.c serial.c
BIN=	flight.elf
FIRMWARE=flight.hex

LFUSE=0xdf
HFUSE=0xdd
EFUSE=0xff

include ../libavr/avr.mk

do_program: $(FIRMWARE) promdata.eep
	sudo avrdude -p $(DEVICE) -c $(PROG) -U flash:w:$(FIRMWARE):i -U eeprom:w:promdata.eep:i
	rm $(FIRMWARE)

eeprom: promdata.eep
	sudo avrdude -p $(DEVICE) -c $(PROG) -U eeprom:w:promdata.eep:i

$(BIN):	$(OBJS)
	$(CC) -o $(BIN) $(LDFLAGS) $(OBJS) $(LIBS)

promdata.eep: pid_tune.rb
	./pid_tune.rb

$(OBJS): flight.h $(REGVALS)
