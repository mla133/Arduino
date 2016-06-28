/*
 Keyer
 
 Simple keyer for the Arduino
 
 m0xpd
 shack.nasties 'at Gee Male dot com'
 
 KC3EYS modified 6/28/16
 
 Pins 6 & 7 connected to a paddle...
   pin 6 is the 'dit' key (usually the left)
   pin 7 is the 'dah' key (usually the right)
 
 Pin 8 connected to a straight key
 
 All the keys have one side to ground, the other pulled up to 5V by 10k resistors
 (advanced users can exploit the internal pull-up resistors instead)
 Pin 8 is for sidetone
 Pin 13 is the Tx output to key the transmitter - using an open-collector NPN transistor or a relay
 */


// set pin numbers:
const int ditPin = 6;   
const int dahPin = 7;
const int str8Pin = 8;
const int sidetonePin = 9;
const int TxPin =  13;   

// variables :
int ditState = 0;        
int dahState = 0;
int str8State = 0;
unsigned sidetoneFreq=600;
int ditLength = 75;             // dit length in milliseconds
int dahLength = 3 * ditLength;
int LastElement = 0;            // code for the last element sent...
                                // 0 :- space
                                // 1 :- dit
                                // 2 :- dah

void setup() {
  // initialize the Tx and sidetone pins as an output:
  pinMode(TxPin, OUTPUT);
  pinMode(sidetonePin,OUTPUT);  
  // initialize the Key pins as an inputs:
  pinMode(ditPin, INPUT);
  pinMode(dahPin, INPUT);
  pinMode(str8Pin, INPUT);
}

void loop(){
  // read the state of the inputs:
  ditState = digitalRead(ditPin);
  dahState = digitalRead(dahPin);
  str8State = digitalRead(str8Pin);

  if (str8State == LOW) {
    digitalWrite(TxPin,HIGH);
    tone(sidetonePin,sidetoneFreq);
  }

  else {
    digitalWrite(TxPin,LOW);
    noTone(sidetonePin);
    if ((ditState == LOW)&(dahState == HIGH)) {       
      LastElement = 1;
      CWKey(ditLength);
      CWSpace(ditLength);    
    } 
    else if ((dahState == LOW)&(ditState == HIGH)) {  
      LastElement = 2;
      CWKey(dahLength);
      CWSpace(ditLength);   
    }  
    else if ((dahState == LOW)&(ditState == LOW)) {  
      if (LastElement == 2) {
        LastElement = 1;
        CWKey(ditLength);
        CWSpace(ditLength);
        }
      else {
        LastElement = 2;
        CWKey(dahLength);
        CWSpace(ditLength);
        }    
      }
    else {
      LastElement = 0;
    }

  }
}

void  CWKey(int duration){
  digitalWrite(TxPin,HIGH);
  tone(sidetonePin,sidetoneFreq);
  delay(duration);
  digitalWrite(TxPin,LOW);
  noTone(sidetonePin);
}

void  CWSpace(int duration){
  digitalWrite(TxPin,LOW);
  delay(duration);
}
