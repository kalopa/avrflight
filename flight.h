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
	uint_t		esc_div;
	uchar_t		state;
	uchar_t		esc_fr;
	uchar_t		esc_fl;
	uchar_t		esc_rr;
	uchar_t		esc_rl;
};

#define STATE_INIT			0
#define STATE_ERROR			1
#define STATE_CALIBRATE		2
#define STATE_DISARMED		3
#define STATE_IDLE			4
#define STATE_INFLIGHT		5

#define EADDR_MAGIC			0
#define EADDR_ROLL_KP		2
#define EADDR_ROLL_KI		4
#define EADDR_ROLL_KD		6
#define EADDR_ROLL_KDIV		8
#define EADDR_ROLL_UMUL		10
#define EADDR_ROLL_UDIV		12
#define EADDR_PITCH_KP		14
#define EADDR_PITCH_KI		16
#define EADDR_PITCH_KD		18
#define EADDR_PITCH_KDIV	20
#define EADDR_PITCH_UMUL	22
#define EADDR_PITCH_UDIV	24
#define EADDR_YAW_KP		26
#define EADDR_YAW_KI		28
#define EADDR_YAW_KD		30
#define EADDR_YAW_KDIV		32
#define EADDR_YAW_UMUL		34
#define EADDR_YAW_UDIV		36
#define EADDR_ESC_DIV		38

extern struct control	control;
extern volatile uchar_t	waitf;

/*
 * Prototypes...
 */
void	clock_init();
void	serial_init();
void	gyro_init(struct control *);
void	control_init(struct control *);
void	receiver_init(struct control *);
void	set_state(struct control *, uchar_t);

void	i2c_init();
uchar_t	i2c_read();
uchar_t	i2c_write();
void	gyro_calibrate(struct control *);
void	gyro_read(struct control *);
void	calculate(struct control *);
void	start_esc_outputs(struct control *);
void	show_rx_data(struct control *);

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
