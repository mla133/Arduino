/*  (c) 2013 M. Ray Burnette, AKA Ray Burne; All Commercial Rights Reserved
    Magic Morse version for Arduino UNO w/ 2x16 parallel LCD driven by LiquidCrystal.h
    Compiler: Arduino 1.0.1 & 1.0.5 (tested)
    Below size and RAM is for Arduino 1.0.1 and gcc version 4.3.3 (WinAVR 20100110)
      ☺  Binary sketch size: 11,066 bytes (of a 32,256 byte maximum) ... USING PROFILE UNO
      ☺  FreeRAM = 1674
    For 1.0.5 and gcc version 4.3.3 (WinAVR 20100110)
      ☺  Binary sketch size: 11,238 bytes (of a 32,256 byte maximum)
      ☺  Free RAM available: 1683
*/

#include <EEPROM.h>
#include <Streaming.h>
#include <LiquidCrystal.h>
// MiniPro backpack & 328P board-duino UNO clone @16MHz w/ UNO bootloader
LiquidCrystal lcd(7, 6, 2, 3, 4, 5);  // BEWARE: does not match Arduino Examples

// constants
const int  EEaddr          = 0;     // EEPROM address for WPM
const int  BAUD            = 9600;  // any standard serial value: 300 - 115200
const int  toneHz          = 748;   // 748 = 746.3 Hz on freq counter (750 = 757.6Hz)
const int  LED_flashdelay  = 2;     // mS ... small dealy to allow eye to see LED
const int  MaxConsoleCount = 80;    // Serial output characters before linefeed+cr
const long debounceDelay   = 20;    // the debounce time. Keep well below dotTime
// Arduino 328P pins (not physical)
// const int  VarResPin       = A0;    // select the input pin for the potentiometer
const byte morseInPin      = A3;    // Used for Morse Key (other end to Gnd)
const byte VerbosePin      =  9;    // Echos from within MagicMorse() decode state of dit/dah
const byte forcePARISpin   = 10;    // When momentary LOW, forces PARIS to be output on LCD/Serial
const byte  toneOutPin     = 11;    // PWM output for simulated 750Hz (F#1/Gb1 = 746. Hz)
const byte  LED_RED        = 12;    // Red   LED indicates Dit
const byte  LED_GREEN      = 13;    // Green LED indicates Dah
// global prog variables
byte WPM;                           // set by POT on pin#A0
byte nRow;                          // line count      (0-5 for NOKIA LCD)
byte nColumn;                       // character count (0-11 for NOKIA LCD)
byte ConsoleCount;                  // Serial character output counter
boolean Verbose;                    // state of Pin#9
char* PROGMEM BlankLine[]  = {"                "};

void setup(void)
  { 
    pinMode(morseInPin, INPUT);
    digitalWrite(morseInPin, HIGH);  // activate internal pullup resistor
    pinMode(VerbosePin, INPUT);
    digitalWrite(VerbosePin, HIGH);  // activate internal pullup
    pinMode(toneOutPin, OUTPUT);
    tone(toneOutPin, toneHz);
    noTone(toneOutPin);
    pinMode(LED_RED, OUTPUT);    digitalWrite(LED_RED, HIGH);
    pinMode(LED_GREEN, OUTPUT);  digitalWrite(LED_GREEN, HIGH);
    pinMode(forcePARISpin, INPUT);
    digitalWrite(forcePARISpin, HIGH);  // internal pullup resistor
    Serial.begin(9600);                 // Serial console log
    lcd.begin(16, 2);                   // Initialize  nColumn = 0 ; nRow = 0;
    // write copyright and version information
    Serial << (F("(c) 2011 - 2013 by M. R. Burnette")) << endl;
    Serial << (F("Free RAM available: ")) << freeRam() << endl;
    lcd.setCursor(nColumn, nRow);  // move to (0, 0)
    lcd.print(F(" Magic Morse V8a"));
    nColumn = 0; ++nRow; lcd.setCursor(nColumn, nRow);
    lcd.print(F("(c) M. Burnette "));
    LEDflasher(LED_GREEN); LEDflasher(LED_RED);
    WPM = EEPROM.read(EEaddr);
    delay(2000);
    // Set WPM default and write to EEPROM IF Morse Key is closed at this point...
    if (!digitalRead(morseInPin) || WPM == 0 || WPM > 40) setWPM(WPM);
    nColumn = 0; nRow = 1;   lcd.setCursor(nColumn, nRow); lcd << BlankLine[0];       // clear lower row
    nColumn = 0; nRow = 0;   lcd.setCursor(nColumn, nRow); lcd << BlankLine[0];       // clear upper row
    lcd.setCursor(nColumn, nRow);
    delay(500);
    setspeed(WPM);
    lcd  << (F("@ ")) << WPM << (F("WPM:")) ;
    nColumn = 9; lcd.setCursor(nColumn, nRow); Paris();
    nColumn = 0; nRow = 1; lcd.setCursor(nColumn, nRow);
    lcd.cursor();  // turn cursor ON
}


void loop(void)
  {
    Verbose = (!digitalRead(VerbosePin)); // IF pin is LOW, Verbose is TRUE, else FALSE
    if (!digitalRead(forcePARISpin)) Paris();
    ReadMorseKeyState();
    char temp = MagicMorse();
    if (temp != 0) {
      if (++ConsoleCount >79 ) {
        Serial << endl;
        ConsoleCount = 1; }
      SendMorseLCD(temp); }
  } // loop()




