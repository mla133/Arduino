// Simple Function Generator that will shut off when it receives 
// an off/on pulse from digital I/O - Valve High / Valve Low
// 05/23/13:  Adding in low flow triggers and pulsing...
// 05/30/13:  Fixed trickle voltage trigger when TIOA is powered down
// 06/03/13:  Added table for set frequencies for low/high flow rates
// 06/04/13:  Added adjustable delay to valve shutoff to simulate slow valve closure
// 06/10/13:  Added 150 & 160 GPM values for Test # 23
// 06/14/13:  Added 100 GPM Values

/**************************************
*  High/Low Flow Values for K-factor = 60            
*                                                
*    GPM    HFlow(fast)  LFlow(slow) 
*    100      4825          49
*    150      3190          33
*    160      2985          31      
*    200      2355          26               
*    250      1860          25
*    300      1532          16
*    350      1295          14       
*    400      1119          12
*    450       980          11         
*    500       870          10                
*    550       781           9      
*    600       705           8       
*    650       643           7     
*
*  High/Low Flow Values for Genesis 2" meter
*    GPM    HFlow(fast)  LFlow(slow)
*    150      47            1529    <-  use delayMicroseconds()
***************************************/

const int highFlowValve = 3;
const int lowFlowValve = 4;
const int pulseOut = 8;
const int pulseInd = 13;
int valHigh = 0;
int valLow  = 0;
const int fast = 47;    // frequency value for High Flow rate (see table above)
const int slow = 1529;      // frequency value for Low Flow rate (see table above)
unsigned long startclose;
int shutoff = 0;
const int interval = 0;  // delay for valve closure (in milliseconds)
void setup()
{
  pinMode(pulseInd, OUTPUT);
  pinMode(pulseOut, OUTPUT);
//  Serial.begin(9600);  // debug code, uncomment and view Serial Monitor for values
  }

void loop ()
{  
  valHigh = analogRead(highFlowValve);
  valLow  = analogRead(lowFlowValve);
/*  Serial.print(valHigh);   // debug code
  Serial.print(" : ");
  Serial.print(valLow);
  Serial.print(" : ");
  Serial.println(shutoff);
 delay(5); */
  if ((valHigh>350) && (valHigh<500))
  {
   digitalWrite(pulseInd, HIGH);
   digitalWrite(pulseOut, HIGH);
   delayMicroseconds(fast);
   digitalWrite(pulseOut, LOW);
   delayMicroseconds(fast);
   shutoff = 0;
  }
  if ((valLow>350) && (valLow<500) && (valHigh>=500))
  {
    digitalWrite(pulseInd, HIGH);
    digitalWrite(pulseOut, HIGH);
    delayMicroseconds(slow);
    digitalWrite(pulseOut, LOW);
    delayMicroseconds(slow);
    shutoff = 0;
  }
  
  if ((valLow>=500) && (valHigh>=500) && !shutoff)
  {
    startclose = millis();
    while(millis()-startclose < interval)
    {
      digitalWrite(pulseInd, HIGH);
      digitalWrite(pulseOut, HIGH);
      delay(slow);
      digitalWrite(pulseOut, LOW);
      delay(slow);
    }
    shutoff = 1;
    digitalWrite(pulseOut, LOW);
    digitalWrite(pulseInd, LOW);
  }
}
