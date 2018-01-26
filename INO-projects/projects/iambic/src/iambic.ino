/* Iambic keyer for arduino by Dimitris Sapountzakis (01/12/2011) */

#define DIT_PIN  8
#define DAH_PIN  10
#define EXC_PIN  9
#define LED      13
#define BAUD_DURATION          80              //mSec
#define INTERBAUD_DURATION     BAUD_DURATION*1
#define INTERLETTER_DURATION   BAUD_DURATION*2    //extra time after a baud
#define DIT_DURATION           BAUD_DURATION
#define DAH_DURATION           BAUD_DURATION*3
#define TOUCH_THRESHOLD        10        //how long to wait in uSec, before sampling the touch pin.

enum{
  IDLE,
  DIT,  
  DAH,
  PAUSE,
};

int dit,dah; 
int state;

void readDit()
{
  digitalWrite(EXC_PIN,HIGH);
  delayMicroseconds(TOUCH_THRESHOLD);
  if(digitalRead(DIT_PIN)) dit=0; else dit=1;
  digitalWrite(EXC_PIN,LOW);  
}

void readDah()
{
  digitalWrite(EXC_PIN,HIGH);
  delayMicroseconds(TOUCH_THRESHOLD);
  if(digitalRead(DAH_PIN)) dah=0; else dah=1;
  digitalWrite(EXC_PIN,LOW);  
}

void setup()
{
  pinMode(EXC_PIN,OUTPUT);
  digitalWrite(EXC_PIN,LOW);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);   //turn off led
  state = 0;
}

//handles the output of the ciruit. if state is 1 then the contact is closed or led is turned on
void contact(unsigned char state)
{
  if(state) {
    digitalWrite(LED,HIGH);
    analogWrite(3,127);  //pin 3 drives an 8 Ohm speaker
  }
  else{
    digitalWrite(LED,LOW);  
    analogWrite(3,0);
  }
}

void loop()
{
  switch(state){
    case IDLE:
      readDit();
      if(dit) {
        state = DIT;              
      }
      else{
        delayMicroseconds(30);
        readDah();
        if(dah) {
          state = DAH;              
        }
      }      
    break;

    case DIT:
      contact(1);
      delay(DIT_DURATION);
      contact(0);
      delay(INTERBAUD_DURATION);
      //now, if dah is pressed go there, else check for dit
      readDah();
      if(dah){
        state = DAH;        
      }
      else{
        //read dit now
        readDit();
        if(dit) {
          state = DIT;              
        } 
        else {
          delay(INTERLETTER_DURATION);
          state = IDLE;
        }
      }        
    break;
   
    case DAH:
      contact(1);
      delay(DAH_DURATION);
      contact(0);
      delay(INTERBAUD_DURATION);
      //now, if dit is pressed go there, else check for dah
      readDit();
      if(dit){
        state = DIT;        
      }
      else{
        //read dit now
        readDah();
        if(dah) {
          state = DAH;              
        }
        else {
          delay(INTERLETTER_DURATION);
          state = IDLE;
        }
      }        
    break;
  }//switch   
  delay(1);
}
