
//By: Chuck Kozik

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used
//Therefore, 16 bit timers Timer1, Timer3, Timer4, Timer5 will be used to
//generate pulse outputs. 
//Timer1 will control PB5 (PULSE1 A) and (Pulse1 B) PB6
//Timer3 will control PE4 (PULSE2 A) and (Pulse2 B) PH3
//Timer4 will control PH3 (PULSE3 A) and (Pulse3 B) PH4
//Timer5 will control PL4 (PULSE4 A) and (Pulse4 B) PL3

#include <SPI.h>
//be careful which Ethernet library to use. Arduino.org vs Arduino.cc
//#include <Ethernet.h>//use for older older ethernet shield
#include <Ethernet2.h> //included in version 1.7.10 of ide
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr-iomxx0_1.h
//be sure Tools-Board is set to correct device

#define A_LEAD_RISE_A        1
#define A_LEAD_RISE_B        2
#define A_LEAD_FALL_A        3
#define A_LEAD_FALL_B        4
#define B_LEAD_RISE_B        6
#define B_LEAD_RISE_A        7
#define B_LEAD_FALL_B        8
#define B_LEAD_FALL_A        9

#define MAX_FREQ_HZ          (unsigned long)1100
#define MIN_FREQ_ANA_VALVE   (unsigned long) 3

#define POF_A_LEAD_B      0
#define POF_B_LEAD_A      1
#define POF_A_ONLY        2
#define POF_B_ONLY        3
#define POF_A_LEAD_B_LEAK 4
#define POF_B_LEAD_A_LEAK 5
#define POF_A_ONLY_LEAK   6
#define POF_B_ONLY_LEAK   7
#define POF_CLEAR_COUNTS  8

#define LAST_RISE_EDGE_NONE   0
#define LAST_RISE_EDGE_A      1
#define LAST_RISE_EDGE_B      2

#define VALVE_TYPE_DIGITAL          0
#define VALVE_TYPE_ANALOG           1


// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x82, 0x5C };//byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xCC, 0xBA };
//IPAddress ip(192, 168, 181, 208); // IP address, may need to change depending on network (ip(10, 0, 0, 20);)
EthernetServer server(80);  // create a server at port 80

String HTTP_req;            // stores the HTTP request

void check_pulse1_adjust(void);
void check_pulse2_adjust(void);
//Pulse Out 1
int PULSE_OUT_1A = 11;//PB5 (arduino digital pin # 11)
int PULSE_OUT_1B = 12;//PB6 (arduino digital pin # 12)
int PULSE_1_UPSTREAM = 48;
int PULSE_1_DOWNSTREAM = 49;
unsigned long pulse1CountA;
unsigned long pulse1CountB;
unsigned long pulse1Error;
int pulse1State;
unsigned int pulse1Freq;
unsigned int pulse1MaxFreq;
unsigned long pulse1AdjustWaitTime;
unsigned long pulse1AdjustTimeout;
int pulse1OutFunctionReq;
int pulse1OutFunction;
int pulse1LastRise;
unsigned int pulse1OCRA;
unsigned int pulse1ValveType;
//Pulse Out 2
int PULSE_OUT_2A = 5;//PE4 (arduino digital pin # 2)
int PULSE_OUT_2B = 2;//PE3 (arduino digital pin # 5)
int PULSE_2_UPSTREAM = 21;
int PULSE_2_DOWNSTREAM = 20;
unsigned long pulse2CountA;
unsigned long pulse2CountB;
unsigned long pulse2Error;
int pulse2State;
unsigned int pulse2Freq;
unsigned int pulse2MaxFreq;
unsigned long pulse2AdjustWaitTime;
unsigned long pulse2AdjustTimeout;
int pulse2OutFunctionReq;
int pulse2OutFunction;
int pulse2LastRise;
unsigned int pulse2OCRA;
unsigned int pulse2ValveType;
//Pulse Out 3
int PULSE_OUT_3A = 6;//PH3 (arduino digital pin # 6)
int PULSE_OUT_3B = 7;//PH4 (arduino digital pin # 7)
int PULSE_3_UPSTREAM = 15;
int PULSE_3_DOWNSTREAM = 14;
unsigned long pulse3CountA;
unsigned long pulse3CountB;
unsigned long pulse3Error;
int pulse3State;
unsigned int pulse3Srate;
unsigned int pulse3Freq;
unsigned int pulse3MaxFreq;
unsigned long pulse3AdjustWaitTime;
unsigned long pulse3AdjustTimeout;
int pulse3OutFunctionReq;
int pulse3OutFunction;
int pulse3LastRise;
unsigned int pulse3OCRA;
unsigned int pulse3ValveType;
//Pulse Out 4
int PULSE_OUT_4A = 45;//PL4 (arduino digital pin # 45)
int PULSE_OUT_4B = 46;//PL3 (arduino digital pin # 46)
int PULSE_4_UPSTREAM = 19;
int PULSE_4_DOWNSTREAM = 18;
unsigned long pulse4CountA;
unsigned long pulse4CountB;
unsigned long pulse4Error;
int pulse4State;
unsigned int pulse4Srate;
unsigned int pulse4Freq;
unsigned int pulse4MaxFreq;
unsigned long pulse4AdjustWaitTime;
unsigned long pulse4AdjustTimeout;
int pulse4OutFunctionReq;
int pulse4OutFunction;
int pulse4LastRise;
unsigned int pulse4OCRA;
unsigned int pulse4ValveType;

const char *prompt =
  "    ______  ___       ___   ______\r\n"
  "   |   ___||   \\    /   | /       |\r\n"
  " __|  |__  |    \\  /    ||   _____|\r\n"
  "(__    __) |     \\/     ||  (_____\r\n"
  "   |  |    |  |\\    /|  ||        |\r\n"
  "   |__|    |__| \\__/ |__| \\_______| Technologies\r\n";

void setup() 
{
    //to use Ethernet 2 set pin 53 as an output
    pinMode(53, OUTPUT);
 Serial.begin(115200);

  //Print Device Information


  Serial.print(prompt);
  Serial.println("\nAccuTest Board - Meter Simulator\n");

  Serial.print("MAC Address: ");
  for (byte i = 0; i < sizeof(mac); i++)
  {
    Serial.print(mac[i], HEX);
    
    if(i < (sizeof(mac) - 1))
    {
      Serial.print("-");
    }
    else
    {
           Serial.print("\n");
    }
  }

  while(!Ethernet.begin(mac))
  {
    Serial.println("Failed to Get DHCP Address, Trying Again");
    delay(2000);
  }

  Serial.print("IP Address:  ");
  for (byte i = 0; i < 4; i++)
  {
    Serial.print(Ethernet.localIP()[i], DEC);
    if(i < 3)
    {
      Serial.print(".");
    }
    else
    {
      Serial.print("\n");
    }
  }
    
    server.begin();           // start to listen for clients
    //indicate cause of reset
    //Serial.println(MCUSR);
    //set up for pulse output 1, uses timer 1 
    pulse1CountA = 0;
    pulse1CountB = 0;
    pulse1Error = 0;
    pulse1State = A_LEAD_RISE_A;
    pulse1Freq = 0;
    pulse1AdjustWaitTime = 500;//time in milliseconds
    pulse1AdjustTimeout  = millis() + pulse1AdjustWaitTime;
    pulse1MaxFreq = MAX_FREQ_HZ;
    pulse1OutFunctionReq = POF_A_LEAD_B;
    pulse1OutFunction = POF_A_LEAD_B;
    pulse1LastRise = LAST_RISE_EDGE_NONE;
    pulse1ValveType = VALVE_TYPE_DIGITAL;
    pinMode(PULSE_OUT_1A, OUTPUT);
    pinMode(PULSE_OUT_1B, OUTPUT);
    pinMode(PULSE_1_UPSTREAM, INPUT);
    pinMode(PULSE_1_DOWNSTREAM, INPUT);
   //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application 
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
    TCCR1A = 0;
    //set control register B for 64 bit prescale, CTC mode
    TCCR1B = ((1 << WGM12) | (1 << CS11) | (1 << CS10));
    pulse1OCRA = OCR1A = 0x00ff;
    TIMSK1 |= (1<<OCIE1A);//enable interript for Timer/Counter1 Output Compare Match A
    
    //set up for pulse output 2, uses timer 3
    pulse2CountA = 0;
    pulse2CountB = 0;
    pulse2Error = 0;
    pulse2State = A_LEAD_RISE_A;
    pulse2Freq = 0;
    pulse2AdjustWaitTime = 500;//time in milliseconds
    pulse2AdjustTimeout  = millis() + pulse2AdjustWaitTime;
    pulse2MaxFreq = MAX_FREQ_HZ;
    pulse2OutFunctionReq = POF_A_LEAD_B;
    pulse2OutFunction = POF_A_LEAD_B;
    pulse2LastRise = LAST_RISE_EDGE_NONE;
    pulse2ValveType = VALVE_TYPE_DIGITAL;
    pinMode(PULSE_OUT_2A, OUTPUT);
    pinMode(PULSE_OUT_2B, OUTPUT);
    pinMode(PULSE_2_UPSTREAM, INPUT);
    pinMode(PULSE_2_DOWNSTREAM, INPUT);
    TCCR3A = 0;
    //set control register B for 64 bit prescale, CTC mode
    TCCR3B = ((1 << WGM32) | (1 << CS31) | (1 << CS30));
    pulse2OCRA = OCR3A = 0x00ff;
    TIMSK3 |= (1<<OCIE3A);//enable interript for Timer/Counter 3 Output Compare Match A
    
     //set up for pulse output 3, uses timer 4
    pulse3CountA = 0;
    pulse3CountB = 0;
    pulse3Error = 0;
    pulse3State = A_LEAD_RISE_A;
    pulse3Freq = 0;
    pulse3AdjustWaitTime = 500;//time in milliseconds
    pulse3AdjustTimeout  = millis() + pulse3AdjustWaitTime;
    pulse3MaxFreq = MAX_FREQ_HZ;
    pulse3OutFunctionReq = POF_A_LEAD_B;
    pulse3OutFunction = POF_A_LEAD_B;
    pulse3LastRise = LAST_RISE_EDGE_NONE;
    pulse3ValveType = VALVE_TYPE_DIGITAL;
    pinMode(PULSE_OUT_3A, OUTPUT);
    pinMode(PULSE_OUT_3B, OUTPUT);
    pinMode(PULSE_3_UPSTREAM, INPUT);
    pinMode(PULSE_3_DOWNSTREAM, INPUT);
    TCCR4A = 0;
    //set control register B for 64 bit prescale, CTC mode
    TCCR4B = ((1 << WGM42) | (1 << CS41) | (1 << CS40));
    pulse3OCRA = OCR4A = 0x00ff;
    TIMSK4 |= (1<<OCIE4A);//enable interript for Timer/Counter 4 Output Compare Match A
    
     //set up for pulse output 4, uses timer 5
    pulse4CountA = 0;
    pulse4CountB = 0;
    pulse4Error = 0;
    pulse4State = A_LEAD_RISE_A;
    pulse4Freq = 0;
    pulse4AdjustWaitTime = 500;//time in milliseconds
    pulse4AdjustTimeout  = millis() + pulse4AdjustWaitTime;
    pulse4MaxFreq = MAX_FREQ_HZ;
    pulse4OutFunctionReq = POF_A_LEAD_B;
    pulse4OutFunction = POF_A_LEAD_B;
    pulse4LastRise = LAST_RISE_EDGE_NONE;
    pulse4ValveType = VALVE_TYPE_DIGITAL;
    pinMode(PULSE_OUT_4A, OUTPUT);
    pinMode(PULSE_OUT_4B, OUTPUT);
    pinMode(PULSE_4_UPSTREAM, INPUT);
    pinMode(PULSE_4_DOWNSTREAM, INPUT);
    TCCR5A = 0;
    //set control register B for 64 bit prescale, CTC mode
    TCCR5B = ((1 << WGM52) | (1 << CS51) | (1 << CS50));
    pulse4OCRA = OCR5A = 0x00ff;
    TIMSK5 |= (1<<OCIE5A);//enable interript for Timer/Counter 5 Output Compare Match A
    
    SREG |= 0b1000000;//global interrupt enable
}


void loop() 
{
    unsigned long current_millis;
    EthernetClient client;
     
    current_millis = millis();
    //Serial.println(current_millis);
    if((long)(current_millis - pulse1AdjustTimeout) >= 0)
    {
       if((pulse1OutFunction == POF_CLEAR_COUNTS) || (pulse1OutFunctionReq == POF_CLEAR_COUNTS))
       {
         pulse1CountA = 0uL;
         pulse1CountB = 0uL;
         pulse1Error = 0uL;
         //pulse1LastRise = LAST_RISE_EDGE_NONE;
       }   
       if(pulse1ValveType == VALVE_TYPE_DIGITAL)
        {
          check_pulse1_adjust();
          if(pulse1Freq < 8)
           pulse1AdjustWaitTime = 750;//time in milliseconds
          else if(pulse1Freq < 12)
           pulse1AdjustWaitTime = 250;//time in milliseconds
          else
           pulse1AdjustWaitTime = 30;//time in milliseconds
        }
        else
        {
          Ana_Valve_Pulse1();
          pulse1AdjustWaitTime = 100;//time in milliseconds
        }
        pulse1AdjustTimeout  = current_millis + pulse1AdjustWaitTime;//maintain last timeout   
        //Serial.print("time1: ");
        //Serial.println(pulse1Freq);
        //Serial.println(pulse1CountAA);
    }
    if((long)(current_millis - pulse2AdjustTimeout) >= 0)
    {
        if((pulse2OutFunction == POF_CLEAR_COUNTS) || (pulse2OutFunctionReq == POF_CLEAR_COUNTS))
        {      
           pulse2CountA = 0uL;
           pulse2CountB = 0uL;
           pulse2Error = 0uL;
         //pulse1LastRise = LAST_RISE_EDGE_NONE;
        } 
      
       if(pulse2ValveType == VALVE_TYPE_DIGITAL)
       {
         check_pulse2_adjust();
         if(pulse2Freq < 8)
            pulse2AdjustWaitTime = 750;//time in milliseconds
         else if(pulse2Freq < 12)
           pulse2AdjustWaitTime = 250;//time in milliseconds
         else
           pulse2AdjustWaitTime = 30;//time in milliseconds
        }
        else
        {
         Ana_Valve_Pulse2();
         pulse2AdjustWaitTime = 100;//time in milliseconds
        }
        pulse2AdjustTimeout  = current_millis + pulse2AdjustWaitTime;//maintain last timeout   
        //Serial.print("time1: ");
        //Serial.println(pulse1Freq);
        //Serial.println(pulse1CountAA);
    }
    if((long)(current_millis - pulse3AdjustTimeout) >= 0)
    {
        if((pulse3OutFunction == POF_CLEAR_COUNTS) || (pulse3OutFunctionReq == POF_CLEAR_COUNTS))
        {
           pulse3CountA = 0uL;
           pulse3CountB = 0uL;
           pulse3Error = 0uL;
           //pulse1LastRise = LAST_RISE_EDGE_NONE;
        } 
       if(pulse3ValveType == VALVE_TYPE_DIGITAL)
       {
         check_pulse3_adjust();
         if(pulse3Freq < 8)
            pulse3AdjustWaitTime = 750;//time in milliseconds
         else if(pulse3Freq < 12)
           pulse3AdjustWaitTime = 250;//time in milliseconds
         else
           pulse3AdjustWaitTime = 30;//time in milliseconds
        }
        else
        {
         Ana_Valve_Pulse3();
         pulse3AdjustWaitTime = 100;//time in milliseconds
        }
        pulse3AdjustTimeout  = current_millis + pulse3AdjustWaitTime;//maintain last timeout 
        //Serial.print("time1: ");
        //Serial.println(pulse1Freq);
        //Serial.println(pulse1CountAA);
    }
    if((long)(current_millis - pulse4AdjustTimeout) >= 0)
    {
        if((pulse4OutFunction == POF_CLEAR_COUNTS) || (pulse4OutFunctionReq == POF_CLEAR_COUNTS))
        {
          pulse4CountA = 0uL;
          pulse4CountB = 0uL;
          pulse4Error = 0uL;
         //pulse1LastRise = LAST_RISE_EDGE_NONE;
        } 
      
        if(pulse4ValveType == VALVE_TYPE_DIGITAL)
       {
         check_pulse4_adjust();
         if(pulse4Freq < 8)
            pulse4AdjustWaitTime = 750;//time in milliseconds
         else if(pulse4Freq < 12)
           pulse4AdjustWaitTime = 250;//time in milliseconds
         else
           pulse4AdjustWaitTime = 30;//time in milliseconds
        }
        else
        {
         Ana_Valve_Pulse4();
         pulse4AdjustWaitTime = 100;//time in milliseconds
        }
        pulse4AdjustTimeout  = current_millis + pulse4AdjustWaitTime;//maintain last timeout 
        //Serial.print("time1: ");
        //Serial.println(pulse1Freq);
        //Serial.println(pulse1CountAA);
    }
    
    Ethernet_Control(client);
}

//pulse 1
void check_pulse1_adjust(void)
{
   bool pulse_increase;
   bool pulse_decrease; 
   bool _new_max_freq;
   unsigned int ocr;
   unsigned int i;
   int _up;
   int _down;
   
  pulse_increase = false;
  pulse_decrease = false;
  _new_max_freq = false; 
  _up = digitalRead(PULSE_1_UPSTREAM);
  _down = digitalRead(PULSE_1_DOWNSTREAM);
  //Serial.println(_up);
  //Serial.println(_down);
  
  //H11L1 inverts
  if(_up && !_down)
    pulse_decrease = true; //illegal state  
  else if(!_up && !_down)
    pulse_increase = true;
  else if(_up && _down)  
    pulse_decrease = true;
    
  if(pulse1Freq == 0)
    pulse1OutFunction =  pulse1OutFunctionReq;  
  if((pulse1OutFunction == POF_A_ONLY_LEAK) || (pulse1OutFunction == POF_B_ONLY_LEAK) ||
    (pulse1OutFunction == POF_A_LEAD_B_LEAK) || (pulse1OutFunction == POF_B_LEAD_A_LEAK))
    {
       pulse_increase = true;
    } 
  //Serial.println(pulse1OutFunctionReq);
  //Serial.println(pulse1OutFunction);
  if(pulse_increase) 
  {
     if(pulse1Freq == 0)
         pulse1Freq = 2;
     else if(pulse1Freq < 20)
         pulse1Freq += 2;
     else
         pulse1Freq += 4;
  } 
  else if(pulse_decrease)
  {
    if(pulse1Freq <= 3)
      pulse1Freq = 0;
    else if(pulse1Freq < 20)
      pulse1Freq -= 2;
    else 
      pulse1Freq -= 4;  
  }  
  
  if(pulse1Freq > pulse1MaxFreq)
  {
    pulse1Freq = pulse1MaxFreq;//the maximum limit   
    _new_max_freq = true;
  }
   
 if(pulse_increase || pulse_decrease || _new_max_freq)
  {  
     ocr = 0x00ff;//default setting
     if(pulse1Freq > 0)
     {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse1Freq;//4000000/64 = 62500. 64 is prescale
     }
                 
    TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
    i++; //delay
    pulse1OCRA = ocr;
    TIMSK1 |= (1<<OCIE1A);//reenable interrupt
    //Serial.print("OCR:");
    //Serial.println(OCR1A);
 }  
}  
//pulse 2
void check_pulse2_adjust(void)
{
   bool pulse_increase;
   bool pulse_decrease;
   bool _new_max_freq; 
   unsigned int ocr;
   unsigned int i;
   
   int _up;
   int _down;
   
  pulse_increase = false;
  pulse_decrease = false;
  _new_max_freq = false;
  _up = digitalRead(PULSE_2_UPSTREAM);
  _down = digitalRead(PULSE_2_DOWNSTREAM);
  //Serial.println(_up);
  //Serial.println(_down);
  
    //H11L1 inverts
  if(_up && !_down)
    pulse_decrease = true; //illegal state  
  else if(!_up && !_down)
    pulse_increase = true;
  else if(_up && _down)  
    pulse_decrease = true;
  
  if(pulse2Freq == 0)
    pulse2OutFunction =  pulse2OutFunctionReq;
  if((pulse2OutFunction == POF_A_ONLY_LEAK) || (pulse2OutFunction == POF_B_ONLY_LEAK) ||
    (pulse2OutFunction == POF_A_LEAD_B_LEAK) || (pulse2OutFunction == POF_B_LEAD_A_LEAK))
     {
       pulse_increase = true;
     } 
  
  if(pulse_increase)
  {
     if(pulse2Freq == 0)
         pulse2Freq = 2;
     else if(pulse2Freq < 20)
         pulse2Freq += 2;
     else
         pulse2Freq += 4;
  } 
  else if(pulse_decrease)
  {
    if(pulse2Freq <= 3)
      pulse2Freq = 0;
    else if(pulse2Freq < 20)
      pulse2Freq -= 2;
    else 
      pulse2Freq -= 4;  
  }  
  
  if(pulse2Freq > pulse2MaxFreq)
  {
    pulse2Freq = pulse2MaxFreq;//the maximum limit 
    _new_max_freq = true;
  }
   
  //pulse2Freq = 3;
  if(pulse_increase || pulse_decrease || _new_max_freq)
  {  
     if(pulse2Freq < 8)
       pulse2AdjustWaitTime = 750;//time in milliseconds
     else if(pulse2Freq < 12)
       pulse2AdjustWaitTime = 250;//time in milliseconds
     else
       pulse2AdjustWaitTime = 50;//time in milliseconds
     ocr = 0x00ff;//default setting
     if(pulse2Freq > 0)
     {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse2Freq;//4000000/64 = 62500. 64 is prescale 
     }
        
    TIMSK3 &= ~(1<<OCIE3A); //temporarily disable interrupt
    i++; //delay
    pulse2OCRA = ocr;
    TIMSK3 |= (1<<OCIE3A);//reenable interrupt
    //Serial.print("OCR:");
    //Serial.println(OCR1A);
 }
}  

//pulse 3
void check_pulse3_adjust(void)
{
   bool pulse_increase;
   bool pulse_decrease;
   bool _new_max_freq; 
   unsigned int ocr;
   unsigned int i;
   int _up;
   int _down;
   
  pulse_increase = false;
  pulse_decrease = false;
  _new_max_freq = false;
  _up = digitalRead(PULSE_3_UPSTREAM);
  _down = digitalRead(PULSE_3_DOWNSTREAM);
  //Serial.println(_up);
  //Serial.println(_down);
  
    //H11L1 inverts
  if(_up && !_down)
    pulse_decrease = true; //illegal state  
  else if(!_up && !_down)
    pulse_increase = true;
  else if(_up && _down)  
    pulse_decrease = true;
  
  if(pulse3Freq == 0)
    pulse3OutFunction =  pulse3OutFunctionReq;
  if((pulse3OutFunction == POF_A_ONLY_LEAK) || (pulse3OutFunction == POF_B_ONLY_LEAK) ||
    (pulse3OutFunction == POF_A_LEAD_B_LEAK) || (pulse3OutFunction == POF_B_LEAD_A_LEAK))
   {
       pulse_increase = true;
   } 
  
  
  if(pulse_increase)
  {
     if(pulse3Freq == 0)
         pulse3Freq = 2;
     else if(pulse3Freq < 20)
         pulse3Freq += 2;
     else
         pulse3Freq += 4;
  } 
  else if(pulse_decrease)
  {
    if(pulse3Freq <= 3)
      pulse3Freq = 0;
    else if(pulse3Freq < 20)
      pulse3Freq -= 2;
    else 
      pulse3Freq -= 4;  
  }  
  
  if(pulse3Freq > pulse3MaxFreq)
  {
    pulse3Freq = pulse3MaxFreq;//the maximum limit    
    _new_max_freq = true;
  }
   
  if(pulse_increase || pulse_decrease || _new_max_freq)
  {  
     if(pulse3Freq < 8)
       pulse3AdjustWaitTime = 750;//time in milliseconds
     else if(pulse3Freq < 12)
       pulse3AdjustWaitTime = 250;//time in milliseconds
     else
       pulse3AdjustWaitTime = 50;//time in milliseconds
     ocr = 0x00ff;
     if(pulse3Freq > 0)
     {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse3Freq;//4000000/64 = 62500. 64 is prescale
     }
     
    TIMSK4 &= ~(1<<OCIE4A); //temporarily disable interrupt
    i++;//delay
    pulse3OCRA = ocr;
    TIMSK4 |= (1<<OCIE4A);//reenable interrupt
    //Serial.print("OCR:");
    //Serial.println(OCR1A);
 } 
} 

//pulse 4
void check_pulse4_adjust(void)
{
   bool pulse_increase;
   bool pulse_decrease;
   bool _new_max_freq; 
   unsigned int ocr;
   unsigned int i;
  int _up;
   int _down;
   
  pulse_increase = false;
  pulse_decrease = false;
  _new_max_freq = false;
  _up = digitalRead(PULSE_4_UPSTREAM);
  _down = digitalRead(PULSE_4_DOWNSTREAM);
  //Serial.println(_up);
  //Serial.println(_down);
  
   //H11L1 inverts
  if(_up && !_down)
    pulse_decrease = true; //illegal state  
  else if(!_up && !_down)
    pulse_increase = true;
  else if(_up && _down)  
    pulse_decrease = true;
    
  if(pulse4Freq == 0)
    pulse4OutFunction =  pulse4OutFunctionReq;
  if((pulse4OutFunction == POF_A_ONLY_LEAK) || (pulse4OutFunction == POF_B_ONLY_LEAK) ||
    (pulse4OutFunction == POF_A_LEAD_B_LEAK) || (pulse4OutFunction == POF_B_LEAD_A_LEAK))
    {
       pulse_increase = true;
    } 
  
  if(pulse_increase)
  {
     if(pulse4Freq == 0)
         pulse4Freq = 2;
     else if(pulse4Freq < 20)
         pulse4Freq += 2;
     else
         pulse4Freq += 4;
  } 
  else if(pulse_decrease)
  {
    if(pulse4Freq <= 3)
      pulse4Freq = 0;
    else if(pulse4Freq < 20)
      pulse4Freq -= 2;
    else 
      pulse4Freq -= 4;  
  }  
  
  if(pulse4Freq > pulse4MaxFreq)
  {
    pulse4Freq = pulse4MaxFreq;//the maximum limit 
    _new_max_freq = true;
  }
   
  if(pulse_increase || pulse_decrease || _new_max_freq)
  {  
     if(pulse4Freq < 8)
       pulse4AdjustWaitTime = 750;//time in milliseconds
     else if(pulse4Freq < 12)
       pulse4AdjustWaitTime = 250;//time in milliseconds
     else
       pulse4AdjustWaitTime = 50;//time in milliseconds
     ocr = 0x00ff;
     if(pulse4Freq > 0)
     {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse4Freq;//4000000/64 = 62500. 64 is prescale  
     }   
    TIMSK5 &= ~(1<<OCIE5A); //temporarily disable interrupt
    i++;//delay
    pulse4OCRA = ocr;
    TIMSK5 |= (1<<OCIE5A);//reenable interrupt
    //Serial.print("OCR:");
    //Serial.println(OCR1A);
 } 
} 

/***********************************************************************************/
//the A/D refeerence is set to 2.56 volts, the resistance to 4-20ma input is 100 ohms
  //We will not output a square wave until voltage reaches 0.425 volts (4.25ma).
  //with 4.25ma input the A/D counts will be  > 202. With 20ma input the A/D counts will be about 896.
  //the A/d is 10 bit (1024). 
#define MIN_AD_COUNTS  202
#define MAX_AD_COUNTS  896
#define Slope  ((float)MAX_FREQ_HZ - (float)MIN_FREQ_ANA_VALVE)/((float)MAX_AD_COUNTS - (float)MIN_AD_COUNTS)
void Ana_Valve_Pulse1(void)
{
  unsigned int _ad_counts;
  unsigned int ocr;
  unsigned char _adcl;
  unsigned char _adch;
  float _float_freq;
  unsigned int _int_freq;
  int i = 0;

  ADCSRA = (1 << ADEN); //enable A/D converter
  ADCSRB = (1 << MUX5);//set to read channel 8
  ADMUX = 0xc0; //set to read channel 8 with internal 2.56 reference
  DIDR2 |= 0x05;//write digital input disable
  ADCSRA |= (1<<ADSC); //start conversion
  //wait or conversion to complete
  //Serial.println("Enter Ana_Valve");
  while ((ADCSRA & 0x40) == 0x40)
    i++;
  //read ad counts
  _adcl = ADCL;
  _adch = ADCH;
  _ad_counts = (unsigned int)(_adch << 8);
  _ad_counts |= (unsigned int)(_adcl);
  Serial.println(_ad_counts);
  Serial.println(ADCSRB);
  Serial.println(ADMUX);
  Serial.println("");
  if(_ad_counts < (unsigned int)MIN_AD_COUNTS)
    pulse1Freq = (unsigned int) 0;
  else if(_ad_counts >= (unsigned int)MAX_AD_COUNTS)
   pulse1Freq = (unsigned int) 1100;   
  else
  {
    _float_freq = (((float)_ad_counts - (float)MIN_AD_COUNTS) * Slope) + (float)MIN_FREQ_ANA_VALVE;
    pulse1Freq = (unsigned int)_float_freq;
  } 
 
  if(pulse1Freq > pulse1MaxFreq)
    pulse1Freq = pulse1MaxFreq;//the maximum limit  
  
  if(pulse1Freq > 0)
  {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse1Freq;//4000000/64 = 62500. 64 is prescale
  }
  TIMSK1 &= ~(1<<OCIE1A); //temporarily disable interrupt
  i++; //delay
  pulse1OCRA = ocr;
  TIMSK1 |= (1<<OCIE1A);//reenable interrupt
}

/**********************************************************************************/
void Ana_Valve_Pulse2(void)
{
  unsigned int _ad_counts;
  unsigned int ocr;
  unsigned char _adcl;
  unsigned char _adch;
  float _float_freq;
  unsigned int _int_freq;
  int i = 0;
  
  ADCSRA = (1 << ADEN); //enable A/D converter
  ADCSRB = (1 << MUX5);//set to read channel 10
  ADMUX = 0xc2; //set to read channel 10 with internal 2.56 reference
  DIDR2 |= 0xa;//write digital input disable
  ADCSRA |= (1<<ADSC); //start conversion
  //wait or conversion to complete
  while ((ADCSRA & 0x40) == 0x40)
    i++;
  //read ad counts
  _adcl = ADCL;
  _adch = ADCH;
  _ad_counts = (unsigned int)(_adch << 8);
  _ad_counts |= (unsigned int)(_adcl);
  if(_ad_counts < (unsigned int)MIN_AD_COUNTS)
    pulse2Freq = (unsigned int) 0;
  else if(_ad_counts >= (unsigned int)MAX_AD_COUNTS)
   pulse2Freq = (unsigned int) 1100;   
  else
  {
    _float_freq = (((float)_ad_counts - (float)MIN_AD_COUNTS) * Slope) + (float)MIN_FREQ_ANA_VALVE;
    pulse2Freq = (unsigned int)_float_freq;
  } 
 
  if(pulse2Freq > pulse2MaxFreq)
    pulse2Freq = pulse2MaxFreq;//the maximum limit  
  
  if(pulse2Freq > 0)
  {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse2Freq;//4000000/64 = 62500. 64 is prescale
  }
  TIMSK3 &= ~(1<<OCIE3A); //temporarily disable interrupt
   i++; //delay
  pulse2OCRA = ocr;
  TIMSK3 |= (1<<OCIE3A);//reenable interrupt
}

/**********************************************************************************/
void Ana_Valve_Pulse3(void)
{
  unsigned int _ad_counts;
  unsigned int ocr;
  unsigned char _adcl;
  unsigned char _adch;
  float _float_freq;
  unsigned int _int_freq;
  int i = 0;
  
  ADCSRA = (1 << ADEN); //enable A/D converter
  ADCSRB = (1 << MUX5);//set to read channel 9
  ADMUX = 0xc1; //set to read channel 9 with internal 2.56 reference
  DIDR2 |= 0x4;//write digital input disable
  ADCSRA |= (1<<ADSC); //start conversion
  //wait or conversion to complete
  while ((ADCSRA & 0x40) == 0x40)
    i++;
  //read ad counts
  _adcl = ADCL;
  _adch = ADCH;
  _ad_counts = (unsigned int)(_adch << 8);
  _ad_counts |= (unsigned int)(_adcl);
  if(_ad_counts < (unsigned int)MIN_AD_COUNTS)
    pulse3Freq = (unsigned int) 0;
  else if(_ad_counts >= (unsigned int)MAX_AD_COUNTS)
   pulse3Freq = (unsigned int) 1100;   
  else
  {
    _float_freq = (((float)_ad_counts - (float)MIN_AD_COUNTS) * Slope) + (float)MIN_FREQ_ANA_VALVE;
    pulse3Freq = (unsigned int)_float_freq;
  } 
 
  if(pulse3Freq > pulse3MaxFreq)
    pulse3Freq = pulse3MaxFreq;//the maximum limit  
  
  if(pulse3Freq > 0)
  {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse3Freq;//4000000/64 = 62500. 64 is prescale
  }
  TIMSK4 &= ~(1<<OCIE4A); //temporarily disable interrupt
  i++;//delay
  pulse3OCRA = ocr;
  TIMSK4 |= (1<<OCIE4A);//reenable interrupt
}

/**********************************************************************************/
void Ana_Valve_Pulse4(void)
{
  unsigned int _ad_counts;
  unsigned int ocr;
  unsigned char _adcl;
  unsigned char _adch;
  float _float_freq;
  unsigned int _int_freq;
  int i = 0;
  
  ADCSRA = (1 << ADEN); //enable A/D converter
  ADCSRB = (1 << MUX5);//set to read channel 11
  ADMUX = 0xc3; //set to read channel 11 with internal 2.56 reference
  DIDR2 |= 0x8;//write digital input disable
  ADCSRA |= (1<<ADSC); //start conversion
  //wait or conversion to complete
  while ((ADCSRA & 0x40) == 0x40)
    i++;
  //read ad counts
  _adcl = ADCL;
  _adch = ADCH;
  _ad_counts = (unsigned int)(_adch << 8);
  _ad_counts |= (unsigned int)(_adcl);
  if(_ad_counts < (unsigned int)MIN_AD_COUNTS)
    pulse4Freq = (unsigned int) 0;
  else if(_ad_counts >= (unsigned int) MAX_AD_COUNTS)
   pulse4Freq = (unsigned int) 1100;   
  else
  {
    _float_freq = (((float)_ad_counts - (float)MIN_AD_COUNTS) * Slope) + (float)MIN_FREQ_ANA_VALVE;
    pulse4Freq = (unsigned int)_float_freq;
  } 
 
  if(pulse4Freq > pulse4MaxFreq)
    pulse4Freq = pulse4MaxFreq;//the maximum limit  
  
  if(pulse4Freq > 0)
  {
       //use 4000000 since we are interrupting on every every edge of the A and B pulse  
       ocr = 62500/pulse4Freq;//4000000/64 = 62500. 64 is prescale
  }
  TIMSK5 &= ~(1<<OCIE5A); //temporarily disable interrupt
  i++;//delay
  pulse4OCRA = ocr;
  TIMSK5 |= (1<<OCIE5A);//reenable interrupt
}

/**********************************************************************************/
void Ethernet_Control(EthernetClient client) 
{
  //Serial.print("Hi Chuck");  
    client = server.available();  // try to get client
    int i =0;
    
    if (client)
    {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: keep-alive");
                    client.println();
                     //Serial.print(HTTP_req);                
                    // AJAX request for switch state
                    if (HTTP_req.indexOf("ajax_pulse") > -1)
                    {
                         // read switch state and analog input
                        //Serial.print(HTTP_req);
                        GetAjaxData(client);
                    }
                    else if(HTTP_req.indexOf("mtr1.txt")> -1)
                    {
                      pulse1OutFunctionReq = pulse_function_out(HTTP_req);
                      //client.println(numSelects);//chuck
                      //Serial.print(pulse1OutFunctionReq);//chuck
                    }
                     else if(HTTP_req.indexOf("mtr2.txt")> -1)
                    {
                       pulse2OutFunctionReq = pulse_function_out(HTTP_req);
                      //client.println(numSelects2);//chuck
                      //Serial.print(HTTP_req);//chuck
                    }
                      else if(HTTP_req.indexOf("mtr3.txt")> -1)
                    {
                      pulse3OutFunctionReq = pulse_function_out(HTTP_req);
                      //client.println(numSelects3);//chuck
                      //Serial.print(HTTP_req);//chuck
                    }
                    else if(HTTP_req.indexOf("mtr4.txt")> -1)
                    {
                      pulse4OutFunctionReq = pulse_function_out(HTTP_req);
                      //client.println(numSelects4);//chuck
                      //Serial.print(HTTP_req);//chuck
                    }
                    else if(HTTP_req.indexOf("m1sld.txt") > -1)
                    {
                      //Serial.print(HTTP_req);//chuck
                      pulse1MaxFreq = freq_string_to_int(HTTP_req);
                      //Serial.print(pulse1MaxFreq);
                    }
                    else if(HTTP_req.indexOf("m2sld.txt") > -1)
                    {
                      //Serial.print(HTTP_req);//chuck
                      pulse2MaxFreq = freq_string_to_int(HTTP_req);
                      //Serial.print(pulse1MaxFreq);
                    }
                    else if(HTTP_req.indexOf("m3sld.txt") > -1)
                    {
                      //Serial.print(HTTP_req);//chuck
                      pulse3MaxFreq = freq_string_to_int(HTTP_req);
                      //Serial.print(pulse1MaxFreq);
                    }
                    else if(HTTP_req.indexOf("m4sld.txt") > -1)
                    {
                      //Serial.print(HTTP_req);//chuck
                      pulse4MaxFreq = freq_string_to_int(HTTP_req);
                      //Serial.print(pulse1MaxFreq);
                    }
                    else if(HTTP_req.indexOf("vlv1.txt")> -1)
                    {
                      if(HTTP_req.indexOf("Ana") > -1)
                        pulse1ValveType = VALVE_TYPE_ANALOG;
                      else 
                        pulse1ValveType = VALVE_TYPE_DIGITAL;
                      //client.println(numSelects);//chuck
                      Serial.print(pulse1ValveType);//chuck
                    }
                     else if(HTTP_req.indexOf("vlv2.txt")> -1)
                    {
                      if(HTTP_req.indexOf("Ana") > -1)
                        pulse2ValveType = VALVE_TYPE_ANALOG;
                      else 
                        pulse2ValveType = VALVE_TYPE_DIGITAL;
                      //client.println(numSelects2);//chuck
                      //Serial.print(HTTP_req);//chuck
                    }
                      else if(HTTP_req.indexOf("vlv3.txt")> -1)
                    {
                      if(HTTP_req.indexOf("Ana") > -1)
                        pulse3ValveType = VALVE_TYPE_ANALOG;
                      else 
                        pulse3ValveType = VALVE_TYPE_DIGITAL;
                      //client.println(numSelects3);//chuck
                      //Serial.print(HTTP_req);//chuck
                    }
                    else if(HTTP_req.indexOf("vlv4.txt")> -1)
                    {
                      if(HTTP_req.indexOf("Ana") > -1)
                        pulse4ValveType = VALVE_TYPE_ANALOG;
                      else 
                        pulse4ValveType = VALVE_TYPE_DIGITAL;
                      //client.println(numSelects4);//chuck
                      //Serial.print(HTTP_req);//chuck
                    }
                    else
                    {  // HTTP request for web page
                        // send web page - contains JavaScript with AJAX calls
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Meter 1-4 Control Web Page</title>");
                        client.println("<script>");
                        client.println("function GetPulseCountData() {");
                        client.println(
                            "nocache = \"&nocache=\" + Math.random() * 1000000;"); 
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");
                        client.println("document.getElementById(\"pulse_data\").innerHTML = this.responseText;");
                        client.println("}}}}");
                        client.println(
                        "request.open(\"GET\", \"ajax_pulse\" + nocache, true);");
                        client.println("request.send(null);");
                        client.println("setTimeout('GetPulseCountData()', 1000);");//client.println("setTimeout('GetPulseCountData()', 1000);");
                        client.println("}");
                        /***/
                        //Meter Valve 1 Selection
                      client.println("function valveSelect1(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    
                      //client.println("document.getElementById(\"pulse1\").selectedIndex;");
                      client.println("xmlhttp.open(\"GET\",\"vlv1.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect1(str) 
                      //Meter Valve 2 Selection
                      client.println("function valveSelect2(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    
                      //client.println("document.getElementById(\"pulse1\").selectedIndex;");
                      client.println("xmlhttp.open(\"GET\",\"vlv2.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect2(str) 
                      //Meter Valve 3 Selection
                      client.println("function valveSelect3(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    
                      //client.println("document.getElementById(\"pulse1\").selectedIndex;");
                      client.println("xmlhttp.open(\"GET\",\"vlv3.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect3(str) 
                      //Meter Valve 4 Selection
                      client.println("function valveSelect4(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    
                      //client.println("document.getElementById(\"pulse1\").selectedIndex;");
                      client.println("xmlhttp.open(\"GET\",\"vlv4.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function valveSelect4(str) 
                       /***/
                       //Mter 1 pulse Selection
                      client.println("function pulseSelect1(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");    
                      //client.println("document.getElementById(\"pulse1\").selectedIndex;");
                      client.println("xmlhttp.open(\"GET\",\"mtr1.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect1(str) 
                    //Meter 2 Pulse Selection 
                      client.println("function pulseSelect2(str) {");
                      client.println("xmlhttp=new XMLHttpRequest();");   
                      client.println("xmlhttp.open(\"GET\",\"mtr2.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect2(str) 
                      //Meter 3 Pulse Selection
                       client.println("function pulseSelect3(str) {");
                       client.println("xmlhttp=new XMLHttpRequest();");    
                      client.println("xmlhttp.open(\"GET\",\"mtr3.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect3(str) 
                      //Meter 4 Pulse Selection
                       client.println("function pulseSelect4(str) {");
                       client.println("xmlhttp=new XMLHttpRequest();");    
                      client.println("xmlhttp.open(\"GET\",\"mtr4.txt?q=\"+str,true);");
                      //client.println("xmlhttp.open(\"GET\", \"getcustomer.asp?q=\" + str, true);");
                      client.println("xmlhttp.send();");
                      client.println("}");//end of function pulseSelect4(str) 
                      
                      //start of meter 1 slider() 
                     client.println("function updateSlider1(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider1\");");
                     client.println("document.getElementById(\"slider1\").innerHTML= \"Meter 1 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m1sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 1 slider 
                    //start of meter 2 slider() 
                     client.println("function updateSlider2(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider2\");");
                     client.println("document.getElementById(\"slider2\").innerHTML= \"Meter 2 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m2sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 2 slider 
                    //start of meter 3 slider() 
                     client.println("function updateSlider3(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider3\");");
                     client.println("document.getElementById(\"slider3\").innerHTML= \"Meter 3 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m3sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 3 slider 
                    //start of meter 4 slider() 
                     client.println("function updateSlider4(slideAmount)");
                     client.println("{");
                     client.println("var xmlhttp= new XMLHttpRequest();");  
                     client.println("var sliderDiv = document.getElementById(\"slider4\");");
                     client.println("document.getElementById(\"slider4\").innerHTML= \"Meter 4 = \"+slideAmount+\" Hz\";");
                     client.println("xmlhttp.open(\"GET\",\"m4sld.txt?q=\"+slideAmount,true);");
                     client.println("xmlhttp.send();");
                     client.println("}");
                    //end of meter 4 slider 
                        
                        /****/
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body onload=\"GetPulseCountData()\">");
                        client.println("<h1>Meter Pulse Data (1-4)</h1>");
                        client.println("<div id=\"pulse_data\">");
                        client.println("</div>");
                        /****************************************/
                        //Start of Valve Selections
                          //Start table for Meter Valve selections
                       client.println("<br>");
                       client.println("<table style=\"width:40%\">");
                       client.println("<tr>");
                         //select list valve 1 
                       client.println("<td>");
                       client.println("<p>Meter 1 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve1\" onchange=\"valveSelect1(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                       //select list Valve 2
                       client.println("<td>");
                      client.println("<p>Meter 2 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve2\" onchange=\"valveSelect2(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                       //select list Valve 3
                       client.println("<td>");
                       client.println("<p>Meter 3 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve3\" onchange=\"valveSelect3(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                        //select list Valve 4
                      client.println("<td>");
                      client.println("<p>Meter 4 Valve</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"valve4\" onchange=\"valveSelect4(this.value)\">");
                       client.println("<option value=\"Dig\">Digital 210</option>");
                       client.println("<option value=\"Ana\">Analog</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                       //end od table
                       client.println("</tr");
                       //client.println("</form>");
                       client.println("</table>");
                        //End of Valve Selections
                        /***************************************/
                       //Start table for Meter Pulse selections
                       //client.println("<br>");
                       client.println("<table style=\"width:40%\">");
                       client.println("<tr>");
                         //select list meter 1
                       client.println("<td>");
                       client.println("<p>Meter 1 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse1\" onchange=\"pulseSelect1(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                       //select list meter 2
                       client.println("<td>");
                       client.println("<p>Meter 2 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse2\" onchange=\"pulseSelect2(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                       //select list meter 3
                       client.println("<td>");
                       client.println("<p>Meter 3 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse3\" onchange=\"pulseSelect3(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                        //select list meter 4
                       client.println("<td>");
                       client.println("<p>Meter 4 Pulse</p>");
                       client.println("<form action=\"\" method=\"get\">"); 
                       client.println("<select name=\"pulse4\" onchange=\"pulseSelect4(this.value)\">");
                       client.println("<option value=\"A0\">A Lead B</option>");
                       client.println("<option value=\"B1\">B Lead A</option>");
                       client.println("<option value=\"C2\">A Only</option>");
                       client.println("<option value=\"D3\">B Only</option>");
                       client.println("<option value=\"E4\">A Lead B Leak</option>");
                       client.println("<option value=\"F5\">B Lead A Leak</option>");
                       client.println("<option value=\"G6\">A Only Leak</option>");
                       client.println("<option value=\"H7\">B Only Leak</option>");
                       client.println("<option value=\"I8\">Clear Counts</option>");
                       client.println("</select>");
                       client.println("</form>");
                       //client.println("<br>");
                       //client.println("<div id=\"txtHint\">""</div>");
                       client.println("</td>");
                       //end od table
                       client.println("</tr");
                       //client.println("</form>");
                       client.println("</table>");
                       //start table for slides to set frequency
                       //slider meter 1        
                       //client.println("<br> <br> <br>");
                       client.println("<table style=\"width:70%\">");
                       client.println("<tr>");
                       client.println("<td>");
                       client.println("<p>Meter 1 Freq Adjust</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider1(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider1\"></div>");
                       client.println("</td>");
                       //end slider meter 1
                       //start meter 2 slide
                       client.println("<td>");
                       client.println("<p>Meter 2 Freq Adjust</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider2(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider2\"></div>");
                       client.println("</td>");
                       //end meter 2 slide
                       //start meter 3 slide
                       client.println("<td>");
                       client.println("<p>Meter 3 Freq Adjust</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider3(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider3\"></div>");
                       client.println("</td>");
                       //end meter 3 slide
                       //start meter 4 slide
                       client.println("<td>");
                       client.println("<p>Meter 4 Freq Adjust</p>");
                       client.println("0 Hz <input id=\"slide\" type=\"range\"");
                       client.println("min=\"0\" max=\"1100\" step=\"8\" value=\"1100\"");
                       client.println("onchange=\"updateSlider4(this.value)\">");
                       client.println("1100 Hz");
                       client.println("<div id=\"slider4\"></div>");
                       client.println("</td>");
                       //end meter 4 slide
                       client.println("</tr");
                       client.println("</table>");
                       
                       client.println("</body>");
                        //*******
                        client.println("</html>");
                    }
                    // display received HTTP request on serial port
                    //Serial.print(HTTP_req);
                    HTTP_req = "";            // finished with request, empty string
                    break;//while loop
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n')
                {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r')
                {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data, 1 millisecond
        client.stop(); // close the connection
    } // end if (client)
}

void GetAjaxData(EthernetClient cl)
{
  //cl.println("<table style="width:100"%>"); 
   cl.println("<table style=\"width:50%\">");
   cl.println("<tr>");
   cl.println("<td>");
   cl.println(" ");
   cl.println("</td>");
   cl.println("<td>");
   cl.println("<h3>A Counts</h3>");
   cl.println("</td>");
   cl.println("<td>");
   cl.println("<h3>B Counts</h3>");
   cl.println("</td>");
   cl.print("<td>");
   cl.println("<h3>Errors</h3>");
   cl.println("</td>");
   cl.print("<td>");
   cl.println("<h3>Pulse Freq</h3>");
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 1
   cl.println("<td>");
   cl.println("Meter 1:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1CountB);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1Error);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse1Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 2
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Meter 2:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2CountB);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2Error);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse2Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //Meter 3
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Meter 3:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse3CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse3CountB);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse3Error);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse3Freq);
   cl.println("</td>");
   cl.println("</tr>");
    //Meter 4
   cl.println("<tr>"); 
   cl.println("<td>");
   cl.println("Meter 4:");
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse4CountA);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse4CountB);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse4Error);
   cl.println("</td>");
   cl.println("<td>");
   cl.println(pulse4Freq);
   cl.println("</td>");
   cl.println("</tr>");
   //close the table
   cl.println("</table>");

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
     //Serial.println(char_val);
   }
   if(valid)
     freq = (unsigned int)my_string.toInt();
   //Serial.println(freq);
   return(freq);
}

/****************************************************************************/
int pulse_function_out(String HTTP_req_arg)
{
  int func;
  func = POF_A_LEAD_B;//default
  if(HTTP_req_arg.indexOf("A0") > -1)
   func = POF_A_LEAD_B;
  else if(HTTP_req_arg.indexOf("B1") > -1)
    func = POF_B_LEAD_A; 
  else if(HTTP_req_arg.indexOf("C2") > -1)
    func = POF_A_ONLY; 
  else if(HTTP_req_arg.indexOf("D3") > -1)
    func = POF_B_ONLY; 
  else if(HTTP_req_arg.indexOf("E4") > -1)
    func = POF_A_LEAD_B_LEAK; 
  else if(HTTP_req_arg.indexOf("F5") > -1)
    func = POF_B_LEAD_A_LEAK; 
  else if(HTTP_req_arg.indexOf("G6") > -1)
    func = POF_A_ONLY_LEAK; 
  else if(HTTP_req_arg.indexOf("H7") > -1)
    func = POF_B_ONLY_LEAK;
   else if(HTTP_req_arg.indexOf("I8") > -1)
    func = POF_CLEAR_COUNTS;  
 //Serial.println("");   
 //Serial.println(func);
 //Serial.println("");
  return(func);
}

/**********************************************************************************/
#define P1_RISE_A   PORTB |=0x20;//set PB5 high(11);
#define P1_RISE_B   PORTB |= 0x40;//set PB6 high(12)
#define P1_FALL_A   PORTB &=0xdf;//clear PB5 (11)
#define P1_FALL_B   PORTB &= 0xbf;//set PB5 high(11); 
ISR(TIMER1_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    if((pulse1OutFunction == POF_A_LEAD_B) || (pulse1OutFunction == POF_A_LEAD_B_LEAK))
    {
      switch(pulse1State)
      {
        case A_LEAD_RISE_A:
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = A_LEAD_RISE_B;
            P1_RISE_A;
            pulse1CountA+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_A)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_A;
          }  
        break;
        case A_LEAD_RISE_B:
          pulse1State = A_LEAD_FALL_A;
          P1_RISE_B;
          pulse1CountB+=1Ul;
          pulse1LastRise = LAST_RISE_EDGE_B;
        break;
        case A_LEAD_FALL_A:
          pulse1State = A_LEAD_FALL_B;
          P1_FALL_A;
        break;
        case A_LEAD_FALL_B:
          pulse1State = A_LEAD_RISE_A;
          P1_FALL_B
          pulse1OutFunction = pulse1OutFunctionReq; 
        break;
        default://same as A_LEAD_RISE_A
         if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = A_LEAD_RISE_B;
            P1_RISE_A;
            pulse1CountA+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_A)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_A;
          }  
        break;
      }
    }
    else if((pulse1OutFunction == POF_B_LEAD_A) || (pulse1OutFunction == POF_B_LEAD_A_LEAK))
    {
      switch(pulse1State)
      {
        case B_LEAD_RISE_B:
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = B_LEAD_RISE_A;
            P1_RISE_B;
            pulse1CountB+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_B)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_B;
          }  
        break;
        case B_LEAD_RISE_A:
          pulse1State = B_LEAD_FALL_B;
          P1_RISE_A;
          pulse1CountA+=1Ul;
          pulse1LastRise = LAST_RISE_EDGE_A;
        break;
        case B_LEAD_FALL_B:
          pulse1State = B_LEAD_FALL_A;
          P1_FALL_B;
        break;
        case B_LEAD_FALL_A:
          pulse1State = B_LEAD_RISE_B;
          P1_FALL_A;
          pulse1OutFunction = pulse1OutFunctionReq; 
        break;
        default://same as B_LEAD_RISE_B
          if(pulse1Freq > 0)
          {
            OCR1A = pulse1OCRA; 
            pulse1State = B_LEAD_RISE_A;
            P1_RISE_B;
            pulse1CountB+=1Ul;
            if(pulse1LastRise == LAST_RISE_EDGE_B)
              pulse1Error++;
            pulse1LastRise = LAST_RISE_EDGE_B;
          }  
        break;
      }
     }
      else if((pulse1OutFunction == POF_A_ONLY) || (pulse1OutFunction == POF_A_ONLY_LEAK))
      {
        switch(pulse1State)
        {
          case A_LEAD_RISE_A:
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = A_LEAD_RISE_B;
              P1_RISE_A;
              pulse1CountA+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_A)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_A;
            }  
          break;
          case A_LEAD_RISE_B:
            pulse1State = A_LEAD_FALL_A;
           //no B pulse
           break;
          case A_LEAD_FALL_A:
            pulse1State = A_LEAD_FALL_B;
            P1_FALL_A;
          break;
          case A_LEAD_FALL_B:
            pulse1State = A_LEAD_RISE_A;
            //no B pulse
            pulse1OutFunction = pulse1OutFunctionReq; 
          break;
          default://same as A_LEAD_RISE_A
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = A_LEAD_RISE_B;
              P1_RISE_A;
              pulse1CountA+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_A)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_A;
            }  
          break;
        }
      }
     else if((pulse1OutFunction == POF_B_ONLY) || (pulse1OutFunction == POF_B_ONLY_LEAK))
      {
        switch(pulse1State)
        {
          case B_LEAD_RISE_B:
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = B_LEAD_RISE_A;
              P1_RISE_B;
              pulse1CountB+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_B)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_B;
            }  
          break;
          case B_LEAD_RISE_A:
            pulse1State = B_LEAD_FALL_B;
            //pulse1Error++;
            //NO A PULSE
           break;
          case B_LEAD_FALL_B:
            pulse1State = B_LEAD_FALL_A;
            P1_FALL_B;
          break;
          case B_LEAD_FALL_A:
            pulse1State = B_LEAD_RISE_B;
            //NO A PULSE
            pulse1OutFunction = pulse1OutFunctionReq; 
          break;
          default://same as B_LEAD_RISE_B
            if(pulse1Freq > 0)
            {
              OCR1A = pulse1OCRA; 
              pulse1State = B_LEAD_RISE_A;
              P1_RISE_B;
              pulse1CountB+=1Ul;
              if(pulse1LastRise == LAST_RISE_EDGE_B)
                pulse1Error++;
              pulse1LastRise = LAST_RISE_EDGE_B;
            }  
          break;
      }
     }
      
      else if(pulse1OutFunction == POF_CLEAR_COUNTS)
      {
        pulse1CountA = 0uL;
        pulse1CountB = 0uL;
        pulse1Error = 0uL;
        //pulse1LastRise = LAST_RISE_EDGE_NONE;
        pulse1OutFunction = pulse1OutFunctionReq;
      }
}  
/***************************************************************************************/
#define P2_RISE_A   PORTE |= 0x08;//set PE3 high
#define P2_RISE_B   PORTE |=0x10;//set PE4 high;
#define P2_FALL_A   PORTE &= 0xf7;//clear PE3
#define P2_FALL_B   PORTE &= 0xef;// clear PE4

ISR(TIMER3_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    if((pulse2OutFunction == POF_A_LEAD_B) || (pulse2OutFunction == POF_A_LEAD_B_LEAK))
    {
      switch(pulse2State)
      {
        case A_LEAD_RISE_A:
          if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = A_LEAD_RISE_B;
            P2_RISE_A;
            pulse2CountA+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_A)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_A;
          }  
        break;
        case A_LEAD_RISE_B:
          pulse2State = A_LEAD_FALL_A;
          P2_RISE_B;
          pulse2CountB+=1Ul;
          pulse2LastRise = LAST_RISE_EDGE_B;
        break;
        case A_LEAD_FALL_A:
          pulse2State = A_LEAD_FALL_B;
          P2_FALL_A;
        break;
        case A_LEAD_FALL_B:
          pulse2State = A_LEAD_RISE_A;
          P2_FALL_B;
          pulse2OutFunction = pulse2OutFunctionReq; 
        break;
        default://same as A_LEAD_RISE_A
         if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = A_LEAD_RISE_B;
            P2_RISE_A;
            pulse2CountA+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_A)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_A;
          }  
        break;
      }
    }
    else if((pulse2OutFunction == POF_B_LEAD_A) || (pulse2OutFunction == POF_B_LEAD_A_LEAK))
    {
      switch(pulse2State)
      {
        case B_LEAD_RISE_B:
          if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = B_LEAD_RISE_A;
            P2_RISE_B;
            pulse2CountB+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_B)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_B;
          }  
        break;
        case B_LEAD_RISE_A:
          pulse2State = B_LEAD_FALL_B;
          P2_RISE_A;
          pulse2CountA+=1Ul;
          pulse2LastRise = LAST_RISE_EDGE_A;
        break;
        case B_LEAD_FALL_B:
          pulse2State = B_LEAD_FALL_A;
          P2_FALL_B;
        break;
        case B_LEAD_FALL_A:
          pulse2State = B_LEAD_RISE_B;
          P2_FALL_A;
          pulse2OutFunction = pulse2OutFunctionReq; 
        break;
        default://same as B_LEAD_RISE_B
          if(pulse2Freq > 0)
          {
            OCR3A = pulse2OCRA; 
            pulse2State = B_LEAD_RISE_A;
            P2_RISE_B;
            pulse2CountB+=1Ul;
            if(pulse2LastRise == LAST_RISE_EDGE_B)
              pulse2Error++;
            pulse2LastRise = LAST_RISE_EDGE_B;
          }  
        break;
      }
     }
      else if((pulse2OutFunction == POF_A_ONLY) || (pulse2OutFunction == POF_A_ONLY_LEAK))
      {
        switch(pulse2State)
        {
          case A_LEAD_RISE_A:
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = A_LEAD_RISE_B;
              P2_RISE_A;
              pulse2CountA+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_A)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_A;
            }  
          break;
          case A_LEAD_RISE_B:
            pulse2State = A_LEAD_FALL_A;
           //no B pulse
           break;
          case A_LEAD_FALL_A:
            pulse2State = A_LEAD_FALL_B;
            P2_FALL_A;
          break;
          case A_LEAD_FALL_B:
            pulse2State = A_LEAD_RISE_A;
            //no B pulse
            pulse2OutFunction = pulse2OutFunctionReq; 
          break;
          default://same as A_LEAD_RISE_A
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = A_LEAD_RISE_B;
              P2_RISE_A;
              pulse2CountA+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_A)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_A;
            }  
          break;
        }
      }
     else if((pulse2OutFunction == POF_B_ONLY) || (pulse2OutFunction == POF_B_ONLY_LEAK))
      {
        switch(pulse2State)
        {
          case B_LEAD_RISE_B:
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = B_LEAD_RISE_A;
              P2_RISE_B;
              pulse2CountB+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_B)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_B;
            }  
          break;
          case B_LEAD_RISE_A:
            pulse2State = B_LEAD_FALL_B;
            //pulse2Error++;
            //NO A PULSE
           break;
          case B_LEAD_FALL_B:
            pulse2State = B_LEAD_FALL_A;
            P2_FALL_B;
          break;
          case B_LEAD_FALL_A:
            pulse2State = B_LEAD_RISE_B;
            //NO A PULSE
            pulse2OutFunction = pulse2OutFunctionReq; 
          break;
          default://same as B_LEAD_RISE_B
            if(pulse2Freq > 0)
            {
              OCR3A = pulse2OCRA; 
              pulse2State = B_LEAD_RISE_A;
              P2_RISE_B;
              pulse2CountB+=1Ul;
              if(pulse2LastRise == LAST_RISE_EDGE_B)
                pulse2Error++;
              pulse2LastRise = LAST_RISE_EDGE_B;
            }  
          break;
      }
     }
      
      else if(pulse2OutFunction == POF_CLEAR_COUNTS)
      {
        pulse2CountA = 0uL;
        pulse2CountB = 0uL;
        pulse2Error = 0uL;
        //pulse2LastRise = LAST_RISE_EDGE_NONE;
        pulse2OutFunction = pulse2OutFunctionReq;
      }
} 
/***************************************************************************************/
#define P3_RISE_A  PORTH |=0x08;//set PH3 high(;   
#define P3_RISE_B  PORTH |= 0x10;//set PH4 high 
#define P3_FALL_A  PORTH &= 0xf7;// clear PH3 
#define P3_FALL_B  PORTH &= 0xef;//clear PH4 

ISR(TIMER4_COMPA_vect)  
{
    //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    if((pulse3OutFunction == POF_A_LEAD_B) || (pulse3OutFunction == POF_A_LEAD_B_LEAK))
    {
      switch(pulse3State)
      {
        case A_LEAD_RISE_A:
          if(pulse3Freq > 0)
          {
            OCR4A = pulse3OCRA; 
            pulse3State = A_LEAD_RISE_B;
            P3_RISE_A;
            pulse3CountA+=1Ul;
            if(pulse3LastRise == LAST_RISE_EDGE_A)
              pulse3Error++;
            pulse3LastRise = LAST_RISE_EDGE_A;
          }  
        break;
        case A_LEAD_RISE_B:
          pulse3State = A_LEAD_FALL_A;
          P3_RISE_B;
          pulse3CountB+=1Ul;
          pulse3LastRise = LAST_RISE_EDGE_B;
        break;
        case A_LEAD_FALL_A:
          pulse3State = A_LEAD_FALL_B;
          P3_FALL_A;
        break;
        case A_LEAD_FALL_B:
          pulse3State = A_LEAD_RISE_A;
          P3_FALL_B;
          pulse3OutFunction = pulse3OutFunctionReq; 
        break;
        default://same as A_LEAD_RISE_A
         if(pulse3Freq > 0)
          {
            OCR4A = pulse3OCRA; 
            pulse3State = A_LEAD_RISE_B;
            P3_RISE_A;
            pulse3CountA+=1Ul;
            if(pulse3LastRise == LAST_RISE_EDGE_A)
              pulse3Error++;
            pulse3LastRise = LAST_RISE_EDGE_A;
          }  
        break;
      }
    }
    else if((pulse3OutFunction == POF_B_LEAD_A) || (pulse3OutFunction == POF_B_LEAD_A_LEAK))
    {
      switch(pulse3State)
      {
        case B_LEAD_RISE_B:
          if(pulse3Freq > 0)
          {
            OCR4A = pulse3OCRA; 
            pulse3State = B_LEAD_RISE_A;
            P3_RISE_B;
            pulse3CountB+=1Ul;
            if(pulse3LastRise == LAST_RISE_EDGE_B)
              pulse3Error++;
            pulse3LastRise = LAST_RISE_EDGE_B;
          }  
        break;
        case B_LEAD_RISE_A:
          pulse3State = B_LEAD_FALL_B;
          P3_RISE_A;
          pulse3CountA+=1Ul;
          pulse3LastRise = LAST_RISE_EDGE_A;
        break;
        case B_LEAD_FALL_B:
          pulse3State = B_LEAD_FALL_A;
          P3_FALL_B;
        break;
        case B_LEAD_FALL_A:
          pulse3State = B_LEAD_RISE_B;
          P3_FALL_A;
          pulse3OutFunction = pulse3OutFunctionReq; 
        break;
        default://same as B_LEAD_RISE_B
          if(pulse3Freq > 0)
          {
            OCR4A = pulse3OCRA; 
            pulse3State = B_LEAD_RISE_A;
            P3_RISE_B;
            pulse3CountB+=1Ul;
            if(pulse3LastRise == LAST_RISE_EDGE_B)
              pulse3Error++;
            pulse3LastRise = LAST_RISE_EDGE_B;
          }  
        break;
      }
     }
      else if((pulse3OutFunction == POF_A_ONLY) || (pulse3OutFunction == POF_A_ONLY_LEAK))
      {
        switch(pulse3State)
        {
          case A_LEAD_RISE_A:
            if(pulse3Freq > 0)
            {
              OCR4A = pulse3OCRA; 
              pulse3State = A_LEAD_RISE_B;
              P3_RISE_A;
              pulse3CountA+=1Ul;
              if(pulse3LastRise == LAST_RISE_EDGE_A)
                pulse3Error++;
              pulse3LastRise = LAST_RISE_EDGE_A;
            }  
          break;
          case A_LEAD_RISE_B:
            pulse3State = A_LEAD_FALL_A;
           //no B pulse
           break;
          case A_LEAD_FALL_A:
            pulse3State = A_LEAD_FALL_B;
            P3_FALL_A;
          break;
          case A_LEAD_FALL_B:
            pulse3State = A_LEAD_RISE_A;
            //no B pulse
            pulse3OutFunction = pulse3OutFunctionReq; 
          break;
          default://same as A_LEAD_RISE_A
            if(pulse3Freq > 0)
            {
              OCR4A = pulse3OCRA; 
              pulse3State = A_LEAD_RISE_B;
              P3_RISE_A;
              pulse3CountA+=1Ul;
              if(pulse3LastRise == LAST_RISE_EDGE_A)
                pulse3Error++;
              pulse3LastRise = LAST_RISE_EDGE_A;
            }  
          break;
        }
      }
     else if((pulse3OutFunction == POF_B_ONLY) || (pulse3OutFunction == POF_B_ONLY_LEAK))
      {
        switch(pulse3State)
        {
          case B_LEAD_RISE_B:
            if(pulse3Freq > 0)
            {
              OCR4A = pulse3OCRA; 
              pulse3State = B_LEAD_RISE_A;
              P3_RISE_B;
              pulse3CountB+=1Ul;
              if(pulse3LastRise == LAST_RISE_EDGE_B)
                pulse3Error++;
              pulse3LastRise = LAST_RISE_EDGE_B;
            }  
          break;
          case B_LEAD_RISE_A:
            pulse3State = B_LEAD_FALL_B;
            //pulse3Error++;
            //NO A PULSE
           break;
          case B_LEAD_FALL_B:
            pulse3State = B_LEAD_FALL_A;
            P3_FALL_B;
          break;
          case B_LEAD_FALL_A:
            pulse3State = B_LEAD_RISE_B;
            //NO A PULSE
            pulse3OutFunction = pulse3OutFunctionReq; 
          break;
          default://same as B_LEAD_RISE_B
            if(pulse3Freq > 0)
            {
              OCR4A = pulse3OCRA; 
              pulse3State = B_LEAD_RISE_A;
              P3_RISE_B;
              pulse3CountB+=1Ul;
              if(pulse3LastRise == LAST_RISE_EDGE_B)
                pulse3Error++;
              pulse3LastRise = LAST_RISE_EDGE_B;
            }  
          break;
      }
     }
      
      else if(pulse3OutFunction == POF_CLEAR_COUNTS)
      {
        pulse3CountA = 0uL;
        pulse3CountB = 0uL;
        pulse3Error = 0uL;
        //pulse3LastRise = LAST_RISE_EDGE_NONE;
        pulse3OutFunction = pulse3OutFunctionReq;
      }
}  
/****************************************************************************************/

#define P4_RISE_A    PORTL |=0x10;//set PL4 high(;
#define P4_RISE_B    PORTL |= 0x08;//set PL3 high
#define P4_FALL_A    PORTL &= 0xef;// clear PL4
#define P4_FALL_B    PORTL &= 0xf7;//clear PL3

ISR(TIMER5_COMPA_vect)  
{
   //static int local_pulse_delay;   
  //interrupt flag automatically cleared upon interrupt execution
    if((pulse4OutFunction == POF_A_LEAD_B) || (pulse4OutFunction == POF_A_LEAD_B_LEAK))
    {
      switch(pulse4State)
      {
        case A_LEAD_RISE_A:
          if(pulse4Freq > 0)
          {
            OCR5A = pulse4OCRA; 
            pulse4State = A_LEAD_RISE_B;
            P4_RISE_A;
            pulse4CountA+=1Ul;
            if(pulse4LastRise == LAST_RISE_EDGE_A)
              pulse4Error++;
            pulse4LastRise = LAST_RISE_EDGE_A;
          }  
        break;
        case A_LEAD_RISE_B:
          pulse4State = A_LEAD_FALL_A;
          P4_RISE_B;
          pulse4CountB+=1Ul;
          pulse4LastRise = LAST_RISE_EDGE_B;
        break;
        case A_LEAD_FALL_A:
          pulse4State = A_LEAD_FALL_B;
          P4_FALL_A;
        break;
        case A_LEAD_FALL_B:
          pulse4State = A_LEAD_RISE_A;
          P4_FALL_B;
          pulse4OutFunction = pulse4OutFunctionReq; 
        break;
        default://same as A_LEAD_RISE_A
         if(pulse4Freq > 0)
          {
            OCR5A = pulse4OCRA; 
            pulse4State = A_LEAD_RISE_B;
            P4_RISE_A;
            pulse4CountA+=1Ul;
            if(pulse4LastRise == LAST_RISE_EDGE_A)
              pulse4Error++;
            pulse4LastRise = LAST_RISE_EDGE_A;
          }  
        break;
      }
    }
    else if((pulse4OutFunction == POF_B_LEAD_A) || (pulse4OutFunction == POF_B_LEAD_A_LEAK))
    {
      switch(pulse4State)
      {
        case B_LEAD_RISE_B:
          if(pulse4Freq > 0)
          {
            OCR5A = pulse4OCRA; 
            pulse4State = B_LEAD_RISE_A;
            P4_RISE_B;
            pulse4CountB+=1Ul;
            if(pulse4LastRise == LAST_RISE_EDGE_B)
              pulse4Error++;
            pulse4LastRise = LAST_RISE_EDGE_B;
          }  
        break;
        case B_LEAD_RISE_A:
          pulse4State = B_LEAD_FALL_B;
          P4_RISE_A;
          pulse4CountA+=1Ul;
          pulse4LastRise = LAST_RISE_EDGE_A;
        break;
        case B_LEAD_FALL_B:
          pulse4State = B_LEAD_FALL_A;
          P4_FALL_B;
        break;
        case B_LEAD_FALL_A:
          pulse4State = B_LEAD_RISE_B;
          P4_FALL_A;
          pulse4OutFunction = pulse4OutFunctionReq; 
        break;
        default://same as B_LEAD_RISE_B
          if(pulse4Freq > 0)
          {
            OCR5A = pulse4OCRA; 
            pulse4State = B_LEAD_RISE_A;
            P4_RISE_B;
            pulse4CountB+=1Ul;
            if(pulse4LastRise == LAST_RISE_EDGE_B)
              pulse4Error++;
            pulse4LastRise = LAST_RISE_EDGE_B;
          }  
        break;
      }
     }
      else if((pulse4OutFunction == POF_A_ONLY) || (pulse4OutFunction == POF_A_ONLY_LEAK))
      {
        switch(pulse4State)
        {
          case A_LEAD_RISE_A:
            if(pulse4Freq > 0)
            {
              OCR5A = pulse4OCRA; 
              pulse4State = A_LEAD_RISE_B;
              P4_RISE_A;
              pulse4CountA+=1Ul;
              if(pulse4LastRise == LAST_RISE_EDGE_A)
                pulse4Error++;
              pulse4LastRise = LAST_RISE_EDGE_A;
            }  
          break;
          case A_LEAD_RISE_B:
            pulse4State = A_LEAD_FALL_A;
           //no B pulse
           break;
          case A_LEAD_FALL_A:
            pulse4State = A_LEAD_FALL_B;
            P4_FALL_A;
          break;
          case A_LEAD_FALL_B:
            pulse4State = A_LEAD_RISE_A;
            //no B pulse
            pulse4OutFunction = pulse4OutFunctionReq; 
          break;
          default://same as A_LEAD_RISE_A
            if(pulse4Freq > 0)
            {
              OCR5A = pulse4OCRA; 
              pulse4State = A_LEAD_RISE_B;
              P4_RISE_A;
              pulse4CountA+=1Ul;
              if(pulse4LastRise == LAST_RISE_EDGE_A)
                pulse4Error++;
              pulse4LastRise = LAST_RISE_EDGE_A;
            }  
          break;
        }
      }
     else if((pulse4OutFunction == POF_B_ONLY) || (pulse4OutFunction == POF_B_ONLY_LEAK))
      {
        switch(pulse4State)
        {
          case B_LEAD_RISE_B:
            if(pulse4Freq > 0)
            {
              OCR5A = pulse4OCRA; 
              pulse4State = B_LEAD_RISE_A;
              P4_RISE_B;
              pulse4CountB+=1Ul;
              if(pulse4LastRise == LAST_RISE_EDGE_B)
                pulse4Error++;
              pulse4LastRise = LAST_RISE_EDGE_B;
            }  
          break;
          case B_LEAD_RISE_A:
            pulse4State = B_LEAD_FALL_B;
            //pulse4Error++;
            //NO A PULSE
           break;
          case B_LEAD_FALL_B:
            pulse4State = B_LEAD_FALL_A;
            P4_FALL_B;
          break;
          case B_LEAD_FALL_A:
            pulse4State = B_LEAD_RISE_B;
            //NO A PULSE
            pulse4OutFunction = pulse4OutFunctionReq; 
          break;
          default://same as B_LEAD_RISE_B
            if(pulse4Freq > 0)
            {
              OCR5A = pulse4OCRA; 
              pulse4State = B_LEAD_RISE_A;
              P4_RISE_B;
              pulse4CountB+=1Ul;
              if(pulse4LastRise == LAST_RISE_EDGE_B)
                pulse4Error++;
              pulse4LastRise = LAST_RISE_EDGE_B;
            }  
          break;
      }
     }
      
      else if(pulse4OutFunction == POF_CLEAR_COUNTS)
      {
        pulse4CountA = 0uL;
        pulse4CountB = 0uL;
        pulse4Error = 0uL;
        //pulse4LastRise = LAST_RISE_EDGE_NONE;
        pulse4OutFunction = pulse4OutFunctionReq;
      }
}    
              


