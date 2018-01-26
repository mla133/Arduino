// Analog Input Exercise and practical "Detector"
// This sketch has several goals one of which is to introduce the "if else" concept
// and a secondary goal is to show how the Arduino can "detect" specific values of input
// voltage and to output a audible signal and the third is to display what is detected 
// using your computer screen. 
// Essentially a voltage bewteen 0 and 5 VDC is impressed on the A0 analog pin. For voltages
// in between these end points the analog read function will output a value bewteen 0 and 1023.
// If the value is 512 that correcsponds to a voltage of 2.5 volts. You can do the math and see 
// how you can read a value and quickly determine a voltage level. This detector will ouput specific
// tones based on the voltage being applied. The lower voltage produce lower tones and the higher 
// voltage produce higher tones. How cool is that? One application would be to have a liquid level control 
// where as the water level rises a paddle arm would move a potentiometer. When  the pot hits a pre-determined 
// level a warning tone would be generated. How about a small wind generator where when the wind speed reaches 
// a certain level a voltage is generated that would signal a warning "It time to drop the Tower!" N6QW 12/2013

#include <toneAC.h>
#define Tone tone1
#define NOTE_A3  220
#define NOTE_E5 659
#define NOTE_A5 880
#define NOTE_C6 1047
#define NOTE_G6 1568
#define NOTE_C7 2093

const int LED = 13; // turn on the on board LED
int val = A0;

void setup() {
  
  pinMode(LED, OUTPUT); // Pin 13 as well as the on-board LED will flash based on the analog input
  Serial.begin(19200);
}

void loop() {
  
  val = analogRead(A0); // read the sensor value on pin A0
  digitalWrite(LED, HIGH);
  delay(10);
 if (analogRead(A0)== 0){
   noTone(10);
   delay(100);
 }else if(analogRead(A0) == 100){
  tone(10,NOTE_A3);
  delay(val/4);
  noTone(10);
  delay(val/10);
  Serial.println(val);
  }else if(analogRead(A0) == 200){
    tone(10,NOTE_E5);
    delay(val/4);
    noTone(10);
    delay(val/10);
    Serial.println(val);
  }else if(analogRead(A0) == 300) {
    tone(10,NOTE_A5);
    delay(val/4);
    noTone(10);
    delay(val/10);
    Serial.println(val);
  }else if(analogRead(A0) == 400){
    tone(10,NOTE_C6);
    delay(val/4);
    noTone(10);
    delay(val/10);
    Serial.println(val);
  }else if(analogRead(A0) == 500){
     tone(10,NOTE_G6);
     delay(val/4);
     noTone(10);
     delay(val/10);
     Serial.println(val);
  }else if(analogRead(A0) == 600){
    tone(10,NOTE_C7);
    delay(val/4);
    noTone(10);
    delay(val/10);
    digitalWrite(LED, LOW);
    Serial.println(val);
   delay(50);
}
}
