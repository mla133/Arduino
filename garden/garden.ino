/***************************************************************
*   Garden Station                                             *                              
*   written by Matt Allen, Feb 20th, 2013                      *
*   revision: 1.1, 01/29/16				       * 
*   Added 8x8 LED display/functions, added debug code          *
*   to shut off features as needed (see #defines)              *
*   Added millis() timers to replace delay() in the looping    *
*   functions (local to the functions).                        *
*   01/29/16 - Added new TFTv2 display and revamped for ino    *
***************************************************************/  

/*  draw number's APP
    drawNumber(long long_num,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
    drawFloat(float floatNumber,INT8U decimal,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
    drawFloat(float floatNumber,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);

    draw text's APP
    drawChar(INT8U ascii,INT16U poX, INT16U poY,INT16U size, INT16U fgcolor);
    drawString(char *string,INT16U poX, INT16U poY,INT16U size,INT16U fgcolor);
*/ 


#include <stdint.h>
#include <TFTv2.h>
#include <SPI.h>

// This is a cool template class that lets me get away with cout << like statements - see PrintSerial()
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

const int low_moisture_alarm = 10;
const int pump_indicator = 12;
const int low_light_alarm = 11;
const int heartbeat_indicator = 13;
const int light_sensor = A0;
const int soil_sensor  = A1;
const int temp_sensor  = A2;
const int relayPin     = 3;

int soilValue;
int tempValue;
int lightValue;
int dry_soil = 256;          //Threshhold value for dry soil (need to determine this 4/23/13)
long interval = 1000;       //60,000 for one minute, 2 for testing against simulation
long previousMillis = 0;
int state = 0;               //what is state of machine? 0=timer/waiting, 1=check_io, 2=activate_pump
int pump_state = 0;
long elapsedTime;          //How many seconds have gone by since start?
unsigned long currentMillis;
int oldlight,oldtemp,oldsoil;

unsigned long pump_timer, display_timer;

void setup() {

  TFT_BL_ON;      // turn on the background light
  Tft.TFTinit();  // init TFT library

  Serial.begin(9600);
  pinMode(low_moisture_alarm, OUTPUT);
  pinMode(low_light_alarm, OUTPUT);
  pinMode(pump_indicator, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(heartbeat_indicator, OUTPUT);
  
  Serial.println("Booting Garden Station...");
  Serial.print("Setting interval checks to: ");
  Serial.print(interval/1000);
  Serial.println(" sec ");
}

/******************************
*  MAIN LOOP RIGHT HERE!!     * 
******************************/    
void loop() 
{
  currentMillis = millis();  
  CheckTimers();        // Active when state = 0
  CheckIO();            // Active when state = 1
  ActivatePump();       // Active when state = 2
}



/******************************
*  Function Declarations      * 
******************************/

/******************************/
void CheckTimers()
{
  // I/O Sensor Timer (10,000 ms interval)
  if(currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    state= 1;                        //time to check sensors
    digitalWrite(heartbeat_indicator, HIGH);
    elapsedTime = int(currentMillis / 1000);   //how many seconds have gone by?
  }
}

/******************************/
void CheckIO()
{
  if (state== 1)
  {
    state= 0;                        //reset timer
    lightValue = analogRead(light_sensor);
    soilValue  = analogRead(soil_sensor);
    tempValue  = analogRead(temp_sensor);

    if (soilValue >= dry_soil)
    {
      digitalWrite(low_moisture_alarm, HIGH);
      Serial.println("Soil Dry - Activating Pump");
      state = 2;                      //Set status to activate pump
    } 
    else 
      digitalWrite(low_moisture_alarm, LOW); 

    if (lightValue < 200)
    {
      digitalWrite(low_light_alarm,HIGH);
      Serial.println("Low Light - Trip Alarm");
    }
    else 
      digitalWrite(low_light_alarm,LOW);

    digitalWrite(heartbeat_indicator, LOW);

    PrintSerial();        // Let's send an update to serial
    PrintDisplay();	  // Let's update the TFT display
  } 

}

/******************************/
void ActivatePump()
{
  if (state == 2)
  {
    pump_state = 1;
    pump_timer = millis();
    digitalWrite(relayPin, HIGH);   //activate pump
    digitalWrite(pump_indicator, pump_state);
    Serial.println("Pump Activated");                
    if (millis() - pump_timer >= 5000UL)    //run for 5 seconds (or more, need to get pump to check how long it takes to prime up)
    {
      pump_state = 0;
      digitalWrite(relayPin, LOW);    //shut off pump
      digitalWrite(pump_indicator, pump_state);
      Serial.println("Pump De-Activated");
    }
    //state = 0;                       //set status to check again on next pass of timer
  }
}
    
/******************************/
void PrintSerial()
{
      Serial << "Time: " << elapsedTime << " Light: " << lightValue << " Temp: " << tempValue << " Soil: " << soilValue << " PUMP: " << pump_state;
      Serial.println();
}


/******************************/
void PrintDisplay()
{

//	Tft.fillRectangle(100,30,100,30,BLACK); 		// 3x25=75 for length, 30 for size 4
//	Tft.fillRectangle(100,60,100,30,BLACK); 		// 3x25=75 for length, 30 for size 4
//	Tft.fillRectangle(100,90,100,30,BLACK); 		// 3x25=75 for length, 30 for size 4
//	Tft.fillRectangle(100,150,100,30,BLACK); 		// 3x25=75 for length, 30 for size 4
	Tft.drawString("LGT:", 0, 30, 4, WHITE);
	Tft.drawString("TMP:", 0, 60, 4, WHITE);
	Tft.drawString("SOL:", 0, 90, 4, WHITE);


	Tft.drawNumber(oldlight, 100, 30, 4, BLACK); 
	Tft.drawNumber(oldtemp, 100, 60, 4, BLACK); 
	Tft.drawNumber(oldsoil, 100, 90, 4, BLACK); 

	Tft.drawNumber(lightValue, 100, 30, 4, YELLOW); 
	Tft.drawNumber(tempValue, 100, 60, 4, YELLOW); 
	Tft.drawNumber(soilValue, 100, 90, 4, YELLOW); 

  	oldlight = lightValue;
  	oldtemp  = tempValue;
  	oldsoil  = soilValue;


}     

/******************************/
void NotifyDisplay(char *msg, int line, int color)
{
	int line_offset = line*30 + 150;
	display_timer = millis();
 	if(millis() - display_timer >= 5000UL)
	{	
	  Tft.fillRectangle(0,line_offset,240,30,BLACK); 		// 3x25=75 for length, 30 for size 4
	  Tft.drawString(msg, 0, line_offset, 4, color);
	}
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
