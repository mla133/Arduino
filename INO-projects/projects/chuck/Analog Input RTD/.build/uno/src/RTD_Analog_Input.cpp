#include <Arduino.h>

void setup();
void loop();
#line 1 "src/RTD_Analog_Input.ino"
//By: Chuck Kozik

/*Analog Input
 Read Analog Input A8*/

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used

#define RTD_INPUT A8
unsigned int adCountsArray[5] = {0,0,0,0,0};
unsigned int adIndex = 0;
unsigned long adCountsSum = 0;
unsigned long currentMicros;
unsigned long waitTime;

void setup() 
{
  Serial.begin(9600);
  Serial.println("Analog Input: RTD @ A8");
   //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application 
  TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
  analogReference(EXTERNAL);
  currentMicros = micros();
  waitTime = currentMicros + 1000000;//1 second
 
}


void loop() 
{
  int i;
  int input;
  int rtd_ad_counts;
  int avg_rtd_ad_counts;
  double resistance;
  double temp;
  const double A = 3.9083e-3;
  const double B = -5.775e-7;
  const double probe_ohms = 100;

  currentMicros = micros();
  while((long)(currentMicros - waitTime) < 0)
    currentMicros = micros();
  //start new timeout period
  waitTime = currentMicros + 1000000;//1 second 
  //get the ad counts 
  rtd_ad_counts = analogRead(RTD_INPUT);
  //subtract oldest
  adCountsSum -= adCountsArray[adIndex];
  //add newest
  adCountsSum += rtd_ad_counts;
  //maintain newest
  adCountsArray[adIndex] = rtd_ad_counts;
  //adjust index
  adIndex++;
  if(adIndex >= 5)
    adIndex = 0;
  avg_rtd_ad_counts = adCountsSum/5;  
  //Serial.println(rtd_ad_counts);
  //span = 55.0 ohms, 1024 for 10 bit A/D converter, 100 ohms minimum
  resistance = (((avg_rtd_ad_counts * 55.0) / 1024.0) + 100.0);
  temp = (-A + sqrt(A*A - 4*B*(1 - resistance/probe_ohms))) / (2*B);
  temp = (1.8*temp) + 32.0;     /* convert to fahrenheit */
  Serial.print("Ohms = ");
  Serial.println(resistance);
  Serial.print("Temp In F = ");
  Serial.println(temp);
}  
  

  

    


