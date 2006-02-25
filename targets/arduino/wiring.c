/*
  wiring.c - Wiring API Partial Implementation
  Part of Arduino - http://arduino.berlios.de/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id$
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/delay.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// from Pascal's avrlib
#include "global.h"
//#include "a2d.h"
#include "timer.h"
#include "uart.h"

// timer.h #defines delay to be delay_us, we need to undefine
// it so our delay can be in milliseconds.
#undef delay

#include "wiring.h"

// Get the hardware port of the given virtual pin number.  This comes from
// the pins_*.c file for the active board configuration.
int digitalPinToPort(int pin)
{
	return digital_pin_to_port[pin].port;
}

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
int digitalPinToBit(int pin)
{
	return digital_pin_to_port[pin].bit;
}

int analogOutPinToPort(int pin)
{
	return analog_out_pin_to_port[pin].port;
}

int analogOutPinToBit(int pin)
{
	return analog_out_pin_to_port[pin].bit;
}

int analogInPinToBit(int pin)
{
	return analog_in_pin_to_port[pin].bit;
}

void timer2PWMOn()
{
	// configure timer 2 for normal (non-inverting) pwm operation
	// this attaches the timer to the pwm pin
	sbi(TCCR2, COM21);
	cbi(TCCR2, COM20);
}

void timer2PWMOff()
{
	// disconnect the timer from the pwm pin
	cbi(TCCR2, COM21);
	cbi(TCCR2, COM20);
}

void timer2PWMSet(unsigned char val)
{
	OCR2 = val;
}

void pinMode(int pin, int mode)
{
	if (digitalPinToPort(pin) != NOT_A_PIN) {
		if (mode == INPUT)
			cbi(_SFR_IO8(port_to_mode[digitalPinToPort(pin)]),
				digitalPinToBit(pin));
		else
			sbi(_SFR_IO8(port_to_mode[digitalPinToPort(pin)]),
				digitalPinToBit(pin));
	}
}

void digitalWrite(int pin, int val)
{
	if (digitalPinToPort(pin) != NOT_A_PIN) {
		// If the pin that support PWM output, we need to turn it off
		// before doing a digital write.

		if (analogOutPinToBit(pin) == 1)
			timer1PWMAOff();

		if (analogOutPinToBit(pin) == 2)
			timer1PWMBOff();

		if (analogOutPinToBit(pin) == 3)
			timer2PWMOff();

		if (val == LOW)
			cbi(_SFR_IO8(port_to_output[digitalPinToPort(pin)]),
				digitalPinToBit(pin));
		else
			sbi(_SFR_IO8(port_to_output[digitalPinToPort(pin)]),
				digitalPinToBit(pin));
	}
}

int digitalRead(int pin)
{
	if (digitalPinToPort(pin) != NOT_A_PIN) {
		// If the pin that support PWM output, we need to turn it off
		// before getting a digital reading.

		if (analogOutPinToBit(pin) == 1)
			timer1PWMAOff();

		if (analogOutPinToBit(pin) == 2)
			timer1PWMBOff();

		if (analogOutPinToBit(pin) == 3)
			timer2PWMOff();

		return (_SFR_IO8(port_to_input[digitalPinToPort(pin)]) >>
			digitalPinToBit(pin)) & 0x01;
	}
	
	return LOW;
}

int analogRead(int pin)
{
	unsigned int low, high, ch = analogInPinToBit(pin);

	// the low 4 bits of ADMUX select the ADC channel
	ADMUX = (ADMUX & (unsigned int) 0xf0) | (ch & (unsigned int) 0x0f);

	// without a delay, we seem to read from the wrong channel
	delay(1);

	// start the conversion
	sbi(ADCSRA, ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low = ADCL;
	high = ADCH;

	// combine the two bytes
	return (high << 8) | low;
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(int pin, int val)
{
	// We need to make sure the PWM output is enabled for those pins
	// that support it, as we turn it off when digitally reading or
	// writing with them.  Also, make sure the pin is in output mode
	// for consistenty with Wiring, which doesn't require a pinMode
	// call for the analog output pins.
	if (analogOutPinToBit(pin) == 1) {
		pinMode(pin, OUTPUT);
		timer1PWMAOn();
		timer1PWMASet(val);
	} else if (analogOutPinToBit(pin) == 2) {
		pinMode(pin, OUTPUT);
		timer1PWMBOn();
		timer1PWMBSet(val);
	} else if (analogOutPinToBit(pin) == 3) {
		pinMode(pin, OUTPUT);
		timer2PWMOn();
		timer2PWMSet(val);
	} else if (val < 128)
		digitalWrite(pin, LOW);
	else
		digitalWrite(pin, HIGH);
}

void beginSerial(long baud)
{
	uartInit();
	uartSetBaudRate(baud);
}

void serialWrite(unsigned char c)
{
	uartSendByte(c);
}

int serialAvailable()
{
	return uartGetRxBuffer()->datalength;
}

int serialRead()
{
	return uartGetByte();
}

void printMode(int mode)
{
	// do nothing, we only support serial printing, not lcd.
}

void printByte(unsigned char c)
{
	serialWrite(c);
}

void printNewline()
{
	printByte('\n');
}

void printString(char *s)
{
	while (*s)
		printByte(*s++);
}

void printIntegerInBase(unsigned long n, unsigned long base)
{ 
	unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
	unsigned long i = 0;

	if (n == 0) {
		printByte('0');
		return;
	} 

	while (n > 0) {
		buf[i++] = n % base;
		n /= base;
	}

	for (; i > 0; i--)
		printByte(buf[i - 1] < 10 ?
			'0' + buf[i - 1] :
			'A' + buf[i - 1] - 10);
}

void printInteger(long n)
{
	if (n < 0) {
		printByte('-');
		n = -n;
	}

	printIntegerInBase(n, 10);
}

void printHex(unsigned long n)
{
	printIntegerInBase(n, 16);
}

void printOctal(unsigned long n)
{
	printIntegerInBase(n, 8);
}

void printBinary(unsigned long n)
{
	printIntegerInBase(n, 2);
}

/* Including print() adds approximately 1500 bytes to the binary size,
 * so we replace it with the smaller and less-confusing printString(),
 * printInteger(), etc.
void print(const char *format, ...)
{
	char buf[256];
	va_list ap;
	
	va_start(ap, format);
	vsnprintf(buf, 256, format, ap);
	va_end(ap);
	
	printString(buf);
}
*/

unsigned long millis()
{
	// timer 0 increments every timer0GetPrescaler() cycles, and
	// overflows when it reaches 256.  we calculate the total
	// number of clock cycles, then divide by the number of clock
	// cycles per millisecond.
	return (unsigned long) timer0GetOverflowCount() *
		timer0GetPrescaler() * 2UL / (F_CPU / 128000UL);
}

void delay(unsigned long ms)
{
	timerPause(ms);
}

/* Delay for the given number of microseconds.  Assumes a 16 MHz clock. 
 * Disables interrupts, which will disrupt the millis() function if used
 * too frequently. */
void delayMicroseconds(unsigned int us)
{
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;

	// disable interrupts, otherwise the timer 0 overflow interrupt that
	// tracks milliseconds will make us delay longer than we want.
	cli();

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);

	// reenable interrupts.
	sei();
}

int main(void)
{
	// this needs to be called before setup() or some functions won't
        // work there
	sei();
	
	// timer 0 is used for millis() and delay()
	timer0Init();

	// timers 1 & 2 are used for the hardware pwm
	timer1Init();
	//timer1SetPrescaler(TIMER_CLK_DIV1);
	timer1PWMInit(8);
	
	timer2Init();

	// configure timer 2 for phase correct pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle
	cbi(TCCR2, WGM21);
	sbi(TCCR2, WGM20);

	// set a2d reference to AVCC (5 volts)
	cbi(ADMUX, REFS1);
	sbi(ADMUX, REFS0);

	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

	// enable a2d conversions
	sbi(ADCSRA, ADEN);
	
	setup();
	
	for (;;)
		loop();
		
	return 0;
}
