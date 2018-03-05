/*
Simple controller for AD9850 modules
Uses serial protocol (refer to AD9850 datasheet what pins need to be pulled up/down
Uses library by Elecfreaks http://elecfreaks.com/store/download/datasheet/breakout/DDS/EF_AD9850_library.zip
(for use with chipkit delete that unnecessary line "#include <avr/pgmspace.h>" from .h file)
Fix the pin numbers used to communicate with module
Serial comms - 9600, NewLine (0x0A) denotes end of input,
f 10000 - set frequency
s 20 20000 10 1000 - sweep from frequency to frequency using this step size in that many milliseconds
o 10 10000 5 3000 - oscillating sweep from frequency to frequency and back using this step size in that many ms
NB! no error checking in input, code relies on intelligence of user
*/
#include <SoftwareSerial.h>

#define MAXBUF 40
#define W_CLK 8   // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9   // Pin 9 - connect to freq update pin (FQ)
#define DATA 10   // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11  // Pin 11 - connect to reset pin (RST) 
#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }
#define TX_PIN 3
#define LED_PIN 13
#define ANALOG_VOLTAGE_PIN 1

double freqValue = 0;
double freqStart = 0;
double freqEnd = 0;
double freqStep = 0;
int sweepDelay = 0;
char freqStr[MAXBUF];
int bufPos = 0;
char byteIn = 0;
int mode = 0;

void setup()
{
    pinMode(FQ_UD, OUTPUT);
    pinMode(W_CLK, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(RESET, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("Serial comms - 9600, NewLine (0x0A) denotes end of input");
  Serial.println("f 10000 - set frequency");
  Serial.println("s 20 20000 10 1000 - sweep from frequency to frequency using this step size in that many milliseconds");
  Serial.println("o 10 10000 5 3000 - oscillating sweep from frequency to frequency and back using this step size in that many ms");

  memset(freqStr,0,sizeof(freqStr));
}

void loop()
{
  if (mode == 1 || mode == 2)
  {
    delay(sweepDelay);
    if (((freqStep > 0.0) && (freqValue + freqStep <= max(freqStart,freqEnd))) || ((freqStep < 0.0) && (freqValue + freqStep >= min(freqStart,freqEnd))))
      freqValue += freqStep;
    else
      if (mode == 1)
        freqValue = freqStart;
      else
      {
        freqStep *= -1;
        freqValue += freqStep;
      }
    //AD9850.wr_serial(0,freqValue);
    Serial.print("Sending: ");
    Serial.print(freqValue);
    Serial.println(" kHz");
    sendFrequency(freqValue);
  }
  while (Serial.available())
  {
    byteIn = Serial.read();
    if (bufPos < sizeof(freqStr))
      freqStr[bufPos++] = byteIn;
    else
      {
        bufPos = 0;
        byteIn = 0;
        memset(freqStr,0,sizeof(freqStr));
        Serial.println("Command too long. Ignored.");
      }
  }
  if ( (byteIn == 0x0a) || (byteIn == 0x0d) )
  {
    switch (freqStr[0])
    {
      case 'f':
        mode = 0;
        freqValue = strtod(freqStr+2,NULL);
        freqEnd = 0;
        freqStep = 0;
        sweepDelay = 0;
        Serial.print("Frequency ");
        Serial.println(freqValue);
        break;
      case 's':
      case 'o':
        char *fEnd1, *fEnd2;
        freqStart = abs(strtod(freqStr+2,&fEnd1));
        freqEnd = abs(strtod(fEnd1,&fEnd2));
        freqStep = abs(strtod(fEnd2,&fEnd1));
        if (freqStep == 0)
        {
          Serial.println("You gotta be kidding me, step can not be 0");
          break;
        }
        sweepDelay = abs(atoi(fEnd1));
        if (freqStr[0] == 's')
        {
          mode = 1;
          Serial.print("Sweep");
        }
        else
        {
          mode = 2;
          Serial.print("Oscillate sweep");
        }
        Serial.print(" start freq. ");
        Serial.print(freqStart);
        Serial.print(" end freq. ");
        Serial.print(freqEnd);
        Serial.print(" step ");
        Serial.print(freqStep);
        Serial.print(" time ");
        Serial.println(sweepDelay);
        sweepDelay /= abs(freqEnd - freqStart) / freqStep;
        if (mode == 2)
          sweepDelay /= 2;
        if (freqStart > freqEnd)
          freqStep *= -1;
        freqValue = freqStart;
        break;
      default:
        Serial.println("AI blown up - unknown command");
    }
    memset(freqStr,0,sizeof(freqStr));
    byteIn = 0;
    bufPos = 0;
    //AD9850.wr_serial(0,freqValue);
    Serial.print("Sending: ");
    Serial.print(freqValue);
    Serial.println(" kHz");
    sendFrequency(freqValue);
  }
}

// frequency calc from datasheet page 8 = <sys clock> * <frequency tuning word>/2^32
void sendFrequency(double frequency) 
{  
  int32_t freq = frequency * 4294967295/125000000;  // note 125 MHz clock on 9850.  You can make 'slight' tuning variations here by adjusting the clock frequency.
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
