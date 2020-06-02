/*
 * Copyright (c) 2020, Dermot Tynan / Kalopa Research.  All rights
 * reserved.
 *
 * See the LICENSE file for more info.
 *
 * ABSTRACT
 * The main include file.
 */
#define HERTZ				250

#define ACCEL_ADDRESS		0x53
#define GYRO_ADDRESS		0x68

#define MIN(a, b)			((a) < (b) ? (a) : (b))
#define MAX(a, b)			((a) > (b) ? (a) : (b))

/*
 * Central data structure for all control data.
 */
struct control {
	uint_t		rx_throttle;
	uint_t		rx_roll;
	uint_t		rx_pitch;
	uint_t		rx_yaw;
	uint_t		rx_gear;
	uint_t		rx_aux[7];
	uint_t		g_roll;
	uint_t		g_pitch;
	uint_t		g_yaw;
	struct pid	p_roll;
	struct pid	p_pitch;
	struct pid	p_yaw;
	uchar_t		state;
	uchar_t		esc_fr;
	uchar_t		esc_fl;
	uchar_t		esc_rr;
	uchar_t		esc_rl;
};

#define STATE_DISARMED	0
#define STATE_IDLE		1
#define STATE_INFLIGHT	2

extern struct control	control;
extern volatile uchar_t	waitf;

/*
 * Prototypes...
 */
void	eeprom_init();
void	clock_init();
void	serial_init();
void	gyro_init(struct control *);
void	control_init(struct control *);
void	receiver_init(struct control *);

void	i2c_init();
uchar_t	i2c_read();
uchar_t	i2c_write();
void	gyro_calibrate(struct control *);
void	gyro_read(struct control *);
void	calculate(struct control *);
void	start_esc_outputs(struct control *);

void	clocktick();
void	waittick();
void	delay(int);

/*
 * These are implemented in assembly language in locore.s and others.
 */
void	_reset();
void	_idle();
void	_sio_rxinton();
void	_sio_rxintoff();
void	_sio_txinton();
void	_sio_txintoff();

void	_setled(uchar_t);
void	_testpt(uchar_t);
void	_watchdog();
