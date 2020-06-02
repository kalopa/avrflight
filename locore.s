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
; Interrupt Vector table.
	.arch   atmega328p
	.section .vectors,"ax",@progbits
	.global	__vectors
	.func	__vectors
__vectors:
	jmp		_reset				; Main reset
	jmp		nointr				; External Interrupt 0
	jmp		nointr				; External Interrupt 1
	jmp		nointr				; Pin Change Interrupt Request 0
	jmp		nointr				; Pin Change Interrupt Request 1
	jmp		nointr				; Pin Change Interrupt Request 2
	jmp		nointr				; Watchdog Timeout Interrupt
	jmp		nointr				; Timer 2 Compare Match A
	jmp		nointr				; Timer 2 Compare Match B
	jmp		nointr				; Timer 2 Overflow
	jmp		nointr				; Timer 1 Capture Event
	jmp		escint				; Timer 1 Compare Match A
	jmp		nointr				; Timer 1 Compare Match B
	jmp		nointr				; Timer 1 Counter Overflow
	jmp		clkint				; Timer 0 Compare Match A
	jmp		nointr				; Timer 0 Compare Match B
	jmp		nointr				; Timer 0 Counter Overflow
	jmp		nointr				; SPI Serial Transfer Complete
	jmp		sio_in	 			; USART Rx Complete
	jmp		sio_out				; USART Data Register Empty
	jmp		nointr				; USART Tx Complete
	jmp		nointr				; ADC Conversion Complete
	jmp		nointr				; EEPROM Ready
	jmp		nointr				; Analog Comparator
	jmp		nointr				; Two Wire Interface
	jmp		nointr				; Store Program Memory Ready
;
nointr:
	reti	 					; Return from interrupt
;
; Handle a clock tick. This is pretty simple - just update the idle
; count to reflect the fact that we've seen a clock tick. Then return.
	.text
	.section .init0,"ax",@progbits
clkint:
	push	r24					; Save the status register
	in		r24,SREG
	push	r24
	lds		r24,waitf
	inc		r24
	sts		waitf,r24
;
end_int:
	pop		r24					; Restore the status register
	out		SREG,r24
	pop		r24
	reti						; Return from interrupt
;
; Handle an interrupt from Timer1. We just turn off all four ESCs and
; declare victory. Remember to disable the interrupt as well.
escint:
	push	r24					; Save the status register
	in		r24,SREG
	push	r24
	in		r24,PORTD
	andi	r24,0x0f
	out		PORTD,r24
	sts		TIMSK1,r1			; Disable further interrupts
	rjmp	end_int
	.endfunc
;
; void _reset();
;
; To begin, initialize the stack frame, configure the timers and
; I/O pins, as well as the serial port (if fitted), enable interrupts,
; and start working.  The main loop here does several things.  The
; key task is to measure the windmill rotational velocity and try
; to keep it within bounds by altering the pulse width of the output.
	.text
	.section .init0,"ax",@progbits
	.global	_reset
	.func	_reset
_reset:
	.section .init2,"ax",@progbits
	cli
	clr		r1
	out		SREG,r1
	ldi		r16,lo8(RAMEND)		; Set up the stack pointer
	ldi		r17,hi8(RAMEND)
	out		SPH,r17
	out		SPL,r16

	ldi		r16,0xff			; DDRB=OOOO OOOO
	ldi		r17,0x01			; PORTB=0000 0001
	out		DDRB,r16
	out		PORTB,r17

	ldi		r16,0xf0			; DDRC=OOOOIIII
	ldi		r17,0x0f			; Pullups on low 4 bits
	out		DDRC,r16
	out		PORTC,r17

	ldi		r16,0xfe			; DDRD=OOOO OOOI
	out		DDRD,r16
	out		PORTD,r1			; PORTD=0000 0000
;
; Initialize BSS and data memory.
	ldi		r17,hi8(__data_end)		; End of data block
	ldi		r26,lo8(__data_start)	; X = start of data
	ldi		r27,hi8(__data_start)
	ldi		r30,lo8(__data_load_start)	; Z = program memory
	ldi		r31,hi8(__data_load_start)
	rjmp	bssl2
bssl1:
	lpm							; Fetch a byte from prog mem
	adiw	r30,1				; Increment Z
	st		x+,r0				; Save in [X]
bssl2:
	cpi		r26,lo8(__data_end) ; We done yet?
	cpc		r27,r17
	brne	bssl1				; No, copy some more

	ldi		r17,hi8(__bss_end)	; Point to end of memory
	rjmp	bssl4
bssl3:
	st		x+,r1				; Clear memory
bssl4:
	cpi		r26,lo8(__bss_end)	; We done yet
	cpc		r27,r17
	brne	bssl3				; No, keep trucking
;
; We're ready for prime time! Call the main C code.
	rcall	main 				; Call the main routine
	rjmp	_reset
	.endfunc
;
; void _idle();
;
; Wait for the waitf flag to increment (if it hasn't already happened)
; and keep the watchdog happy. Also use PORTD3 to indicate what we're
; doing, so we can measure the idle time.
	.section .init6,"ax",@progbits
	.global	_idle
	.func	_idle
_idle:
	in		r24,PORTD			; Set PORTD3=1 for idle timing
	ori		r24,0x08
	out		PORTD,r24
	wdr							; Clear the watchdog timer
;
	lds		r24,waitf
	tst		r24
	brne	idle1
	sleep						; Wait for an interrupt
	rjmp	_idle
;
idle1:
	in		r24,PORTD			; Set PORTD3=0 (end timer)
	andi	r24,0xf7
	out		PORTD,r24
	cli							; Decrement waitf (thread-safe)
	lds		r24,waitf
	subi	r24,1
	sts		waitf,r24
	sei
	ret
	.endfunc
;
; void sio_in();
;
; Serial I/O input interrupt routine (RX).
	.section .init6,"ax",@progbits
	.global	sio_in
	.func	sio_in
sio_in:
	push	r24					; Save the status register
	in		r24,SREG
	push	r24
	push	r25
	push	r26
	push	r27
;
	lds		r26,UDR0			; Pull the character
	lds		r24,ihead			; Get head & tail
	lds		r25,itail
	inc		r24
	andi	r24,31
	cp		r24,r25				; Buffer full?
	breq	siol1				; No choice but to drop the character
;
	lds		r24,ihead
	mov		r25,r26
	ldi		r26,lo8(iring)
	ldi		r27,hi8(iring)
	add		r26,r24				; Add the head pointer
	adc		r27,r1				; R1 is always zero!!!
	st		X,r25				; Save the character
	inc		r24
	andi	r24,31
	sts		ihead,r24			; Save the new head pointer
;
	rjmp	siol1				; We're done
	.endfunc
;
; void sio_out();
;
; Serial I/O output interrupt routine (TX).
	.global	sio_out
	.func	sio_out
sio_out:
	push	r24					; Save the status register
	in		r24,SREG
	push	r24
	push	r25
	push	r26
	push	r27
;
	ldi		r26,lo8(oring)		; Get the base ring buffer addr
	ldi		r27,hi8(oring)
	lds		r24,otail			; Add the tail pointer
	add		r26,r24
	adc		r27,r1				; R1 is always zero!!!
	ld		r25,X				; Get the character in the ring
	sts		UDR0,r25			; Transmit it
;
	inc		r24					; Increment the tail pointer
	andi	r24,31
	sts		otail,r24
	lds		r25,ohead			; Get the head pointer
	cp		r24,r25				; Buffer empty?
	brne	siol1				; No, continue
	rcall	_sio_txintoff
;
; Common return routine from interrupts.
siol1:
	pop		r27
	pop		r26
	pop		r25					; Restore the status register
	pop		r24
	out		SREG,r24
	pop		r24
	reti						; Return from interrupt
	.endfunc
;
; void _sio_rxinton();
;
; Enable transmission interrupts.
	.global _sio_rxinton
	.func   _sio_rxinton
_sio_rxinton:
	lds		r24,UCSR0B			; Enable RX ints
	ori		r24,0x80
	sts		UCSR0B,r24
	ret
	.endfunc
;
; void _sio_rxintoff();
;
; Disable transmission interrupts.
	.global	_sio_rxintoff
	.func	_sio_rxintoff
_sio_rxintoff:
	lds		r24,UCSR0B			; Disable RX ints
	andi	r24,0x7f
	sts		UCSR0B,r24
	ret
	.endfunc
;
; void _sio_txinton();
;
; Enable transmission interrupts.
	.global _sio_txinton
	.func   _sio_txinton
_sio_txinton:
	lds		r24,UCSR0B			; Enable TX ints
	ori		r24,0x20
	sts		UCSR0B,r24
	ret
	.endfunc
;
; void _sio_txintoff();
;
; Disable transmission interrupts.
	.global _sio_txintoff
	.func   _sio_txintoff
_sio_txintoff:
	lds		r24,UCSR0B			; Disable TX ints
	andi	r24,0xdf
	sts		UCSR0B,r24
	ret
	.endfunc
;
; Some useful (low level) variables...
	.weak	__heap_end
	.set	__heap_end,0
;
; Fin
