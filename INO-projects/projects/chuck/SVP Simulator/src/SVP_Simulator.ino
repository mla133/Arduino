//By: Chuck Kozik

/*SVP Simulator:
 When input is active, we will delay for 4 seconds, output a 1 millisecond pulse (active low)
 on output 1. We will delay another 4 seconds the output another 1 millisecond pulse.
 we will the insure input goes inactive before repeating this sequence*/

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used

#define A_LEAD_RISE_A    0
#define A_LEAD_FALL_A    1

//Pulse Out 1
int START_SVP_PROVE = 19; //PD2 
int DETECT_SWITCH = 13;//PB7
int METER_PULSE_OUT = 6;//PH3
unsigned int OCR;
unsigned int pulse1OCRA;
int pulse1State = A_LEAD_RISE_A;

void setup() 
{
  Serial.begin(9600);
  Serial.println("Hi there, i'm setting up");
  //setup for digital input to start prove and output to generate 1 millisecond pulse
  pinMode(DETECT_SWITCH, OUTPUT);
  pinMode(START_SVP_PROVE, INPUT);
  pinMode(METER_PULSE_OUT, OUTPUT);
  digitalWrite(DETECT_SWITCH, LOW);
  

  //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application 
  TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
  //Timer 1 used to simulate the meter pulse output
  TCCR1A = 0;
  //set control register B for 64 bit prescale, CTC mode
  TCCR1B = ((1 << WGM12) | (1 << CS11) | (1 << CS10));
  OCR = 62500/37;//4000000/64 = 62500. 64 is prescale 
  OCR = (OCR << 1);//multiply by two 
  TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
  TIMSK1 &= ~(1<<OCIE1A); //disable again
  pulse1OCRA = OCR;
  TIMSK1 |= (1<<OCIE1A);//reenable interrupt
  Serial.println("Activate input to start prove");
}


void loop() 
{
  unsigned long current_micros;
  unsigned long waitTime;
  int i;
  int input;

  current_micros = micros();
  waitTime = current_micros + 500000;//.5 seconds
  while((long)(current_micros - waitTime) < 0)
    current_micros = micros(); 
  input = digitalRead(START_SVP_PROVE);
  Serial.println(input);
  if(!digitalRead(START_SVP_PROVE))//when this input is low, its active (reverse logic
  {   
    current_micros = micros();
    waitTime = current_micros + 4000000;//4 seconds
    Serial.println("Puller motor active");
    while((long)(current_micros - waitTime) < 0)
      current_micros = micros();
    //4 seconds has elapsed now we will output the 1 milliseond pulse
    current_micros = micros();
    waitTime = current_micros + 1000;//1 millisecond
    digitalWrite(DETECT_SWITCH, HIGH);
    while((long)(current_micros - waitTime) < 0)
      current_micros = micros();
    digitalWrite(DETECT_SWITCH, LOW);
    //setup to delay 4 more seconds
    current_micros = micros();
    waitTime = current_micros + 4000000;//4 seconds
    Serial.println("1st detect done");
    while((long)(current_micros - waitTime) < 0)
      current_micros = micros();
    //4 seconds has elapsed now we will output the 1 milliseond pulse
    current_micros = micros();
    waitTime = current_micros + 1000;//1 millisecond
    digitalWrite(DETECT_SWITCH, HIGH);
    while((long)(current_micros - waitTime) < 0)
      current_micros = micros();
    digitalWrite(DETECT_SWITCH, LOW);
    Serial.println("2nd detect done");
    current_micros = micros();
    waitTime = current_micros + 2000000;//2 seconds
    while((long)(current_micros - waitTime) < 0)
      current_micros = micros();
    Serial.println("Home Position");
    Serial.println("Be sure to Deactivate Input");
    while(!digitalRead(START_SVP_PROVE))//be sure input is not active
      i++;
  }
  else
     Serial.println("Activate input to start prove");  
}
  
  ISR(TIMER1_COMPA_vect)  
  {
    static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    switch(pulse1State)
    {
      case A_LEAD_RISE_A:
            OCR1A = pulse1OCRA; 
            pulse1State = A_LEAD_FALL_A;
            PORTH |= 0x8;//PH3..set it high
      break;
      
      case A_LEAD_FALL_A:
          pulse1State = A_LEAD_RISE_A;
          PORTH &= 0xf7;//PH3, set low
      break;
    }
  }  

    


