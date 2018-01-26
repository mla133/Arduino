  
/*

 N6QW / KK6FUT Super Simple DDS
 
 This is a super simple DDS sketch to run a single band AD9850-based DDS.
 Kept super-super simple to make it easy for the non-software folks.
 
 Inputs: Rotary encoder
 Outputs: LCD display, DDS frequency

 Uses a a (16*2) LCD Display on the I2C Interface for TwRobot. 
 
 */

#include<stdlib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //TwRobot and others Adafruit uses a a different Library

const double bandStart = 7000000;  // start of 40m
const double bandEnd = 7300000; // end of 40m
const double bandInit = 7100000; // where to initially set the frequency

double freq = bandInit ;  // this is a variable (changes) - set it to the beginning of the band
double freqDelta = 10000; // how much to change the frequency by, clicking the rotary encoder will change this.

// Set pin numbers for ADS9850 N6QW in his version changed these to 4,5,6, 7 from 2,3,4,5 to mate with his test bed
const int W_CLK = 4;
const int FQ_UD = 5;
const int DATAPIN = 6;
const int RESET = 7;

// Set pins for ROTARY ENCODER - we are NOT using rotary encoder switch, so omit for clarity
const int RotEncAPin = 11;
const int RotEncBPin = 12;
const int RotEncSwPin = A3; //added for the selection of Frequency Delta Tuning rate

// Display library assumes use of A4 for clock, A5 for data. No code needed.

// Variables for Rotary Encoder handling
boolean OldRotEncA = true;
boolean RotEncA = true;
boolean RotEncB = true;
boolean RotEncSw = true;

// Instantiate the LCD display...
LiquidCrystal_I2C lcd(0x027,16,2);  // Set the LCD I2C address used by N6QW TwRobot uses a different string

void setup() {

  // Set up LCD
  //lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight TwRobot
  lcd.init(); // initialization sequence for the I2C backpack that was used by N6QW
  delay(200); // initialization sequence for the I2C backpack that was used by N6QW
  lcd.init();  // initialization sequence for the I2C backpack that was used by N6QW
  lcd.blink(); // initialization sequence for the I2C backpack that was used by N6QW
  lcd.backlight(); 
  lcd.setCursor(0,1); // This places a display on the LCD at turn on at the 2nd line
  lcd.print("   LBS PART I   "); //Any LCD message can be place here. Just make sure there are 16 spaces between the "" marks
  
  // Set up ROTARY ENCODER
  pinMode(RotEncAPin, INPUT);
  pinMode(RotEncBPin, INPUT);
  pinMode(RotEncSwPin, INPUT);
  // set up pull-up resistors on inputs...  
  digitalWrite(RotEncAPin,HIGH);
  digitalWrite(RotEncBPin,HIGH);
  digitalWrite(RotEncSwPin,HIGH); 
  
  // Set up DDS
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATAPIN, OUTPUT);
  pinMode(RESET, OUTPUT);
 // start up the DDS... 
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD); 
  // start the oscillator...
  send_frequency(freq);     
  display_frequency(freq);

}

void loop() {

  // Read the inputs...
  RotEncA = digitalRead(RotEncAPin);
  RotEncB = digitalRead(RotEncBPin); 
  RotEncSw = digitalRead(RotEncSwPin);

  // check the rotary encoder values
  if ((RotEncA == HIGH)&&(OldRotEncA == LOW)){
    // adjust frequency
    if (RotEncB == LOW) {
      freq=constrain(freq+freqDelta,bandStart,bandEnd);
    } else {
      freq=constrain(freq-freqDelta,bandStart,bandEnd);
    }
    OldRotEncA=RotEncA;  // store rotary encoder position for next go around
 
     // Now, update the LCD with frequency 
    display_frequency(freq); // push the frequency to LCD display
    send_frequency(freq);  // set the DDS to the new frequency  
    delay(60); // let the frequency/voltages settle out after changing the frequency
   }
  
  // check for the click of the rotary encoder 
  if (RotEncSw==LOW){
    // if user clicks rotary encoder, change the size of the increment
    // use an if then loop, but this could be more elegantly done with some shifting math
    if (freqDelta == 1000) {
       freqDelta = 100;
    } else if (freqDelta == 100) {
       freqDelta = 1000;
    } else { // this is either 100 or we goofed somewhere else, so set it back to the big change
       freqDelta = 1000;
    }
  }    
  
  OldRotEncA=RotEncA; 
  // End of loop()
}



// subroutine to display the frequency...
void display_frequency(double frequency) {  
  lcd.setCursor(0, 0); //was 17
  if (frequency<10000000){
    lcd.print(" ");
  }  
  lcd.print(frequency/1e6,4);  
  lcd.print(" MHz");
}  


// Subroutine to generate a positive pulse on 'pin'...
void pulseHigh(int pin) {
  digitalWrite(pin, HIGH); 
  digitalWrite(pin, LOW); 
}

// calculate and send frequency code to DDS Module...
void send_frequency(double frequency) {
  int32_t freq = (frequency) * 4294967295/125000000;
  for (int b=0; b<4; b++, freq>>=8) {
    shiftOut(DATAPIN, W_CLK, LSBFIRST, freq & 0xFF);
  } 
  shiftOut(DATAPIN, W_CLK, LSBFIRST, 0x00);  
  pulseHigh(FQ_UD); 
}








