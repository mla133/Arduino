// mini_tremolo.pde
// minimalisitic tremelo program
// http://wiki.openmusiclabs.com/wiki/PWMDAC

#include "MiniArDSP.h"

#define PWM_FREQ 0x00FF // pwm frequency - see table
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // number of pwms, either 1 or 2

// create sinewave lookup table
// PROGMEM stores the values in the program memory
// it is automatically included with MiniArDSP.h
const PROGMEM int sinewave[]  = {
  // this file is stored in MiniArDSP and is a 1024 value
  // sinewave lookup table of signed 16bit integers
  // you can replace it with your own waveform if you like
  #include <sinetable.inc>
};
unsigned int location; // lookup table value location

byte rate = 5; // tremolo rate
byte rate_counter; // modifiable version of rate
unsigned int amplitude; // current tremolo amplitude
unsigned int button;

void setup() {
  // setup button pins
  PORTD |= 0x0c; // turn on pullups for pins 2 and 3
  
  // setup ADC
  ADMUX = 0x60; // left adjust, adc0, internal vcc
  ADCSRA = 0xe5; // turn on adc, ck/32, auto trigger
  ADCSRB =0x07; // t1 capture for trigger
  DIDR0 = 0x01; // turn off digital inputs for adc0
  
  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); // 
  TCCR1B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK1 = 0x20; // interrupt on capture interrupt
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  
  TIMSK0 = 0; // turn of t0 - no delay() or millis()
  sei(); // turn on interrupts - not really necessary with arduino
}

void loop() {
  while(1); // gets rid of jitter
  // nothing happens up here.  if you want to put code up here
  // get rid of the ISR_NAKED and the reti(); below
}

ISR(TIMER1_CAPT_vect, ISR_NAKED) { // ISR_NAKED is used to save
// clock cycles, but prohibits code in the loop() section.
  
  // get ADC data
  byte temp1 = ADCL; // you need to fetch the low byte first
  byte temp2 = ADCH; // yes it needs to be done this way
  int input = ((temp2 << 8) | temp1) + 0x8000; // make a signed 16b value
  
  button--; // check buttons every so often
  if (button == 0) {
    byte temp3 = PIND & 0x0c; // get pin 2 and 3 values
    if (temp3 == 0) { // both buttons pressed
    } // do nothing
    else if (temp3 == 0x08) { // up button pressed
      if (rate == 1); // do nothing
      else rate--; // make tremolo faster
    }
    else if (temp3 == 0x04) { // down button pressed
      if (rate == 255); // do nothing
      else rate++; // make tremolo slower
    }
    button = 0x0400; // reset counter
  }

  rate_counter--; // decrement our counter
  if (rate_counter == 0) {
    // create a variable frequency and amplitude sinewave
    // fetch a sample from the lookup table
    amplitude = pgm_read_word_near(sinewave + location) + 0x8000;
    // the + 0x8000 turns the signed value to unsigned
    location++;
    // if weve gone over the table boundary -> loop back
    // around to the other side.
    location &= 0x03ff; // fast way of doing rollover for 2^n numbers
    rate_counter = rate; // reset rate counter
  }
  
 
  // multiply our input by the sinewave value
  // note the use of the special multiply macro
  // this is faster than the normal * method
  // this is a signed * unsigned, returning the high 16 bits
  int output; // define variable before using it
  MultiSU16X16toH16(output, input, amplitude);
  
  // output data
  OCR1AL = (output + 0x8000) >> 8; // convert to unsigned
  // and output the top byte
  OCR1BL = output; // output the bottom byte
  reti(); // return from interrupt - required because of ISR_NAKED
}
