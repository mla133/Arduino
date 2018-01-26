 // Keyer circuit for sending with a paddle. KK6FUT & N6QW 12/2013
// For use with an Arduino Uno R3. This software is for the Part II CW Sender

#include <toneAC.h>
#define NOTE_F5 698

const int LED = 13;
const int relay = 11;
const int tonepin = 10; // Speaker on PIN 10
const int dotpin = 9; // dot key into PIN 9
const int dashpin = 8; // dash ley into pin 8
const int analogPin1 = A0;
const int analogPin2 = A1;

long wpm=15;
long z = 1200/wpm; // convert wpm to milliseconds
long dashdelay = 3*z; // length of a dash
long dotdelay = z; // length of a dot
long dddelay = z; // length of pauses between dots and dashes

int dotstate = LOW;
int dashstate = LOW;
//int oldstate;

void setup()
{
pinMode(LED, OUTPUT);
pinMode(tonepin, OUTPUT);
pinMode(relay, OUTPUT); // keying the transmitter with this pin
pinMode(dotpin, INPUT);
pinMode(dashpin, INPUT);
digitalWrite(dotpin, HIGH); // turn on pullup resistors
digitalWrite(dashpin, HIGH);
Serial.begin(19200);

}

void senddash() {
digitalWrite(LED, HIGH);
digitalWrite(relay, HIGH);
tone(10,NOTE_F5);
delay(dashdelay);
noTone(10);
digitalWrite(LED, LOW);
digitalWrite(relay, LOW);
delay(dddelay);
}

void senddot() {
digitalWrite(LED, HIGH);
digitalWrite(relay, HIGH);
tone(10,NOTE_F5);
delay(dotdelay);
noTone(10);
digitalWrite(LED, LOW);
digitalWrite(relay, LOW);
delay(dddelay);
}


void loop()
{

// first check analog inputs for changes
int dotval = analogRead(analogPin1); // read the input pin for dot length
dotdelay = ((dotval * 250L) / 1024) + 30; // scale our input to be between 30 and 280 milliseconds of delay

int delayval = analogRead(analogPin2); // read the input pin for delay between dots/dashes
dddelay = ((delayval * 250L) / 1024) + 30; // scale our input to be between 30 and 280 milliseconds of delay
Serial.print("dotdelay is"); Serial.print(dotdelay); Serial.print("\r\n");

// now check if we need to send a dot, dash, or nothing
dotstate = digitalRead(dotpin);
dashstate = digitalRead(dashpin);
if((dotstate == LOW) &&(dashstate == HIGH)){ // dot key pressed send a dot
senddot();
}else if((dashstate == LOW)&&(dotstate==HIGH)) {
senddash();
}else{
noTone(tonepin);
delay(dddelay);
}

}

