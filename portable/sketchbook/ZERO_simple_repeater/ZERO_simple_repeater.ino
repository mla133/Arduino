// UART bridge for data exchange between
// RigExpert AA-30 ZERO antenna & cable analyzer and Arduino Uno
//
// Receives from the Arduino, sends to AA-30 ZERO.
// Receives from AA-30 ZERO, sends to the Arduino.
//
// 26 June 2017, Rig Expert Ukraine Ltd.
//
#include "SoftwareSerial.h"
#define RX0_Pin  0
#define TX0_Pin  1
#define RX1_Pin  4
#define TX1_Pin  7


#define HW_SERIAL

#ifndef HW_SERIAL
  SoftwareSerial ZERO(RX1_Pin, TX1_Pin); // RX, TX
#endif

void setup() {
  #ifdef HW_SERIAL
    pinMode(RX0_Pin, INPUT);
    pinMode(TX0_Pin, OUTPUT);
    
    pinMode(RX1_Pin, INPUT);
    pinMode(TX1_Pin, OUTPUT);
  #else
    ZERO.begin(38400); // init AA side UART
    ZERO.flush() 
    Serial.begin(38400); // init PC side UART
    Serial.flush();
  #endif
}

void loop() {
  #ifdef HW_SERIAL
    //digitalWrite(TX0_Pin, digitalRead(RX1_Pin));
    //digitalWrite(TX1_Pin, digitalRead(RX0_Pin));
    if (PIND & (1 << 4)) PORTD |= (1 << 1); else PORTD &= ~(1 << 1);
    if (PIND & (1 << 0)) PORTD |= (1 << 7); else PORTD &= ~(1 << 7);
    
  #else  
    if (ZERO.available()) Serial.write(ZERO.read()); // data stream from AA to PC
    if (Serial.available()) ZERO.write(Serial.read()); // data stream from PC to AA
  #endif  
}
