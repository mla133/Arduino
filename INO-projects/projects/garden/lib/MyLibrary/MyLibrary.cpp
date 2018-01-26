#include "MyLibrary.h"

void printDouble( double val, byte precision){
  // prints val with number of decimal places determine by precision
  // precision is a number from 0 to 6 indicating the desired decimial places
  // example: printDouble( 3.1415, 2); // prints 3.14 (two decimal places)

  Serial.print (int(val));  //prints the int part
  if( precision > 0) {
    Serial.print("."); // print the decimal point
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision -1;
    while(precision--)
       mult *=10;
       
    if(val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val)- val ) * mult;
    unsigned long frac1 = frac;
    while( frac1 /= 10 )
      padding--;
    while(  padding--)
      Serial.print("0");
    Serial.print(frac,DEC) ;
  }
}

char* numToAscii(double num)
{
  // Takes a floating or double floating point number in, returns an ascii string with up to 1/1000 of precision.
  char ascii[32];
  int frac;
  frac=(unsigned int)(num*1000)%1000;  //get three numbers to right of decimal point
  itoa((int)num,ascii,10);
  strcat(ascii,".");
  itoa(frac,&ascii[strlen(ascii)],10);  //put the frac after the decimal
  return ascii;
}
