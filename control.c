/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
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

#define THROTTLE_ENGAGED	1200
#define ESC_IDLE_SPEED		12

/*
 * Initialize the control surfaces. We kick things off with the ESCs in a
 * STOP state (not moving) and set all four speeds to 1.0ms.
 */
void
control_init(struct control *cp)
{
	if (eeprom_rdword(EADDR_MAGIC) != 0x55aa) {
		set_state(cp, STATE_ERROR);
		return;
	}
	cp->esc_fr = cp->esc_fl = cp->esc_rr = cp->esc_rl = 0;
	cp->p_roll.e_sigma = cp->p_roll.e_prev = 0;
	cp->p_pitch.e_sigma = cp->p_pitch.e_prev = 0;
	cp->p_yaw.e_sigma = cp->p_yaw.e_prev = 0;
	cp->esc_div = eeprom_rdword(EADDR_ESC_DIV);
	/*
	 * Set up PID for roll.
	 */
	cp->p_roll.kp = eeprom_rdword(EADDR_ROLL_KP);
	cp->p_roll.ki = eeprom_rdword(EADDR_ROLL_KI);
	cp->p_roll.kd = eeprom_rdword(EADDR_ROLL_KD);
	cp->p_roll.k_div = eeprom_rdword(EADDR_ROLL_KDIV);
	cp->p_roll.u_mul = eeprom_rdword(EADDR_ROLL_UMUL);
	cp->p_roll.u_div = eeprom_rdword(EADDR_ROLL_UDIV);
	/*
	 * Set up PID for pitch.
	 */
	cp->p_pitch.kp = eeprom_rdword(EADDR_PITCH_KP);
	cp->p_pitch.ki = eeprom_rdword(EADDR_PITCH_KI);
	cp->p_pitch.kd = eeprom_rdword(EADDR_PITCH_KD);
	cp->p_pitch.k_div = eeprom_rdword(EADDR_PITCH_KDIV);
	cp->p_pitch.u_mul = eeprom_rdword(EADDR_PITCH_UMUL);
	cp->p_pitch.u_div = eeprom_rdword(EADDR_PITCH_UDIV);
	/*
	 * Set up PID for yaw.
	 */
	cp->p_yaw.kp = eeprom_rdword(EADDR_YAW_KP);
	cp->p_yaw.ki = eeprom_rdword(EADDR_YAW_KI);
	cp->p_yaw.kd = eeprom_rdword(EADDR_YAW_KD);
	cp->p_yaw.k_div = eeprom_rdword(EADDR_YAW_KDIV);
	cp->p_yaw.u_mul = eeprom_rdword(EADDR_YAW_UMUL);
	cp->p_yaw.u_div = eeprom_rdword(EADDR_YAW_UDIV);
}

/*
 * Compute the new ESC values.
 */
void
calculate(struct control *cp)
{
	int roll_u, pitch_u, yaw_u;

	/*
	 * Set the props to idling, unless we're in-flight.
	 */
	if (cp->state == STATE_IDLE) {
		if (cp->rx_throttle >= THROTTLE_ENGAGED)
			set_state(cp, STATE_INFLIGHT);
		else
			cp->esc_fr = cp->esc_fl = cp->esc_rr = cp->esc_rl = ESC_IDLE_SPEED;
		return;
	}
	/*
	 * Compute control outputs using PIDs
	 */
	roll_u = pidcalc(&cp->p_roll, cp->rx_roll - cp->g_roll);
	pitch_u = pidcalc(&cp->p_pitch, cp->rx_pitch - cp->g_pitch);
	yaw_u = pidcalc(&cp->p_yaw, cp->rx_yaw - cp->g_yaw);
	/*
	 * Adjust each ESC based on control outputs.
	 */
	cp->esc_fr = MIN((cp->rx_throttle - roll_u + pitch_u - yaw_u) / cp->esc_div, 250);
	cp->esc_fl = MIN((cp->rx_throttle + roll_u + pitch_u + yaw_u) / cp->esc_div, 250);
	cp->esc_rr = MIN((cp->rx_throttle - roll_u - pitch_u + yaw_u) / cp->esc_div, 250);
	cp->esc_rl = MIN((cp->rx_throttle + roll_u - pitch_u - yaw_u) / cp->esc_div, 250);
#if 0
	printf("%02x/%02x/%02x/%02x\n", cp->esc_fr, cp->esc_fl, cp->esc_rr, cp->esc_rl);
#endif
	cp->esc_fr = 0;		/* For testing purposes */
	cp->esc_fl = 50;
	cp->esc_rr = 100;
	cp->esc_rl = 250;
}
