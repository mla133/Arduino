//By: Chuck Kozik

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used
//Therefore, 16 bit timers Timer1, Timer3, Timer4, Timer5 will be used to
//generate pulse outputs. 
//Timer1 will control PB7 (PULSE1 A) and (Pulse1 B) PH3
//Timer3
//Timer4
//Timer5



#define A_LEAD_RISE_A        1
#define A_LEAD_RISE_B        2
#define A_LEAD_FALL_A        3
#define A_LEAD_FALL_B        4
#define B_LEAD_RISE_B        6
#define B_LEAD_RISE_A        7
#define B_LEAD_FALL_B        8
#define B_LEAD_FALL_A        9

void check_pulse1_adjust(void);
void check_pulse2_adjust(void);
//Pulse Out 1
int VALVE1_LO_FLOW_DIG_IN = 19; //PD2 
int VALVE1_HI_FLOW_DIG_IN = 18; //PD3
int PULSE_OUT_1A = 13;//PB7
int PULSE_OUT_1B = 6;//PH3
unsigned long pulse1Count;
int pulse1State;
unsigned int pulse1Freq;
unsigned long pulse1AdjustWaitTime;
unsigned long pulse1AdjustTimeout;
//unsigned char pulse1Prescale;
unsigned int pulse1OCRA;
//Pulse Out 2
int VALVE2_LO_FLOW_DIG_IN = 24; //PA1 
int VALVE2_HI_FLOW_DIG_IN = 22; //PA0
int PULSE_OUT_2A = 12;//PB6
int PULSE_OUT_2B = 5;//PE3
unsigned long pulse2Count;
int pulse2State;
unsigned int pulse2Freq;
unsigned long pulse2AdjustWaitTime;
unsigned long pulse2AdjustTimeout;
unsigned int pulse2OCRA;
//Pulse Out 3
int PULSE_OUT_3A = 11;//PB5
int PULSE_OUT_3B = 4;//PG5
unsigned long pulse3Count;
int pulse3State;
unsigned int pulse3Srate;
unsigned int pulse3Freq;
unsigned long pulse3AdjustWaitTime;
unsigned long pulse3AdjustTimeout;
unsigned int pulse3OCRA;


void setup() 
{
    //set up for pulse 1 inputs and outputs
    pulse1Count = 0;
    pulse1State = A_LEAD_RISE_A;
    pulse1Freq = 0;
    pulse1AdjustWaitTime = 500;//time in milliseconds
    pulse1AdjustTimeout  = millis() + pulse1AdjustWaitTime;
    pinMode(PULSE_OUT_1A, OUTPUT);
    pinMode(PULSE_OUT_1B, OUTPUT);
    pinMode(VALVE1_LO_FLOW_DIG_IN, INPUT);
    pinMode(VALVE1_HI_FLOW_DIG_IN, INPUT);
   //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application 
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
    TCCR1A = 0;
    //set control register B for 64 bit prescale, CTC mode
    TCCR1B = ((1 << WGM12) | (1 << CS11) | (1 << CS10));
    pulse1OCRA = OCR1A = 0x00ff;
    Serial.begin(9600);
    TIMSK1 |= (1<<OCIE1A);//enable interript for Timer/Counter1 Output Compare Match A
    
    //set up for pulse 2 inputs and outputs
    pulse2Count = 0;
    pulse2State = A_LEAD_RISE_A;
    pulse2Freq = 0;
    pulse2AdjustWaitTime = 500;//time in milliseconds
    pulse2AdjustTimeout  = millis() + pulse1AdjustWaitTime;
    pinMode(PULSE_OUT_2A, OUTPUT);
    pinMode(PULSE_OUT_2B, OUTPUT);
    pinMode(VALVE2_LO_FLOW_DIG_IN, INPUT);
    pinMode(VALVE2_HI_FLOW_DIG_IN, INPUT);
    TCCR3A = 0;
    //set control register B for 64 bit prescale, CTC mode
    TCCR3B = ((1 << WGM32) | (1 << CS31) | (1 << CS30));
    pulse2OCRA = OCR3A = 0x00ff;
    TIMSK3 |= (1<<OCIE3A);//enable interript for Timer/Counter 3 Output Compare Match A
    
    SREG |= 0b1000000;//global interrupt enable
}


void loop() 
{
    unsigned long current_millis;
     
      
    current_millis = millis();
    //Serial.println(current_millis);
    if((long)(current_millis - pulse1AdjustTimeout) >= 0)
    {
        pulse1AdjustTimeout  = current_millis + pulse1AdjustWaitTime;//maintain last timeout
        check_pulse1_adjust();
        //Serial.print("time1: ");
        //Serial.println(pulse1Freq);
        //Serial.println(pulse1Count);
    }
    if((long)(current_millis - pulse2AdjustTimeout) >= 0)
    {
        pulse2AdjustTimeout  = current_millis + pulse2AdjustWaitTime;//maintain last timeout
        check_pulse2_adjust();
        //Serial.print("time1: ");
        //Serial.println(pulse1Freq);
        //Serial.println(pulse1Count);
    }
  
}

void check_pulse1_adjust(void)
{
    unsigned int ocr;
    byte dig_in;
   /*READING PORT D DIRECT
  dig_in = PIND;//don't use PORTD
  Serial.println(dig_in);*/
  //if PD2 is low then we want low flow rate or low frequency
  if(!digitalRead(VALVE1_LO_FLOW_DIG_IN)) 
    pulse1Freq = 15;//PD2 is low
  //see if we want high flow
  else if (!digitalRead(VALVE1_HI_FLOW_DIG_IN))
    pulse1Freq = 250;//PD3 is low
  else
    pulse1Freq = 0;//neither PD2 or PD3 is active 
   
  pulse1AdjustWaitTime = 50;//time in milliseconds
  //use 4000000 since we are interrupting on every every edge of the A and B pulse  
  ocr = 62500/pulse1Freq;//4000000/64 = 62500. 64 is prescale  
       
  TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
  TIMSK1 &= ~(1<<OCIE1A); //disable again
  pulse1OCRA = ocr;
  TIMSK1 |= (1<<OCIE1A);//reenable interrupt
    //Serial.print("OCR:");
    //Serial.println(OCR1A);
 
}  

void check_pulse2_adjust(void)
{
   unsigned int ocr;
   
   //if PA1 is low then we want low flow rate or low frequency
  if(!digitalRead(VALVE2_LO_FLOW_DIG_IN))
   pulse2Freq = 15;//PA1 is low
  //if PA0 is low then we want high flow rate or high frequency
  else if (!digitalRead(VALVE2_HI_FLOW_DIG_IN))
    pulse2Freq = 250;//PA0 is low
  else
    pulse2Freq = 0;//neither PA0 or PA1 is active 
  pulse1AdjustWaitTime = 50;//time in milliseconds
 //use 4000000 since we are interrupting on every every edge of the A and B pulse  
  ocr = 62500/pulse2Freq;//4000000/64 = 62500. 64 is prescale  
       
  TIMSK3 &= ~(1<<OCIE3A); //temporarily disable interrupt
  TIMSK3 &= ~(1<<OCIE3A); //disable again
  pulse2OCRA = ocr;
  TIMSK3 |= (1<<OCIE3A);//reenable interrupt
    //Serial.print("OCR:");
    //Serial.println(OCR1A);
}  


ISR(TIMER1_COMPA_vect)  
{
   static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    switch(pulse1State)
    {
      case A_LEAD_RISE_A:
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = A_LEAD_RISE_B;
            PORTB |=0x80;//set output high;
            pulse1Count++;
          }  
      break;
      case A_LEAD_RISE_B:
          pulse1State = A_LEAD_FALL_A;
          PORTH |= 0x8;//PH3..set it high
      break;
      case A_LEAD_FALL_A:
          pulse1State = A_LEAD_FALL_B;
          PORTB &=0x7f;//set low
      break;
      case A_LEAD_FALL_B:
          pulse1State = A_LEAD_RISE_A;
          PORTH &= 0xf7;//PH3, set low
      break;
    }
}  

ISR(TIMER3_COMPA_vect)  
{
   static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    switch(pulse1State)
    {
      case A_LEAD_RISE_A:
          if(pulse2Freq > 0)
          {
            OCR3A = pulse1OCRA; 
            pulse2State = A_LEAD_RISE_B;
            PORTB |=0x40;//PB6 set output high;
            pulse2Count++;
          }  
      break;
      case A_LEAD_RISE_B:
          pulse2State = A_LEAD_FALL_A;
          PORTE |= 0x8;//PE3..set it high
      break;
      case A_LEAD_FALL_A:
          pulse2State = A_LEAD_FALL_B;
          PORTB &= 0xbf;// PB6 set low
      break;
      case A_LEAD_FALL_B:
          pulse2State = A_LEAD_RISE_A;
          PORTE &= 0xf7;//PE3, set low
      break;
    }
}    
              
