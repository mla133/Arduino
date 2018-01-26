// Part III CQ SENDER - Serial Port Sender with LCD Display
// Authors: Ben Kuo, KK6FUT and Pete Juliano, N6QW 08/2014
// Check the N6QW website http;//www.jessystems.com and clcik on the Arduino Link for the Part III wiring diagram and details on the LCD implementation

#include <toneAC.h>
#define NOTE_F5  698
#include <Wire.h> // Needed for the LCD Display
#include<LiquidCrystal_I2C.h> //Needed for the LCD Display. This specific library is for the 16X2 LCD with the I2C backpack

const int LED = 13;
const int relay = 11;
const int tonepin = 10;  // Speaker on PIN 10
const int dotpin = 9; // dot key into PIN 9
const int dashpin = 8; // dash ley into pin 8
const int analogPin1 = A0;
const int analogPin2 = A1;

// Analog Pin A4 is used for the I2C SDA Data Line --- LCD Implementation
// Analog Pin A5 is used for the I2C SCL Clock Line --- LCD Implementation

long wpm=15; // If you are feeling your oats make this 20 -- if you failed to eat your Wheaties makee it 8
long z = 1200/wpm; // convert wpm to milliseconds
long dashdelay = 3*z; // length of a dash
long dotdelay = z; // length of a dot
long dddelay = z; // length of pauses between dots and dashes
long chardelay = 3*z; // length of spacing between charactersf

int curcol = 0; // track column for lcd.

LiquidCrystal_I2C lcd(0x27,16,2); // This is the address  for the I2C backpack. The Ada Fruit I2C uses a different address and Library, as does the SainSmart I2C + Display. see the website
                                  // If a 16X4 dispaly is used then the address must be lcd(0x27), 16, 4). A 20X4 would be lcd(0x27, 20, 4) -- don't forget the ; at the end.

void setup()
{
  pinMode(LED, OUTPUT);  // sets the digital pin as output
  pinMode(relay, OUTPUT); //sets the relay pin
  pinMode(tonepin, OUTPUT);  //Sets the speaker output
  Serial.begin(19200);     // opens serial port, sets data rate to 19200 bps. 9600 (slower) can be used. But you must use the same data rate in the serial terminal program
  
  lcd.init(); // initialize LCD
  lcd.backlight();
  lcd.setCursor(0,0); // This places a display on the LCD at turn on at the 1st line
  lcd.print("  KK6FUT & N6QW  "); //Any LCD message can be place here. Just make sure there are 16 spaces between the "" marks

  
}

void senddash() {
  digitalWrite(LED, HIGH); 
  digitalWrite(relay, HIGH);  // keys the dash
  tone(10,NOTE_F5);
  delay(dashdelay);
  noTone(10);
  digitalWrite(LED, LOW);
  digitalWrite(relay, LOW);
  delay(dddelay);
}

void senddot() {
  digitalWrite(LED, HIGH);
  digitalWrite(relay, HIGH); // keys the dot
  tone(10,NOTE_F5);
  delay(dotdelay);
  noTone(10);
  digitalWrite(LED, LOW);
  digitalWrite(relay, LOW);
  delay(dddelay);
}

void  sendchar(char letter) {
  if (letter == 'A') {
    senddot();
    senddash(); 
  } 
  else if (letter == 'B') {
    senddash(); // B
    senddot();
    senddot();
    senddot();
  } 
  else if (letter == 'C') {
    senddash(); // C
    senddot();
    senddash();
    senddot();
  } 
  else if (letter == 'D') {
    senddash();   //D
    senddot();
    senddot();
  } 
  else if (letter == 'E') {
    senddot();   //E
  } 
  else if (letter == 'F') {
    senddot();   
    senddot();   
    senddash();   
    senddot();  
  } 
  else if (letter == 'G') {
    senddash();  //G
    senddash();
    senddot();
  } 
  else if (letter == 'H') {
    senddot();  //H
    senddot();
    senddot();
    senddot(); 
  } 
  else if (letter == 'I') {
    senddot();    //I
    senddot();
  } 
  else if (letter == 'J') {
    senddot();    //J
    senddash();
    senddash();
    senddash();   
  } 
  else if (letter == 'K') {
    senddash();    //K
    senddot();
    senddash();   
  } 
  else if (letter == 'L') {
    senddot();   //L
    senddash();    
    senddot();   
    senddot(); 
  } 
  else if (letter =='M') {
    senddash();    //M
    senddash();
  } 
  else if (letter =='N') {
    senddash();    //N
    senddot();
  } 
  else if (letter == 'O') {
    senddash();  //O
    senddash();  
    senddash();  
  } 
  else if (letter == 'P') {
    senddot();    //P
    senddash();
    senddash();
    senddot();  
  } 
  else if (letter == 'Q') { 
    senddash(); //Q
    senddash();
    senddot();
    senddash();
  } 
  else if (letter == 'R') {
    senddot();  //R
    senddash();  
    senddot();  
  } 
  else if (letter == 'S') {
    senddot();
    senddot();
    senddot();
  } 
  else if (letter == 'T') {
    senddash();  //T
  } 
  else if (letter == 'U') {
    senddot();   //U
    senddot();
    senddash();
  } 
  else if (letter == 'V') {
    senddot();    //V
    senddot();
    senddot();
    senddash();
  } 
  else if (letter == 'W') {
    senddot();  //W
    senddash();
    senddash();
  } 
  else if (letter == 'X') {
    senddash();
    senddot();  
    senddot();  
    senddash();
  } 
  else if (letter == 'Y') {
    senddash();    //Y
    senddot();
    senddash();
    senddash();
  } 
  else if (letter == 'Z') {
    senddash();   // Z
    senddash();
    senddot();
    senddot();
  } 
  else if (letter == '1') {
    senddot();    //1
    senddash();
    senddash();
    senddash();
    senddash();
  }
  else if (letter == '2') {
    senddot();    //2
    senddot();
    senddash();
    senddash();
    senddash();
  } 
  else if (letter == '3') {
    senddot();     //3
    senddot();
    senddot();
    senddash();
    senddash();
  } 
   else if (letter == '4') {
    senddot();     //4
    senddot();
    senddot();
    senddot();
    senddash();
  } 
  else if (letter == '5') {
    senddot();
    senddot();
    senddot();
    senddot();
    senddot();  
  } 
  else if (letter == '6') {
    senddash();    //6
    senddot();
    senddot();
    senddot();
    senddot();
  } 
  else if (letter == '7') {
    senddash();    //7
    senddash();
    senddot();
    senddot();
    senddot();
  } 
  else if (letter == '8') {
    senddash();    //8
    senddash();
    senddash();
    senddot();
    senddot();
  } 
  else if (letter == '9') {
    senddash();  //9
    senddash();
    senddash();
    senddash();
    senddot();
  } 
  else if (letter == '0') {
    senddash();    //
    senddash();
    senddash();
    senddash();
    senddash();
  } 
  else if (letter == '?') {
    senddot();   //?
    senddot();
    senddash();
    senddash();
    senddot();
    senddot();
  } 
  else if (letter == '/') {
    senddash();  ///
    senddot();
    senddot();
    senddash();
    senddot();
  } 
  else if (letter == ',') {
    senddash();    //,
    senddash();
    senddot();
    senddot();
    senddash();
    senddash();
  } 
  else if (letter == '-') {
    senddash();    // -
    senddot();
    senddot();
    senddot();
    senddash();
  }
  
  else if (letter == ']') {
    senddot();    // Error in typing
    senddot();
    senddot();
    senddot();
    senddot();
    senddot();
    senddot();
    senddot();
  }
  else if (letter == '.') {
    senddot();    // .
    senddash();
    senddot();
    senddash();
    senddot();
    senddash();
  }



    delay(chardelay);
  }


int check_canned_message(char letter) {
  if (letter == '=') {
    Serial.write('='); // For this canned message this caharacter will not show up on the LCD screen but will on the computer screen
    sendchar('C');
    sendchar('Q');
    delay(200);
    
    sendchar('C');
    sendchar('Q');
    delay(200);

    sendchar('C');
    sendchar('Q');
    delay(200);

    sendchar('D');
    sendchar('E');
    delay(200);
    
    sendchar('N');  // if you want your call sign in here simply insert your call sign using this format
    sendchar('6');
    sendchar('Q');
    sendchar('W');
    delay(200);
    
    sendchar('K');
    delay(200);
    return 1;
  }
  
  return 0;
    
}

int incomingByte = 0;   // for incoming serial data

void loop() {

  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    if (check_canned_message(incomingByte)) { 
       // if =1, we sent canned message, so don't send that character
    } else {
        incomingByte = toupper(incomingByte); // make sure this is uppercase
        sendchar(incomingByte);
        Serial.write(incomingByte); // this is used to display the letters on the computer screen
       
        lcd.setCursor(curcol,1); // The curcol set the location of the cursor in positions 1 -16. The 1 is the second line (line 1 = 0, line 2 = 1)
        lcd.print(char(incomingByte));} // this is used to dsiplay the letters on the LCD 
 
      
        curcol = ( curcol + 1 ) % 16; // next column, wrap at 16th character. This is a simple counter that counts the LCD characters and advances it 1 space. When the counter =16 it goes bacl to 1

 
    }    
    // reflect what was typed, even if canned message character

  {
   
}  //check analog inputs and adjust speed - directly from CW Sender Part IIf
  int dotval = analogRead(analogPin1);    // read the input pin for dot length
  dotdelay = ((dotval * 250L) / 1024) + 30;  // scale our input to be between 30 and 280 milliseconds of delay
 
  int delayval = analogRead(analogPin2);    // read the input pin for delay between dots/dashes
  dddelay = ((delayval * 250L) / 1024) + 30;  // scale our input to be between 30 and 280 milliseconds of delay

}


