/*
  (c) Mickey R. Burnette, AKA: M. Ray Burne, author and software engineer: various blogs, Instructables, etc.
  Liquid Crystal library reference: http://arduino.cc/en/Reference/LiquidCrystal?from=Tutorial.LCDLibrary
  Timing semantics for Arduino published by Raron:
      http://raronoff.wordpress.com/2010/12/16/morse-endecoder/
  Magic Morse Algorithm Copyright (c) 2011, 2012, 2013 by mrburnette and published at:
      http://code.google.com/p/morse-endecoder/wiki/Usage  
      http://www.instructables.com/id/Morse-Code-Magic-An-Algorithm/
      http://www.youtube.com/watch?v=9kZOqdeUl2w&feature=youtube_gdata
      http://www.picaxeforum.co.uk/showthread.php?19088-Morse-Code-Decoder-for-Cheap-using-a-20X2&p=177912#post177912  
  Contact Ray Burne: magic.morse @ gmail.com - All commercial rights reserved worldwide
  >>> Publically published articles with source grants end-use one (1) personal use license ONLY <<<

Target compiles:  ATmega328P - profile Mini w/328P @ 16MHz 
    ☺ 20130525 - Version 8a  Implemented Auto-setup for WPM and storing in EEPROM
    ☺ 20130525 - Version 8   Merged Nokia base routines with 2x16 LCD
    ☺ 20130523 - Version 6   Configured LCD on Bareboard to be same pinout as on MiniPro, same codebase
    ☺ 20130421 - Version 4.  Board-duio version with LCD and variable resistor for speed timing
    ☺ 20121001 - Version .20 rewrite to eliminate object oriented code for faster inline
    ☺ 20120922 - Version .10 with WPM changes based on D0
    ☺ 20120919 - Edited MM[] + code to allow some prosigns as in PICAXE code
    ☺          - Works from 25WPM to 40WPM using ARRL test files (w/ WPM = 30)
    ☺ 20120918 - Magic Morse Interface MM[] array implemented ClipperDigital
    ☺          - Works from  5WPM to 35WPM using ARRL test files (w/ WPM = 20)
    ☺ 20120917 - Initial code structual build from raron timing strutures

  // Interfacing requirements for use of digital I/O (NPN audio clipper circuit)
     2.2K / 0.1uF (0.2) and input resistance 110 Ohms to 50 Ohms
  // Compile options: (commercial = UNO)

                            MiniPro backpack        328P Board-duino
                           lcd(7, 6, 2, 3, 4, 5)       <ditto>
   * LCD RS     pin to     digital pin 7                  "
   * LCD Enable pin to     digital pin 6                  "
   * LCD D7     pin to     digital pin 5                  "
   * LCD D6     pin to     digital pin 4                  "
   * LCD D5     pin to     digital pin 3                  "
   * LCD D4     pin to     digital pin 2                  "
   * LCD R/W pin to ground
   * 10K variable resistor (contrast):
       * ends to +5V and ground
       * wiper to LCD VO pin (pin 3)


        elements	WPM	tElements	mS / element
        50	          5	  250	          240
        50	         10	  500	          120
        50	         15	  750	           80
        50	         20	 1000	           60
        50	         25	 1250	           48
        50	         30	 1500	           40
        50	         35	 1750	           34
        50	         40	 2000	           30
        50	         45	 2250	           27
        50	         50	 2500	           24


                                      +-\/-+
Reset                           PC6  1|    |28  PC5 (AI 5)
Rx                        (D 0) PD0  2|    |27  PC4 (AI 4)
Tx                        (D 1) PD1  3|    |26  PC3 (AI 3)             Contact (non-Gnd) end of Morse Key
LCD D4                    (D 2) PD2  4|    |25  PC2 (AI 2)
LCD D5               PWM+ (D 3) PD3  5|    |24  PC1 (AI 1)
LCD D6                    (D 4) PD4  6|    |23  PC0 (AI 0)
+5                              VCC  7|    |22  GND
GND                             GND  8|    |21  AREF
XTL1                            PB6  9|    |20  AVCC
XTL2                            PB7 10|    |19  PB5 (D 13)             Green LED  - DAH
LCD D7               PWM+ (D 5) PD5 11|    |18  PB4 (D 12)             Red LED    - DIT
LCD E                PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM         (PWM 750Hz) to Piezo 
LCD RS                    (D 7) PD7 13|    |16  PB2 (D 10) PWM         Force PARIS (momentary Gnd)
                          (D 8) PB0 14|    |15  PB1 (D  9) PWM         Verbose console option - Debug
                                      +----+


*/

