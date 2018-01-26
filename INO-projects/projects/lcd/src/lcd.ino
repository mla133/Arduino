#include <SoftwareSerial.h>
const int TxPin = 6;
const int linelength = 16;
int reading, reading2, reading3 = 0;

SoftwareSerial lcd = SoftwareSerial(255, TxPin);
char lineBuffer[linelength];
unsigned long timer = 0;

char* numToAscii(double num)
{
	// Takes a double floating point number in, returns an ascii string with up to 1/1000 precision
	char ascii[32];
	int frac;
	frac=(unsigned int)(num*1000)%1000;  // get 3 numbers to the right of the decimal point
	itoa((int)num,ascii,10);
	strcat(ascii,".");
	itoa(frac,&ascii[strlen(ascii)],10);  // put frac after decimal
	return ascii;
}

void setup() 
{
	pinMode(TxPin, OUTPUT);
	digitalWrite(TxPin, HIGH);
	lcd.begin(9600);
	delay(100);
	lcd.write(12); // Clear
	lcd.write(17); // Turn backlight on
	delay(5); // Required delay
	lcd.print("Hello, world..."); // First line
	lcd.write(13); // Form feed
	lcd.print("73 DE KC3EYS"); // Second line
//	lcd.write(212); // Quarter note
//	lcd.write(220); // A tone
	delay(3000); // Wait 3 seconds
	lcd.write(18); // Turn backlight off
	timer = millis();
}
void loop() 
{
	if(millis() - timer >=500UL)
	{
		lcd.write(128);
		
		reading = random(25, 75);  // Random number test
		reading2 = random(25, 75);  // Random number test
		reading3 = random(25, 75);  // Random number test
		lcd.write(12);
		delay(5);
		lcd.print("   SWR METER   ");	
		lcd.write(13);	//Form Feed
		lcd.print("S:");
		lcd.print(reading);
		lcd.print(" F:");
		lcd.print(reading2);
		lcd.print(" R:");
		lcd.print(reading3);
		timer = millis();
	}
}
