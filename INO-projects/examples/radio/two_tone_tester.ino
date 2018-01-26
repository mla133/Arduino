//Analog Input Exercise and practical Two Tone Tester
// or how to drive your Mother-in-law crazy.
//The concept here is that two tones are separately generated
// but the timing is such that if the rate is fast enough they
//will appear as two simultaneous tones suitable for testing
// SSB transceivers. Typically the output of the tone generator
// is not a pure sign wave so some smoothing via an RC filter is
//required. Two tones one of 880 and the other of 2217 Hz were
//selected from a table of tones available on the Arduino website.
//This sketch has two functions: 1) to generate the tones and 2)
//to key the transmitter for the testing.
//Connect a 10K resistor to Pin 13 and feed the base of a 2N3904
//that has the collector connected to +12V through a reed relay
// and the emitter tied to ground. The relay contacts are connected
// to the transmitter PTT switch and the audio output after filtering
// is fed into the microphone input. Keying is off of Pin 13 and
// the signal output off of Pin 10 through a 10K resistor
// For adjusting the tone generation rate the anlog input A0 is used.
// A 10K pot is connected with one end to the +5VDC on the Arduino
// and the other to ground. The center wiper goes to the A0 port.
// A 10K ohm resistor is connected to digital pin 10 and that goes
// to the RC filter and then on to the microphone input. N6QW 12/2013

#include <toneAC.h>
#define Tone tone1
#define NOTE_A5  880
#define NOTE_CS7 2217

const int LED = 13; // turn on the on board LED
int val = A0;

void setup() {
  
  pinMode(LED, OUTPUT); // Pin 13 as well as the on-board LED will be hot
  
}

void loop() {
  
  val = analogRead(A0); // read the sensor value on pin A0
  
  digitalWrite(LED,HIGH);
  delay(10);
  tone(10,NOTE_A5);
  delay(val/4);
  tone(10,NOTE_CS7);
  delay(val/4);
  noTone(10);
  delay(100);
  digitalWrite(LED, LOW);
 
}
