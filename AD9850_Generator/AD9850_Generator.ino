/*  Rev 0.0 - Initial build of simple freq generator using AD9859 chip w/Uno */
#include <SoftwareSerial.h>

#define W_CLK 8   // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9   // Pin 9 - connect to freq update pin (FQ)
#define DATA 10   // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11  // Pin 11 - connect to reset pin (RST) 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }
#define LED_PIN 13
#define ANALOG_VOLTAGE_PIN 1

double rx = 7025;
double rx2;

const char *splash =
" _  __   _____   ____    ______  __    __   _____ \r\n"
"| |/ /  / ____| |___ \\  |  ____| \\ \\  / /  / ____|\r\n"
"| ' /  | |        __) | | |__     \\ \\/ /  | (___  \r\n"
"|  <   | |       |__ <  |  __|     \\  /    \\___ \\ \r\n"
"| . \\  | |____   ___) | | |____    |  |    ____) |\r\n"
"|_|\\_\\  \\_____| |____/  |______|   |__|   |_____/ \r\n";

void setup() {
  
  Serial.begin(9600);
  Serial.print(splash);
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT); 
  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);             // this pulse enables serial mode on the AD9850 - Datasheet page 12.
  Serial.println("AD9850 initialized...");
  Serial.println("Usage:  Enter desired frequency in kHz and hit <ENTER>");
}

void loop() 
{
    while (Serial.available() > 0) 
      rx = Serial.parseFloat();
      
    if (rx != rx2)
    {    
          Serial.print("Current Setting: ");
          Serial.print(rx);
          Serial.println(" kHZ");
          sendFrequency(rx);
          rx2 = rx;
    }

}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) 
{  
  int32_t freq = frequency * 4294967295/125000;  // note 125 MHz clock on 9850.  You can make 'slight' tuning variations here by adjusting the clock frequency.
  for (int b=0; b<4; b++, freq>>=8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}

// transfers a byte, a bit at a time, LSB first to the 9850 via serial DATA line
void tfr_byte(byte data)
{
  for (int i=0; i<8; i++, data>>=1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}
