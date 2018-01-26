#include <stdint.h>
#include <TouchScreen.h> 
#include <TFT.h>

#ifdef SEEEDUINO
  #define YP A2   // must be an analog pin, use "An" notation!
  #define XM A1   // must be an analog pin, use "An" notation!
  #define YM 14   // can be a digital pin, this is A0
  #define XP 17   // can be a digital pin, this is A3 
#endif

#ifdef MEGA
  #define YP A2   // must be an analog pin, use "An" notation!
  #define XM A1   // must be an analog pin, use "An" notation!
  #define YM 54   // can be a digital pin, this is A0
  #define XP 57   // can be a digital pin, this is A3 
#endif 

//Measured ADC values for (0,0) and (210-1,320-1)
//TS_MINX corresponds to ADC value when X = 0
//TS_MINY corresponds to ADC value when Y = 0
//TS_MAXX corresponds to ADC value when X = 240 -1
//TS_MAXY corresponds to ADC value when Y = 320 -1

#define TS_MINX 140 
#define TS_MAXX 900
#define TS_MINY 120
#define TS_MAXY 940

int color = BLACK;  //Paint brush color
int ver_num = 5;
char ver_no[] =      "ver 0.05";
char ver_date[] =    "01/04/13";
char* ver_text[] = { "********************************", 
                     "Initial Rev from TFT example",
                     "Added color buttons",
                     "Added text, clear canvas, etc.",
                     "Added clearCanvas,menu functions",
                     "Added pen-select, some strings.",
                     "Found out TFT 2.8in v1 won't work with SD card capability"
                    };

String stringX;

void menu(void)
{
    Tft.fillRectangle(0,0,30,30,BLACK); 
    Tft.drawRectangle(0,0,30,30,WHITE);
    Tft.fillRectangle(30,0,30,30,RED);
    Tft.drawRectangle(30,0,30,30,WHITE);
    Tft.fillRectangle(60,0,30,30,GREEN);
    Tft.drawRectangle(60,0,30,30,WHITE);
    Tft.fillRectangle(90,0,30,30,BLUE);
    Tft.drawRectangle(90,0,30,30,WHITE);
    Tft.fillRectangle(120,0,30,30,CYAN);
    Tft.drawRectangle(120,0,30,30,WHITE);
    Tft.fillRectangle(150,0,30,30,0xffd0);
    Tft.drawRectangle(150,0,30,30,WHITE);
    Tft.fillRectangle(180,0,30,30,WHITE);
    Tft.setDisplayDirect(DOWN2UP);
    Tft.drawString("V",188,21,2,BLACK);
    Tft.setDisplayDirect(LEFT2RIGHT);
    Tft.drawRectangle(180,0,30,30,WHITE);
    Tft.fillRectangle(210,0,30,30,GRAY1);
    Tft.drawRectangle(210,0,30,30,WHITE);
}
  
void clearCanvas(void)
{
    Tft.fillRectangle(0,30,239,289,0xffd0);
    Tft.drawRectangle(0,31,239,288,RED);
    Tft.drawRectangle(1,32,237,286,RED); 
}

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); //init TouchScreen port pins

void setup()
{

Tft.init();  //init TFT library
pinMode(0,OUTPUT);
//Draw the pallet & menu
menu();
clearCanvas();
}

void loop()
{
  
  // a point object holds x y and z coordinates.
  Point p = ts.getPoint();

  //map the ADC value read to into pixel co-ordinates

  p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
  p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > ts.pressureThreshhold) 
  {
 // Detect  paint brush color change
   if(p.y < 30)
    {
      if(p.x >= 0 && p.x < 30)
      {
        menu();
        color = BLACK;
        Tft.drawLine(0,0,30,30,WHITE);
      }
      if(p.x >= 30 && p.x < 60)
      {
        menu();
        color = RED;
        Tft.drawLine(30,0,60,30,WHITE);
//        digitalWrite(0,HIGH);
      }
      if(p.x >= 60 && p.x < 90)
      {
        menu();
        color = GREEN;
        Tft.drawLine(60,0,90,30,BLACK);
      }
      if(p.x >= 90 && p.x < 120)
      {
        menu();
        color = BLUE;
        Tft.drawLine(90,0,120,30,WHITE);
//        digitalWrite(0,LOW);
      }
      if(p.x >= 120 && p.x < 150)
      {
        menu();
        color = CYAN;
        Tft.drawLine(120,0,150,30,BLACK);
      }
      if(p.x >= 150 && p.x < 180)
      {
        menu();
        color = 0xffd0;
        Tft.drawLine(150,0,180,30,BLACK);
      }
      if(p.x >= 180 && p.x < 210)
      {
        menu();
        Tft.setDisplayDirect(DOWN2UP);
        Tft.drawString(ver_no,10,310,1,BLACK);
        Tft.drawString(ver_date,20,310,1,BLUE);
        Tft.drawString(ver_text[ver_num],30,310,1,RED);
        Tft.setDisplayDirect(LEFT2RIGHT);
      }
      if(p.x >= 210 && p.x < 240)
      {
        menu();
        clearCanvas();
      }    
    }
    else    
    {
      Tft.fillCircle(p.x,p.y,2,color);
    }
 }
}
