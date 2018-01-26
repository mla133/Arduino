/*

 N6QW / KK6FUT Antenna Analyzer Code
 
 Code for the LCD version of the N6QW / KK6FUT Antenna Analyzer. This antenna
 analyzer code provides a simple, LCD and rotary encoder driven antenna analyzer
 providing details similar to the MFJ-259 series of antenna analyzers.
 
 Inputs: Antenna, rotary encoder
 Outputs: LCD display
 
 The antenna analyzer uses a modified VSWR bridge design for Arduino
 based on the antenna analyzer project de K6BEZ.
 
 VFO Source code is based on thethe Kanga_VFO_2 source code 
 for the AD9850 DDS signal generator de m0xpd.
  
 Also uses a a (16*2) LCD Display on the I2C Interface. (TwRobot and Adafruit options).
 
 */

#include<stdlib.h>
#include <Wire.h>
//#include <LiquidCrystal.h> //Adafruit
#include <LiquidCrystal_I2C.h> //TwRobot

/*=====================================================
 Frequency defaults... Borrowed from M0XPD's Kanga VFO code.
 
 The system is set up to use the 
 136kHz, 160m, 80m, 60m, 40m, 30m, 20m, 17m, 15m, 12m 
 and 10m amateur bands 
 
 You can edit the band edges and the default "start" 
 frequency in any band by changing the following lines... */

double BandBases[]={
  135700, 1810000, 3500000, 5258500, 7000000, 10100000, 14000000, 18068000, 21000000, 24890000, 28000000};
double BandTops[]={
  137800, 2000000, 4000000, 5406500, 7300000, 10150000, 14350000, 18168000, 21450000, 24990000, 29700000}; //the continuous changed to 35 MHz
double BandCWFreqs[]={
  135700, 1836000, 3560000, 5258500, 7030000, 10106000, 14200000, 18086000, 21060000, 24906000, 28060000};  //the continuous starts at 5.0 MHz and su at 14.2 MHz

/*======================================================================
 Defining CONSTANTS...
 */
//======================================
// HARDWARE: Arduino I/O Pin allocations..
// set pin numbers:
// AD9850 Module....
const int W_CLK = 2;
const int FQ_UD = 3;
const int DATAPIN = 4;
const int RESET = 5;
// Rotary Encoder...
const int RotEncAPin = 11;
const int RotEncBPin = 12;
const int RotEncSwPin = A3;
// SWR redings
const int SWRA = A0;
const int SWRB = A1;

// Display...
// the display uses the I2C connection, 
// which uses 
// A4 for the Clock and
// A5 for the Data
//======================================
// SOFTWARE: defining other constants...


//VSWR variables
double FWD=0;
double REV=0;
double vswr;
// end of Constants
//=================================================================

/*=================================================================
 Declare (and initialize) some VARIABLES...
 */
int bandindex = 6;
double freq = BandCWFreqs[bandindex];
int dfindex = 3;
// Declare and initialise some Rotary Encoder Variables...
boolean OldRotEncA = true;
boolean RotEncA = true;
boolean RotEncB = true;
boolean RotEncSw = true;

int loopcount = 0;
// end of Variables
//=================================================================


// Instantiate the LCD display...
//LiquidCrystal lcd(0x27); // Adafruit I2C 
LiquidCrystal_I2C lcd(0x027, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address TwRobot

// (I2C address = 0)This is for the Adafruit backpack
// (this is for a Samsung 16*4 display -

void setup() {

  //  lcd.begin(16,4); //Adafruit I2C
  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight TwRobot

  // Set up I/O pins...  
 
  // Set up our inputs for measuring SWR
  pinMode(SWRA, INPUT);
  pinMode(SWRB, INPUT);

  // set up our inputs for the rotary encoder
  pinMode(RotEncAPin, INPUT);
  pinMode(RotEncBPin, INPUT);
  pinMode(RotEncSwPin, INPUT);
  // set up pull-up resistors on inputs...  
  digitalWrite(RotEncAPin,HIGH);
  digitalWrite(RotEncBPin,HIGH);
  digitalWrite(RotEncSwPin,HIGH); 

  // set up the pins for the DDS  
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATAPIN, OUTPUT);
  pinMode(RESET, OUTPUT);
 // start up the DDS... 
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD); 
  // start the oscillator...
  sendFrequency(freq);     

  // Print opening message to the LCD...
  Normal_Display();
 
  Calc_VSWR();
  LCD_Display_VSWR(vswr);

}

void loop() {
 
  if (loopcount > 15000) {
   // occasionally update display/frequency - do this only every so often to reduce flickering on LCD
     Calc_VSWR();
     LCD_Display_VSWR(vswr);
     loopcount = 0;
  }
  loopcount = loopcount + 1;
  
  // Read the inputs...
  RotEncA = digitalRead(RotEncAPin);
  RotEncB = digitalRead(RotEncBPin); 
  RotEncSw = digitalRead(RotEncSwPin);

  // check the rotary encoder values
  if ((RotEncA == HIGH)&&(OldRotEncA == LOW)){
    // adjust frequency
    if (RotEncB == LOW) {
      freq=constrain(freq+10000,BandBases[bandindex],BandTops[bandindex]);
    } else {
      freq=constrain(freq-10000,BandBases[bandindex],BandTops[bandindex]);
    }
    OldRotEncA=RotEncA;  // store rotary encoder position for next go around
 
    // Now, update the LCD with frequency and recalculate VSWR
    LCD_Display_Freq(freq); // push the frequency to LCD display
    sendFrequency(freq);  // set the DDS to the new frequency  
    delay(400); // let the frequency/voltages settle out after changing the frequency
    Calc_VSWR(); // routine to calculate VSWR
    LCD_Display_VSWR(vswr); // push the values to the LCD output

  }

  if (RotEncSw==LOW){
    // if user clicks rotary encoder, toggle band to next settingfs
    bandindex = (bandindex+1)%11;
f    freq=constrain(freq,BandBases[bandindex],BandTops[bandindex]);
    LCD_Display_Freq(freq);
    sendFrequency(freq);
    delay(400);
  }    
  OldRotEncA=RotEncA; 
  // End of loop()
}


//==============================================================
// SUBROUTINES...

// Calculation of VSWR using two voltage levels
void Calc_VSWR() { 
  // Read the forawrd and reverse voltages
  REV = analogRead(SWRA);
  FWD = analogRead(SWRB);
  if(REV>=FWD){
    // To avoid a divide by zero or negative VSWR then set to max 999
    vswr = 999;
  }
  else{
    // Calculate VSWR
    vswr = (FWD+REV)/(FWD-REV);
  }

}

// subroutine to display the frequency...
void LCD_Display_Freq(double frequency) {  
  lcd.setCursor(0, 0); //was 17
  if (frequency<10000000){
    lcd.print(" ");
  }  
  lcd.print(frequency/1e6,4);  
  lcd.print(" MHz");
  // establish the cursor position
  int c_position=25-dfindex;
  lcd.setCursor(c_position, 0);
  //lcd.blink();   
}  

void LCD_Display_VSWR(double swr) {  
  lcd.setCursor(0, 2); //was 17
  lcd.print("SWR ");
  lcd.print(swr);  

}  

// subroutine to set up the normal display...
void Normal_Display(){
  LCD_Display_Freq(freq);
  LCD_Display_VSWR(vswr);
}


// Subroutine to generate a positive pulse on 'pin'...
void pulseHigh(int pin) {
  digitalWrite(pin, HIGH); 
  digitalWrite(pin, LOW); 
}

// calculate and send frequency code to DDS Module...
void sendFrequency(double frequency) {
  int32_t freq = (frequency) * 4294967295/125000000;
  for (int b=0; b<4; b++, freq>>=8) {
    shiftOut(DATAPIN, W_CLK, LSBFIRST, freq & 0xFF);
  } 
  shiftOut(DATAPIN, W_CLK, LSBFIRST, 0x00);  
  pulseHigh(FQ_UD); 
}








