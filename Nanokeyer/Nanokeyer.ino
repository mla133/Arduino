// Keyer for AVR
// Copyright 2011, 2012 Bud Tribble, N6SN
// License: http://www.opensource.org/licenses/mit-license

// UP     B0		D8
// DOWN   B1		D9
// QRS    B2		D10
// QRQ    B3		D11
// TX     B4		D12
// LED    B5		D13
// DIT    D4  [TIP]	D4
// DAH    D5  [RING]	D5
// TONE   D6		D6
// MODE	  D7		D7

// The PWM output (pin D6 on the Arduino Nano) goes through a simple
// low pass RC filter before it goes to the LM386 amplifier.

// The PWM frequency is 62.5KHz.  The 3dB corner of the filter is
// about 16KHz.  (R = 10K, C = .001uF) The default modulation
// frequency for the code signal is 600Hz (can be changed up or down
// with pushbuttons).

#include <Arduino.h>

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef USE_LCD
#include <lcd.h>
#else
#ifdef USE_SERIAL
#define lcd_init() Serial.begin(19200)
#define lcd_line1(x) Serial.println(x)
#define lcd_line2(x) Serial.println(x)
#else
#define lcd_init() 
#define lcd_line1(x) 
#define lcd_line2(x) 
#endif
#endif

#define UI_DELAY 80
#define STRAIGHT 0
#define PADDLE 1

void dash(uint8_t key_xmitter);

uint8_t mode;
uint8_t reverse_polarity;
uint8_t left_key_down, right_key_down, key_down;

volatile int wpm;
int dot_time;
char buf[64];

volatile uint8_t modulation;
volatile uint8_t rl_modulation = 0; // rate limited modulation;

volatile uint8_t timer1_busy = 0;

int frequency;
int8_t sine_table[256];
volatile uint16_t phase;
volatile uint16_t delta_phase;

ISR(TIMER0_OVF_vect)
{
     int value;

     if (rl_modulation < modulation) {
	  rl_modulation++;
     } else if (rl_modulation > modulation) {
	  rl_modulation--;
     }

     value = sine_table[phase >> 8];
     value = value * rl_modulation;      // [-(127*256)..(127*256)]
     value = value + (127*255);
     value = value >> 8;
     phase += delta_phase;

     OCR0A = value;
     return;
}

ISR(TIMER1_OVF_vect)
{
     modulation = 0;
     timer1_busy = 0;
}

void start_timer1(int msec)
{
     uint16_t count;
     TCCR1B = 0;			// Turn of timer1
     TIFR1 = TOV1;			// Make sure overflow bit is clear
     timer1_busy = 1;

     // timer will count up to 0xFFFF
     count = trunc(65536.0 - (msec * 1000.0)/16.0);
     cli();
     TCNT1 = count;
     sei();
     timer1_busy = 1;
     TCCR1B = 0x04;		// Turn on timer1
     // 16 usec ticks
}

void check_key()
{
     // D4 = Tip, D5 = Ring
     if (reverse_polarity) {
	  right_key_down = ((PIND & _BV(PORTD4)) == 0);
	  left_key_down = ((PIND & _BV(PORTD5)) == 0);
     } else {
	  left_key_down = ((PIND & _BV(PORTD4)) == 0);
	  right_key_down = ((PIND & _BV(PORTD5)) == 0);
     }
}

void space(uint8_t i)
{
     while (i > 0) {
	  start_timer1(dot_time);
	  while (timer1_busy != 0);
	  i--;
     }
}

void dot(uint8_t key_xmitter)
{
     uint8_t got_dash = 0;
     modulation = 255;
     if (key_xmitter) PORTB |= _BV(PORTB4);
     PORTB |= _BV(PORTB5);
     start_timer1(dot_time);
     while (timer1_busy != 0) {
	  // check for dash
	  check_key();
	  if (right_key_down) got_dash = 1;
     }
     modulation = 0;
     PORTB &= ~_BV(PORTB4);
     PORTB &= ~_BV(PORTB5);
     space(1);
     if (got_dash) dash(key_xmitter);
}

void dash(uint8_t key_xmitter)
{
  uint8_t got_dot = 0;
  modulation = 255;
  if (key_xmitter) PORTB |= _BV(PORTB4);
  PORTB |= _BV(PORTB5);
  start_timer1(dot_time * 3);
  while (timer1_busy != 0) {
    // check for dot
    check_key();
    if (left_key_down) got_dot = 1;
  }

  modulation = 0;
  PORTB &= ~_BV(PORTB4);
  PORTB &= ~_BV(PORTB5);
  space(1);
  if (got_dot) {
    dot(key_xmitter);
  }
}

void update_display()
{
  if (mode == STRAIGHT) {
    lcd_line1("Straight Key");
  } else {
    if (reverse_polarity)
      lcd_line1("Paddle Key (rev)");
    else lcd_line1("Paddle Key");
  }
  sprintf(buf, "%d Hz %d WPM", frequency, wpm);
  lcd_line2(buf);
}

int main( void )
{
  int i;

  // Initialize sine_table
  for (i = 0; i < 256; i++) {
    sine_table[i] = 127 * sin((2*M_PI*i)/256.0);
  }

  DDRB |= _BV(PORTB5); // LED output
  DDRD |= _BV(PORTD6); // PWM output
  DDRB |= _BV(PORTB4); // xmitter key output

  // Turn on pull-ups for inputs
  PORTD |= _BV(PORTD4) | _BV(PORTD5) |_BV(PORTD7);
  PORTB |= _BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2) | _BV(PORTB3) | _BV(PORTB4);

  TCCR0A = 0x83;
  TCCR0B = 0x01;
  OCR0A = 128;
  TIFR0 = 0x01;			// Make sure overflow bit is clear


  TIMSK0 = 0x01;
  TCCR1A = 0x00;
  TIFR1 = 0x01;			// Make sure overflow bit is clear


  TIMSK1 = 0x01;
  PORTB &= ~_BV(PORTB4);	// Make sure key is off

  lcd_init();
  if (!(PIND & _BV(PORTD7)))
       mode = PADDLE;
  else
       mode = STRAIGHT;

  frequency = 600;
  wpm = 15;
  dot_time = 1200/wpm;
  phase = 0;
  delta_phase = round((256.0*256.0*16.0*frequency)/1000000.0);  // 1/16us sample rate
  modulation = 0;
  rl_modulation = 0;
  timer1_busy = 0;

  // if key down on power up, reverse the dot dash polarity on paddle
  check_key();
  reverse_polarity = left_key_down || right_key_down;
  update_display();
  sei();

  dot(0); dot(0); dot(0); dash(0);

  while (1) {
    check_key();

    if (mode == STRAIGHT) {
      // STRAIGHT mode
      if (!(PIND & _BV(PORTD7))) {
	mode = PADDLE;
	update_display();
      }

      key_down = left_key_down || right_key_down;

      if (key_down) {
	modulation = 255;
	PORTB |= _BV(PORTB4); // key xmitter
	PORTB |= _BV(PORTB5); // LED
      } else {
	modulation = 0;
	PORTB &= ~_BV(PORTB4); // unkey xmitter
	PORTB &= ~_BV(PORTB5); // LED
      }
    } else {
      // PADDLE mode
      if (PIND & _BV(PORTD7)) {
	mode = STRAIGHT;
	update_display();
      }

      if (left_key_down) {
	dot(1);
      } else {
	modulation = 0;
      }

      if (right_key_down) {
	dash(1);
      } else {
	modulation = 0;
      }
    }

    if (((PINB & _BV(PORTB2)) == 0) || ((PINB & _BV(PORTB3)) == 0)) {
      // change wpm
      if (PINB & _BV(PORTB2)) wpm--; else wpm++;
      if (wpm >= 30) wpm = 30;
      if (wpm <= 5) wpm = 5;
      dot_time = 1200/wpm;
      update_display();
      dot(0);
    }

    if (((PINB & _BV(PORTB0)) == 0) || ((PINB & _BV(PORTB1)) == 0)) {
      // change frequency
      if (PINB & _BV(PORTB0)) frequency -= 10; else frequency += 10;
      if (frequency <= 300) frequency = 300;
      if (frequency >= 1200) frequency = 1200;
      delta_phase = round((256.0*256.0*16.0*frequency)/1000000.0); // 1/16uS sample rate
      update_display();
      dot(0);
    }
  }

  return 0;
}
