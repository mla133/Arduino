//  Parallax 27977-RT Serial LCD, backlit with Speaker demo
//  Hook pin to +5v, GND, Rx pin to D6

const int ledPin = 13;
const int TxPin = 6;
int incomingByte;  //variable to read incoming serial data

#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(255, TxPin);

void setup() {
  
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
    
  pinMode(TxPin, OUTPUT);
  digitalWrite(TxPin, HIGH);
  
  mySerial.begin(9600);
  delay(100);
  mySerial.write(12);                 // Clear             
  mySerial.write(17);                 // Turn backlight on
  delay(5);                           // Required delay
  mySerial.print("RX Testing...");  // First line (will wrap if needed)
  mySerial.write(13);                 // Form feed
  mySerial.print("for 27977-RT LCD");   // Second line
//  mySerial.write(212);                // Quarter note
//  mySerial.write(220);                // A tone
  delay(3000);                        // Wait 3 seconds
  //mySerial.write(18);                 // Turn backlight off
}

void loop() {

}

