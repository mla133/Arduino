/***************************************************************
*   Garden Station                                             *                              
*   written by Matt Allen, Feb 20th, 2013                      *
*   revision: 0.5, Apr 25th, 2013                              *
*   Added 8x8 LED display/functions, added debug code          *
*   to shut off features as needed (see #defines)              *
*   Added millis() timers to replace delay() in the looping    *
*   functions (local to the functions).                        *
***************************************************************/   

// This is a cool template class that lets me get away with cout << like statements - see PrintSerial()
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

const int low_moisture_alarm = 13;
const int pump_indicator = 12;
const int low_light_alarm = 11;
const int light_sensor = A0;
const int soil_sensor  = A1;
const int temp_sensor  = A2;
const int relayPin     = 3;

#define USE_LED_BKPK 1         //Uncomment to turn on this feature (for debugging)
//#define USE_LCD 1         //Uncomment to turn on this feature (for debugging)
//#define SHOW_STARTUP_SEQUENCE 1
//#define ENABLE_SIMULATOR 1  //Enable debugging via simulator

int soilValue;
int tempValue;
int lightValue;
#ifdef USE_LCD
const int TxPin = 6;        //D6 Tx pin for LCD 2x16
#endif
#ifdef USE_LED_BKPK
const int LED_CLK = 5;       //D5 pin for 8x8 matrix (CLK)
const int LED_DAT = 4;       //D4 pin for 8x8 matrix (DAT)
#endif
int dry_soil = 512;          //Threshhold value for dry soil (need to determine this 4/23/13)
long interval = 10000;       //60,000 for one minute, 2 for testing against simulation
long previousMillis = 0;
int flag = 0;               //what is state of machine? 0=timer/waiting, 1=check_io, 2=activate_pump
long elapsedTime;          //How many seconds have gone by since start?
unsigned long currentMillis;

#ifdef USE_LCD
#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(255,TxPin);
#endif
#ifdef USE_LED_BKPK
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();   //might as well add in the 8x8 LED...
#endif

void setup() {
  Serial.begin(9600);
  pinMode(low_moisture_alarm, OUTPUT);
  pinMode(low_light_alarm, OUTPUT);
  pinMode(pump_indicator, OUTPUT);
  pinMode(relayPin, OUTPUT);
  
  Serial.println("Booting Garden Station...");
  
#ifdef USE_LCD
  pinMode(TxPin, OUTPUT);
#endif
#ifdef USE_LED_BKPK
  pinMode(LED_CLK, OUTPUT);
  pinMode(LED_DAT, OUTPUT);
  
/******************************
*  Firing up the 8x8 LED      * 
******************************/ 
  startupSequence();
  matrix.begin(0x70);
#endif  
  
/******************************
*  Firing up the LCD display  * 
******************************/
#ifdef USE_LCD
  digitalWrite(TxPin, HIGH);
  mySerial.begin(9600);
  delay(100);           //Need to wait for LCD...
  mySerial.write(12);   //Clear LCD
  mySerial.write(17);   //Turn backlight ON
  delay(5);             //Required delay for LCD
  mySerial.print("Garden Station Booting...");
  mySerial.write(13);   //Form Feed
  delay(500);
  mySerial.write(18);   // Turn backlight off
#endif
 Serial.println("Done booting!");
}
#ifdef USE_LED_BKPK
static uint8_t PROGMEM
  smile_bmp[] =
  { B00111100, B01000010, B10100101, B10000001, B10100101, B10011001, B01000010, B00111100 },
  neutral_bmp[] =
  { B00111100, B01000010, B10100101, B10000001, B10111101, B10000001, B01000010, B00111100 },
  frown_bmp[] =
  { B00111100, B01000010, B10100101, B10000001, B10011001, B10100101, B01000010, B00111100 };
#endif
 

/******************************
*  MAIN LOOP RIGHT HERE!!     * 
******************************/    
void loop() 
{
  currentMillis = millis();  
  CheckTimer();         // Active when flag = 0
  CheckIO();            // Active when flag = 1
  ActivatePump();       // Active when flag = 2
  
}



/******************************
*  Function Declarations      * 
******************************/

void CheckTimer()
{
  if(currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    flag = 1;                        //time to check sensors
    elapsedTime = int(currentMillis / 1000);   //how many seconds have gone by?
  }
}

/******************************/
void CheckIO()
{
  if (flag == 1)
  {
    flag = 0;                        //reset timer
    lightValue = analogRead(light_sensor);
    soilValue  = analogRead(soil_sensor);
    tempValue  = analogRead(temp_sensor);
    
    if (soilValue >= dry_soil)
    {
      digitalWrite(low_moisture_alarm, HIGH);
      Serial.println("Soil Dry - Activating Pump");
      flag = 2;                      //Set status to activate pump
      ShowDisplayStatus(flag);
    } 
    else digitalWrite(low_moisture_alarm, LOW); 
    if (lightValue < 200)
    {
      digitalWrite(low_light_alarm,HIGH);
      Serial.println("Low Light - Trip Alarm");
      ShowDisplayStatus(1);
    }
    else digitalWrite(low_light_alarm,LOW);
    PrintSerial();                  // Let's send an update to serial
    PrintLCD();                     // Update sent to LCD screen  
    ShowSoilLED();
  } 

}

/******************************/
void ActivatePump()
{
  unsigned long timer;
  if (flag == 2)
  {
    timer = millis();
    digitalWrite(relayPin, HIGH);   //activate pump
    digitalWrite(pump_indicator, HIGH);
    Serial.println("Pump Activated");                
    if (millis() - timer >= 5000UL)    //run for 5 seconds (or more, need to get pump to check how long it takes to prime up)
    {
      digitalWrite(relayPin, LOW);    //shut off pump
      digitalWrite(pump_indicator, LOW);
      Serial.println("Pump De-Activated");
    }
    flag = 0;                       //set status to check again on next pass of timer
  }
}
    
/******************************/
void PrintSerial()
{
      Serial << "Time: " << elapsedTime << " Light: " << lightValue << " Temp: " << tempValue << " Soil: ";
      Serial.println(soilValue);
}    

/******************************/
void PrintLCD()
{
#ifdef USE_LCD
  unsigned long timer5;
  unsigned long timer500;
  mySerial.write(17);   //Turn backlight ON
  mySerial.write(12);   //Clear previous display
  
  if (millis() - timer5 >= 5UL)   //Required delay for LCD
  {
    mySerial.print("Garden Station");
    mySerial.write(13);   //Form Feed
    mySerial.print("L: ");
    mySerial.print(lightValue);
    mySerial.print("M: ");
    mySerial.print(soilValue);
    mySerial.print("T: ");
    mySerial.print(tempValue);
  }
  if (millis() - timer500 >= 500UL) mySerial.write(18);   // Turn backlight off
#endif
}

/******************************/
void ShowDisplayPixel(int x, int y)
{
  if (flag == 3)
  {
#ifdef USE_LED_BKPK
  matrix.clear();      // clear display
  matrix.drawPixel(x, y, LED_ON);  
  matrix.writeDisplay();  // write the changes we just made to the display
#endif
  }
  flag = 0;
}

/******************************/
void ShowDisplayStatus(int face)
{
#ifdef USE_LED_BKPK
  matrix.clear();
  if (face == 1) matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
  if (face == 2) matrix.drawBitmap(0, 0, neutral_bmp, 8, 8, LED_ON);
  if (face == 3) matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_ON);
  matrix.writeDisplay();
#endif
  flag = 0;
}

void ShowSoilLED()
{
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  for (int8_t x=0; x>=-36; x--) {
    matrix.clear();
    matrix.setCursor(x,0);
//    matrix.print("Hello");
    matrix << "S: " << soilValue;  // cool cout way :)
    matrix.writeDisplay();
    delay(100);
  }
}

/******************************/
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

/******************************/
void startupSequence() {
  // shows a neat startup sequence on the 8x8 matrix backpack (if enabled)
  // may use for scaring off pests (maybe)
#ifdef SHOW_STARTUP_SEQUENCE
  unsigned long timer1;
  unsigned long timer2;
  
  for (int row=0; row<8; row++) 
  {
    if (row%2 == 0) 
    {
      for (int col=0; col<8; col++) 
      {
        matrix.drawPixel(row, col, LED_ON);
        if(millis() - timer1 >= 25UL) matrix.drawPixel(row, col, LED_OFF);
      }
    } else 
    {
      for (int col=8-1; col>=0; col--) 
      {
        matrix.drawPixel(row, col, LED_ON);
        if(millis() - timer2 >= 25UL) matrix.drawPixel(row, col, LED_OFF);
      }
    }
  }
#endif
}

/******************************
*  Extraneous Info Below      * 
******************************/

/*    LIGHT SENSOR
      PR = Photoresistor range (82-995 values in ambient fluorescent: 82 illuminate, 995 covered), hand shadow triggers ~200ish

        	/-------------- A0
        	|
	        |
        /-------x--[10Kohm]---- Vcc (5v)
        |	| +
        # PR	% 0.1 uF
        |	| -
        \-------x
	        |
               GND   
               digitalWrite(light_sensor_alarm, LOW);
    
*/

/****  NOTE:  LCD hooked to +5V, GND, and pin D6 for Tx ****/
