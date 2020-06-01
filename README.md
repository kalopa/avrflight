# AVRFlight - Flight Control Software

Quadcopter flight controller using an Atmel AVR (ATmega328p) processor.

The point of this exercise is to control a quadcopter with an Atmel
ATmega328P and an IMU.
The question is, is the AVR chip fast enough to control the quad??

The quadcopter has four 2200KV motors and ESC controllers in the
following configuration:

![quadcopter top-view](/quad-motor.png "Quadcopter Top-down View")

By varying the speeds of the four motors relative to each other,
we can control the pitch, roll and yaw of the aircraft.
We receive flight inputs from a Spektrum DSM 9645 device which
transmits a serial data packet with the received control
inputs.
The device supports twelve inputs.

We use a series of PID controllers to determine the control
inputs for pitch, roll and yaw.
We then combine these with the throttle input, and adjust
accordingly for each ESC.
The ESCs are laid out as follows

| ESC #  | Position | Rotation | Atmel Port |
| --- | --- | --- | --- |
| 1 | Front-Right (FR) | CW | D4 |
| 2 | Rear-Left (RL) | CW | D5 |
| 3 | Front-Left (FL) | CCW | D6 |
| 4 | Rear-Right (RR) | CCW | D7 |

Initially, the board will use an Arduino Nano with the following
ports for control and data:

| Port | AVR Function | I/O | Arduino Nano | Label | Description |
| --- | --- | --- | --- | --- | --- |
| B0 | ICP1 | O | D8 | | |
| B1 | OC1A | O | D9 | | |
| B2 | OC1B | O | D10 | | |
| B3 | OC2A | O | D11 | | |
| B4 | MISO | O | D12 | | |
| B5 | SCK | O | D13 | LED | Built-in LED |
| C0 | ADC0 | P | A0 | | |
| C1 | ADC1 | P | A1 | | |
| C2 | ADC2 | P | A2 | | |
| C3 | ADC3 | P | A3 | | |
| C4 | ADC4 | O | A4 | SDA | I2C Interface (Data) |
| C5 | ADC5 | O | A5 | SCK | I2C Interface (Clock) |
| D0 | RX | I | RX | DSM | Spektrum Receive Data |
| D1 | TX | O | TX | TXD | USB Transmit Data |
| D2 | INT0 | O | D2 | | |
| D3 | OC2B | O | D3 | IDLE | Indicate time in the IDLE loop |
| D4 | T0 | O | D4 | ESC_FR | Front-Right ESC |
| D5 | OC0B | O | D5 | ESC_RL | Rear-Left ESC |
| D6 | OC0A | O | D6 | ESC_FL | Front-Left ESC |
| D7 | AIN1 | O | D7 | ESC_RR | Rear-Right ESC |
