//  Parallax 27977-RT Serial LCD demo
//  Hook pin to +5v, GND, Rx pin to D6

const int ledPin = 13;
const int TxPin = 6;
int incomingByte;  //variable to read incoming serial data
unsigned long timer = 0;
const int linelength = 16;
int reading, reading2, reading3 = 0;

#include <SoftwareSerial.h>
SoftwareSerial lcd = SoftwareSerial(255, TxPin);

void setup() {
  
        Serial.begin(9600);
        pinMode(TxPin, OUTPUT);
        digitalWrite(TxPin, HIGH);
        lcd.begin(9600);
        delay(100);
        lcd.write(12); // Clear
        lcd.write(17); // Turn backlight on
        delay(5); // Required delay
        lcd.print("Hello, world..."); // First line
        lcd.write(13); // Form feed
        lcd.print("73 DE KC3EYS"); // Second line
        delay(3000); // Wait 3 seconds
        //lcd.write(18); // Turn backlight off
        timer = millis();
}

void loop() 
{
        if(millis() - timer >=500UL)
        {
                lcd.write(128);

                reading = random(25, 75);  // Random number test
                reading2 = random(25, 75);  // Random number test
                reading3 = random(25, 75);  // Random number test
                lcd.write(12);
                delay(5);
                lcd.print("   SWR METER   ");
                lcd.write(13);  //Form Feed
                lcd.print("S:");
                lcd.print(reading);
                lcd.print(" F:");
                lcd.print(reading2);
                lcd.print(" R:");
                lcd.print(reading3);
                timer = millis();
        }

}

