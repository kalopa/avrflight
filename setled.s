;
; Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
; reserved.
;
; See the LICENSE file for more info.
;
; ABSTRACT
;
.include "atmega328p.inc"
;
; void _setled(uchar_t);
;
; Turn on or off the LED.
	.global	_setled
	.func	_setled
_setled:
	tst		r24					; LED on or off?
	breq	sl1
	sbi		PORTB,5				; Turn on the LED
	ret
sl1:
	cbi		PORTB,5				; Turn off the LED
	ret
	.endfunc
;
; Fin
