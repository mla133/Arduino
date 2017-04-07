/*
  This is Program to implement three dgital filters:
  (1) 200Hz-600Hz
  (2) 600Hz-1200Hz
  (3) 1200Hz-2400Hz
  The output can be verified using a function generator and reading relative magnitudes of output on Serial Monitor 
  and on Oscilloscope if you have DACs 
*/
#define M 50
#define N 65
#include<avr/io.h>
#include<avr/delay.h>
#include <util/delay.h>
uint16_t x[N];                    //Samples

// Filter coefficiants to implement Three Digital Filters
int32_t b1[N]={1,0,-3,-5,-7,-8,-5,0,8,17,25,28,23,6,-25,-72,-134,-205,-280,-349,-401,-425,-413,-360,-262,-124,44,231,418,589,725,813,844,813,725,589,418,231,44,-124,-262,-360,-413,-425,-401,-349,-280,-205,-134,-72,-25,6,23,28,25,17,8,0,-5,-8,-7,-5,-3,0,1};
int32_t b2[N]={-3,-2,3,12,22,27,20,0,-30,-58,-69,-55,-21,14,29,11,-25,-45,-9,101,256,382,388,214,-128,-538,-855,-923,-668,-139,493,1001,1196,1001,493,-139,-668,-923,-855,-538,-128,214,388,382,256,101,-9,-45,-25,11,29,14,-21,-55,-69,-58,-30,0,20,27,22,12,3,-2,-3};
int32_t b3[N]={0,11,15,-4,-23,-14,6,0,-9,30,72,18,-89,-92,5,22,-39,45,240,174,-200,-368,-92,107,-80,-25,662,929,-288,-1827,-1380,998,2406,998,-1380,-1827,-288,929,662,-25,-80,107,-92,-368,-200,174,240,45,-39,22,5,-92,-89,18,72,30,-9,0,6,-14,-23,-4,15,11,0};

uint16_t a;
uint16_t i;

void InitADC()                    // Function to initialise ADC Channel
{
ADMUX=(1<<REFS0); 
ADMUX&=~((1<<REFS1)|(1<<ADLAR)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0));                      
ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); 
}

uint16_t ReadADC(uint8_t ch)      // Function to read value of ADC
{
   ch=ch&0b00000111;
   ADMUX|=ch;
   ADCSRA|=(1<<ADSC);
   while(!(ADCSRA & (1<<ADIF)));
   //_delay_ms(1000);
   ADCSRA|=(1<<ADIF);
   return(ADC);
}
void setup()          //Setup Function to intialise ADC and Serial Communication
{
DDRC=0x00;            //PORTC as INPUT
DDRD=0xFF;            //PORTD is OUTPUT
DDRD=0xFF;
for(i=0;i<N;i++)
  {
    InitADC();
    a=ReadADC(4);
    x[i]=a;
  }
  Serial.begin(9600);
}

int32_t num1=0,num2=0,num3=0,y1[M],y2[M],y3[M],sum1=0,sum2=0,sum3=0,ans1=0,ans2=0,ans3=0;    // Various variables to determine Y-OUTPUT

void loop()            //Loop Function
{
  for(int i=0;i<M;i++)
  {
    num1=0;                //cumulative summation of filter1
    num2=0;                //cumulative summation of filter2
    num3=0;                //cumulative summation of filter3
    for(int j=0;j<N;j++)    // taking Samples
    {
      InitADC();
      x[j]=ReadADC(4);
    }
    for(int k=0;k<N;k++)   //Convolution
    {
      num1+=b1[k]*x[N-1-k];        //Filter1
      num2+=b2[k]*x[N-1-k];        //Filter2
      num3+=b3[k]*x[N-1-k];        //Filter3
    }
    
    y1[i]=num1/10000;              //Normalisation
    y2[i]=num2/10000;
    y3[i]=num3/10000;    
  }
    
  //Now take RMS values
  for(int i=0;i<M;i++)
  {
    sum1+=y1[i]*y1[i];
    sum2+=y2[i]*y2[i];
    sum3+=y3[i]*y3[i];
  }
  sum1/=M;
  sum2/=M;
  sum3/=M;
  ans1=sqrt(sum1);
  ans2=sqrt(sum2);
  ans3=sqrt(sum3);
  
  // Printing them on Serial Monitor
  Serial.print("200-600Hz::");
  Serial.println(ans1);
  Serial.print("600-1200Hz::");
  Serial.println(ans2);
  Serial.print("1200-2400Hz::");
  Serial.println(ans3);
  
  //Checking the OUTPUT on Oscilloscope
  dacx(ans1);
  dacy(ans2);
}

void dacx(unsigned char a)            // Function to write values on one of the DACs DACx
{
  byte k=0;
   for(unsigned char i=0;i<2;i++)
    {
      k=a&(1<<i);
      if(k)
      PORTB|=(1<<i);
      else
      PORTB&=~(1<<i);      
    }
   for(unsigned char i=0;i<6;i++)
    {
      k=a&(1<<(i+2));
      if(k)
      PORTD|=(1<<(7-i));
      else
      PORTD&=~(1<<(7-i));     
    }
} 

void dacy(unsigned char a)              // Function to write values on one of the DACs DACy
{
  byte k=0;
  for(unsigned char i=0;i<4;i++)
    {
      k=a&(1<<i);
      if(k)
      PORTC|=(1<<(3-i));
      else
      PORTC&=~(1<<(3-i));     
    }
  for(unsigned char i=0;i<4;i++)
    {
      k=a&(1<<(i+4));
      if(k)
      PORTB|=(1<<(5-i));
      else
      PORTB&=~(1<<(5-i));
    }
}
