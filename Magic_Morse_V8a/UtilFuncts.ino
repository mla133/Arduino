
// Utility Functions for Magic Morse: common to Nokia 6110 and parallel LCD, 2x16, etc.

void SendMorseLCD( char temp ) {
   Serial << temp;                      // to diagnostic channel
   lcd.setCursor(nColumn, nRow);        // LCD position
   lcd << temp;  ++nColumn;             // print to LCD
   if (nColumn > 15) {                  // are we pointing beyond end-of-line?
     nColumn = 0;                       // reset column
     nRow = ++nRow % 2;                 // reset line
     lcd.setCursor(nColumn, nRow);      // set cursor location
     lcd.print(BlankLine[0]);           // blank line on LCD
     }
}


void LEDflasher( int PIN ) {
  int LEDcolor = PIN;
  digitalWrite(LEDcolor, HIGH);
  delay(LED_flashdelay);// a small delay of a few milliseconds is required for eyes to detect blink
  digitalWrite(LED_RED, LOW);  digitalWrite(LED_GREEN, LOW);  // off
}


void showtime(int Which) {
    switch (Which) {
        case 1:    // DIT
              Serial << (F(".(")) << (keyUP-keyDOWN) << (F(") "));
              break;
        case 2:    // DAH
              Serial << (F("-(")) << (keyUP-keyDOWN) << (F(") "));
              break;
        case 3:    // Word break
              Serial << (F(" \\ ")) << endl;
              Serial << (F("Dit Avg = ")) << DitSum / DitCount << (F("[")) << DITmS << (F("]"));
              Serial << (F("  Dah Avg = ")) << DahSum / DahCount << (F("[")) << DAHmS << (F("]")) << endl;
              DitSum = 0; DahSum = 0; DitCount = 0; DahCount = 0;
              break;
    }
}


void STATUS (void)
{
    if (!digitalRead(VerbosePin)); {
      Serial << (F("DIT: "))            << DITmS      <<(F(" mS  "));
      Serial << (F("DIT range:   >  ")) << quarterDIT << (F(" < ")) << halfDAH   << (F(" mS")) << endl;
      Serial << (F("DAH: "))            << DAHmS      <<(F(" mS  "));
      Serial << (F("DAH range:   >= ")) << halfDAH    << (F(" < ")) << DITDAH    << (F(" mS")) << endl;
      Serial << (F("Char Break:  >= ")) << DiDiDi     << (F(" < ")) << wordBreak << (F(" mS")) << endl;
      Serial << (F("Word Break:  >= ")) << wordBreak  << (F(" mS")) << endl      << endl;
    }
}


char PARIS(byte Letter)
{
  switch(Letter)
    {
    case 0:      // P
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin,toneHz); delay(DAHmS);   // <dah>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin,toneHz); delay(DAHmS);   // <dah>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      return 'P';
    case 1:      // A
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin,toneHz); delay(DAHmS);   // <dah>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      return 'A';
    case 2:      // R
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin,toneHz); delay(DAHmS);   // <dah>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      return 'R';
    case 3:      // I
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      return 'I';
    case 4:      //S
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin); delay(DITmS);        // <quiet>
      tone(toneOutPin, toneHz); delay(DITmS);  // <dit>
      noTone(toneOutPin);
      return 'S';
    }
}


void Paris( void )
{
  Serial << (F("WPM: "))            << WPM        << endl;
  if (!digitalRead(VerbosePin)) STATUS();
  Serial << (F("Printing PARIS on LCD display:")) << endl;
  for (int q = 0; q < 5;q++)
    { SendMorseLCD(PARIS(q)); }
  Serial << endl;
}


void setspeed(byte value)  // see:http://kf7ekb.com/morse-code-cw/morse-code-spacing/  
{
  WPM        = value;
  DITmS      = 1200 / WPM;
  DAHmS      = 3 * 1200 / WPM;
  // character break is 3 counts of quiet where dah is 3 counts of tone
  // wordSpace  = 7 * 1200 / WPM;
  wordBreak  = 7 * DITmS;    // changed from wordSpace*2/3; Key UP time in mS for WORDBREAK (space)
  Elements   = MaxElement;   // International Morse is 5 characters but ProSigns are 6 characters
  halfDIT    = DITmS/2;      // Minimum mS that Key must be UP (quiet) before MM assignment to dot/dash
  quarterDIT = DITmS/4;      // Minimum accepted value in mS for a DIT element (sloppy)
  halfDAH    = DAHmS/2;      // Maximum accepted value in mS for a DIT element (sloppy)
  DITDAH     = DITmS + DAHmS;// Maximum accepted value in mS for a DAH element (sloppy)
  DiDiDi     = DITmS * 3;    // Minimum mS that Key must be up to decode a character via MM
}


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


void setWPM(byte WPM)
{
  WPM = 5; setspeed(WPM);  // initialize to 10 for consistent behavior
  tone(toneOutPin, 1500);
  nColumn = 0; nRow = 0;   lcd.setCursor(nColumn, nRow); lcd << BlankLine[0];
    lcd.setCursor(nColumn, nRow);
    lcd.print(F("Auto Cycling WPM"));
    nRow = 1;  // move to second line
    Serial << (F("Waiting on operator to press Morse Key...")) << "\r";
    lcd.setCursor(nColumn, nRow); lcd << BlankLine[0];
    lcd.print(F("Release Key NOW!"));
    delay(2000);
    noTone(toneOutPin);
    nRow = 0; lcd.setCursor(nColumn, nRow); lcd << BlankLine[0];
    lcd.setCursor(nColumn, nRow);
    lcd.print(F("LONG Dash:Record"));
    nRow = 1; lcd.setCursor(nColumn, nRow); lcd << BlankLine[0];
    while(digitalRead(morseInPin)) {
      ++WPM; if (WPM > 40) WPM = 5;
      setspeed(WPM);
      lcd.setCursor(nColumn, nRow);
      lcd << (F(" WPM = ")) << WPM << BlankLine[0];    // YES... goes offscreen!
      delay(700);
    }  // end while
    EEPROM.write(EEaddr, WPM);
    delay(100);
}
