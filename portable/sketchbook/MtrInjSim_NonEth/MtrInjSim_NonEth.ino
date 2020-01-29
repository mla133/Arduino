
//By: Chuck Kozik

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used
//Therefore, 16 bit timers Timer1, Timer3, Timer4, Timer5 will be used to
//generate pulse outputs. 
//Timer1 will control PB5 (PULSE1)
//Timer3 will control PE4 (PULSE2)
//Timer4 will control PH3 (PULSE3)
//Timer5 will control PL4 (PULSE4)

#include <SPI.h>

//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr-iomxx0_1.h
//be sure Tools-Board is set to correct device

#define RISE_EDGE        1
#define FALL_EDGE        2

#define MAX_FREQ_HZ          (unsigned long)130

#define POF_NORMAL_MODE   0
#define POF_LEAK_MODE     1
#define POF_CLEAR_COUNTS  2

void check_MI1_adjust(void);
void check_MI2_adjust(void);
int MtrInjOut1 = 11;//PB5 (arduino digital pin # 11)
int MtrInjOut2 = 12;//PB6 (arduino digital pin # 12)
int MtrInjOut3 = 5;//PE4 (arduino digital pin # 2)
int MtrInjOut4 = 2;//PE3 (arduino digital pin # 5)
int MtrInjSol1 = 48;
int MtrInjSol2 = 49;
int MtrInjSol3 = 21;
int MtrInjSol4 = 20;
unsigned long pulse1CountA;
int pulse1State;
unsigned int pulse1Freq;
unsigned int pulse1MaxFreq;
int pulse1OutFunctionReq;
int pulse1OutFunction;
unsigned int pulse1OCRA;
unsigned long pulse2CountA;
int pulse2State;
unsigned int pulse2Freq;
unsigned int pulse2MaxFreq;
int pulse2OutFunctionReq;
int pulse2OutFunction;
unsigned int pulse2OCRA;
unsigned long pulse3CountA;
int pulse3State;
unsigned int pulse3Srate;
unsigned int pulse3Freq;
unsigned int pulse3MaxFreq;
int pulse3OutFunctionReq;
int pulse3OutFunction;
unsigned int pulse3OCRA;
unsigned long pulse4CountA;
int pulse4State;
unsigned int pulse4Srate;
unsigned int pulse4Freq;
unsigned int pulse4MaxFreq;
int pulse4OutFunctionReq;
int pulse4OutFunction;
unsigned int pulse4OCRA;

const char *prompt =
" _____  _____   ____  _    _  _   _  _____  _____   ______  __  __    ____  \r\n"
"|_   _||  ___| / ___|| |  | || \\ | ||_   _||  __ \\ |  ____||  \\/  |  / ___| \r\n"
"  | |  | |__  | |    | |__| ||  \\| |  | |  | |__) || |__   | \\  / | | |     \r\n"
"  | |  |  __| | |    |  __  || . ` |  | |  |  ___/ |  __|  | |\\/| | | |     \r\n"
"  | |  | |___ | |___ | |  | || |\\  | _| |_ | |     | |     | |  | | | |___  \r\n"
"  |_|  |_____| \\____||_|  |_||_| \\_||_____||_|     |_|     |_|  |_|  \\____| \r\n";


void setup() 
{
    pinMode(53, OUTPUT);
    Serial.begin(115200);

    //Print Device Information
    Serial.print(prompt);
    Serial.println("\nAccuTest Board - Metered Injector Simulator\n");
   
    //indicate cause of reset
    //set up for pulse output 1, uses timer 1 
    pulse1CountA = 0;
    pulse1State = RISE_EDGE;
    pulse1Freq = 0;
    pulse1MaxFreq = MAX_FREQ_HZ;
    pinMode(MtrInjOut1, OUTPUT);
    pinMode(MtrInjOut2, OUTPUT);
    pinMode(MtrInjSol1, INPUT);
    pinMode(MtrInjSol2, INPUT);
     //TIMER0 is used for internal Arduino functionality e.g millis()
    //diconnect port pins from timer since they will be used by application 
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
    TCCR1A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR1B = ((1 << WGM12) | (1 << CS12));
    pulse1OCRA = OCR1A = 0x00ff;
    TIMSK1 |= (1<<OCIE1A);//enable interript for Timer/Counter1 Output Compare Match A
    
    //set up for pulse output 2, uses timer 3
    pulse2CountA = 0;
    pulse2State = RISE_EDGE;
    pulse2Freq = 0;
    pulse2MaxFreq = MAX_FREQ_HZ;
    pinMode(MtrInjOut3, OUTPUT);
    pinMode(MtrInjOut4, OUTPUT);
    pinMode(MtrInjSol3, INPUT);
    pinMode(MtrInjSol4, INPUT);
    TCCR3A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR3B = ((1 << WGM32) | (1 << CS32));
    pulse2OCRA = OCR3A = 0x00ff;
    TIMSK3 |= (1<<OCIE3A);//enable interript for Timer/Counter 3 Output Compare Match A
    
     //set up for pulse output 3, uses timer 4
    pulse3CountA = 0;
    pulse3State = RISE_EDGE;
    pulse3Freq = 0;
    pulse3MaxFreq = MAX_FREQ_HZ;
     TCCR4A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR4B = ((1 << WGM42) | (1 << CS42));
    pulse3OCRA = OCR4A = 0x00ff;
    TIMSK4 |= (1<<OCIE4A);//enable interript for Timer/Counter 4 Output Compare Match A
    
     //set up for pulse output 4, uses timer 5
    pulse4CountA = 0;
    pulse4State = RISE_EDGE;
    pulse4Freq = 0;
    pulse4MaxFreq = MAX_FREQ_HZ;
    TCCR5A = 0;
    //set control register B for 256 bit prescale, CTC mode
    TCCR5B = ((1 << WGM52) | (1 << CS52));
    pulse4OCRA = OCR5A = 0x00ff;
    TIMSK5 |= (1<<OCIE5A);//enable interript for Timer/Counter 5 Output Compare Match A
    
    SREG |= 0b1000000;//global interrupt enable
  
}


void loop() 
{
    unsigned long current_millis;
        
    current_millis = millis();
    check_MI1_adjust();
    check_MI2_adjust();
    check_MI3_adjust();
    check_MI4_adjust();
}

//Mtr Inj 1
void check_MI1_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   
  _on = digitalRead(MtrInjSol1);
  
  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    Serial.println("MtrInj 1 - ON");
    _freq = pulse1MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
    i++; //delay
    pulse1OCRA = ocr;
    TIMSK1 |= (1<<OCIE1A);//reenable interrupt
    _was_on = true;
    pulse1Freq = _freq;
  } 
  else if(_on==1)
  {
    //solenoid  just went not active
    pulse1Freq = 0;
    _was_on = false;
  }
   
}  
//pulse 2
void check_MI2_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   //int _down;
   
  _on = digitalRead(MtrInjSol2);

  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    Serial.println("MtrInj 2 - ON");
    _freq = pulse2MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK3 &= ~(1<<OCIE3A); //temporarily disable interrupt
    i++; //delay
    pulse2OCRA = ocr;
    TIMSK3 |= (1<<OCIE3A);//reenable interrupt
    _was_on = true;
    pulse2Freq = _freq;
  } 
  else if(_on==1) 
  {
    //solenoid  just went not active
    pulse2Freq = 0;
    _was_on = false;
  }
  
}  

//pulse 3
void check_MI3_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   
  _on = digitalRead(MtrInjSol3);
  
  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    Serial.println("MtrInj 3 - ON");
    _freq = pulse3MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK4 &= ~(1<<OCIE4A); //temporarily disable interrupt
    i++; //delay
    pulse3OCRA = ocr;
    TIMSK4 |= (1<<OCIE4A);//reenable interrupt
    _was_on = true;
    pulse3Freq = _freq;
  } 
  else if(_on==1)
  {
    //solenoid  just went not active
    pulse3Freq = 0;
    _was_on = false;
  }
   
} 

//pulse 4
void check_MI4_adjust(void)
{
   unsigned int ocr;
   unsigned int i;
   int _on;
   static bool _was_on = false;
   unsigned int _freq;
   //int _down;
   
  _on = digitalRead(MtrInjSol4);
  
  if((_on==0) && !_was_on)  //H11L1 inverts
  {
    //solenoid just went active
    Serial.println("MtrInj 4 - ON");
    _freq = pulse4MaxFreq = 70;    
   //use 8000000 since we are interrupting on rising and falling B  
    ocr = 31250/_freq;//8000000/256 = 31250. 256 is prescale
    TIMSK5 &= ~(1<<OCIE5A); //temporarily disable interrupt
    i++; //delay
    pulse4OCRA = ocr;
    TIMSK5 |= (1<<OCIE5A);//reenable interrupt
    _was_on = true;
    pulse4Freq = _freq;
  } 
  else if(_on==1)
  {
    //solenoid  just went not active
    pulse4Freq = 0;
    _was_on = false;
  }
  
} 

/********************************************************************************/

unsigned int freq_string_to_int(String HTTP_req_arg)
{
   String my_string;//object
   char char_val = 0;
   int index = 17;
   unsigned int freq = 1200;
   bool valid = false;
    
   char_val = HTTP_req_arg.charAt(index);//get first character
   //Serial.println(char_val);
   while((char_val >='0') && (char_val<='9'))
   {
     my_string += char_val;//append
     valid = true;
     index++;
     char_val = HTTP_req_arg.charAt(index);//get successive characters
   }
   if(valid)
     freq = (unsigned int)my_string.toInt();
   return(freq);
}

/**********************************************************************************/
#define P1_RISE   PORTB |=0x20;//set PB5 high(11);
#define P1_FALL   PORTB &=0xdf;//clear PB5 (11)
 
ISR(TIMER1_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
  switch(pulse1State)
  {
    case RISE_EDGE:
      if((pulse1Freq > 0) && ((PORTL && 0x02) == 0x00))//PL1 is solenoid 1
      {
        OCR1A = pulse1OCRA; 
        pulse1State = FALL_EDGE;
        P1_RISE;//generates rising edge of a pulse at output
        pulse1CountA+=1Ul;
      }  
      else
        OCR1A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse1State = RISE_EDGE;//generates falling edge of a pulse at output
        P1_FALL;
      break;

    
  }     
}  
/***************************************************************************************/
#define P2_RISE   PORTB |= 0x40;//set PB6 high(12)
#define P2_FALL   PORTB &= 0xbf;//set PB5 high(11); 

ISR(TIMER3_COMPA_vect)  
{  
  //interrupt flag automatically cleared upon interrupt execution
   switch(pulse2State)
  {
    case RISE_EDGE:
      if((pulse2Freq > 0) && ((PORTL && 0x01) == 0x00))//PL0 is solenoid 2
      {
        OCR3A = pulse2OCRA; //used to set frequency 
        pulse2State = FALL_EDGE;//setup for next time around
        P2_RISE;//generates rising edge of a pulse at output
        pulse2CountA+=1Ul;
      }  
      else
        OCR3A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse2State = RISE_EDGE;
        P2_FALL;//generates falling edge of a pulse output
      break;

    
  }     
} 
/***************************************************************************************/
#define P3_RISE   PORTE |= 0x08;//set PE4 high;
#define P3_FALL   PORTE &= 0xf7;// clear PE4 

ISR(TIMER4_COMPA_vect)  
{
    //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    switch(pulse3State)
  {
    case RISE_EDGE:
      if((pulse3Freq > 0) && ((PORTD && 0x01) == 0x00)) //PD0 is inj3 solenoid
      {
        OCR4A = pulse3OCRA; //used to set frequency 
        pulse3State = FALL_EDGE;//setup for next time around
        P3_RISE;//generates rising edge of a pulse at output
        pulse3CountA+=1Ul;
      }  
      else
        OCR4A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse3State = RISE_EDGE;
        P3_FALL;//generates falling edge of a pulse output
      break;    
  }     
 
}  
/****************************************************************************************/
#define P4_RISE   PORTE |=0x10;//set PE3 high
#define P4_FALL   PORTE &= 0xef;//clear PE3

ISR(TIMER5_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
     switch(pulse4State)
  {
    case RISE_EDGE:
      if((pulse4Freq > 0) && ((PORTD && 0x02) == 0x00))//PD1 is solenoid 4
      {
        OCR5A = pulse4OCRA; //used to set frequency 
        pulse4State = FALL_EDGE;//setup for next time around
        P4_RISE;//generates rising edge of a pulse at output
        pulse4CountA+=1Ul;
      }  
      else
        OCR5A = 0x0c35;// a default value
       break;
          
      case FALL_EDGE:
        pulse4State = RISE_EDGE;
        P4_FALL;//generates falling edge of a pulse output
      break;
   
  }     
  
}
