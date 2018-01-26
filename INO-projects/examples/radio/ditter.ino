// Analog Input Exercise and practical "ditter".
// This sketch is not only to test the use of the analog input
// function but for use as a practical CW transmitter tester for
// alignment, SWR checks and "being kind to your finals". This sketch
// provides a variable duration pulse that is dependent upon the setting of a 10K
// pot. In addition to the pulse being developed, the on-board LED flashes and
// a the pulse rate can be heard in a speaker. The connections to the
// Arduino consist of the following. A 10K pot with one end connected to the
// + 5VDC port and the other end connected to the ground connection. The center
// wiper is connected to the A0 analog input port. This pot sets the rep rate.
// The on-board LED which is also tied to Pin 13 gives a visual indication as well 
// as providing a means of keying the transmitter.
// Note the val/4 is so that the anlog value on pin A0 are in the range of 0- 5 VDC
// but are read as 0-1023 so to get the rate less than 1023 we divide by 4 or 225 max.
// For connections to the transmitter a 10K ohm resistor is connected
// to pin 13 with the other end feeding the base of a 2N3904. The collector is fed with 
// +12 VDC through a reed relay and the emitter is grounded. As the pulses are generated 
// the transmitter is keyed and some delays are included to account for turning on the
// tones as well as turning on the transmitter. Keep in mind there is an order and 
// sequence with which events need to occur.The 8 ohm speaker is connected through a 
// a 100 Ohm resistor. DON"T SKIP" this as there is a possiblity of damage to the speaker 
// or Arduino because of the square wave pulse and collapsing magnetic field of the speaker.
// Bottom line pulsing a CW transmitter is far better than holding the key down for long periods
// of time. N6QW 12/2013


#include <toneAC.h>
#define Tone tone1
#define NOTE_F5  698

const int LED = 13; // turn on the on board LED
int val = A0;

void setup() {
  
  pinMode(LED, OUTPUT); // Pin 13 as well as the on-board LED will flash based on the analog input
  
}

void loop() {
  
  val = analogRead(A0); // read the sensor value on pin A0
  digitalWrite(LED, HIGH);
  delay(10);
  tone(10,NOTE_F5);
  delay(val/4);
  noTone(10);
  delay(val/10);
  digitalWrite(LED, LOW);
  delay(50);
}
