// Magic Morse algorithm is (c) 2011, 2012, 2013 by M. Ray Burnette 
// M. Ray Burnette AKA: Ray Burne ALL COMMERCIAL RIGHTS RESERVED WORLDWIDE
// Magic Morse other: 
//    http://www.picaxeforum.co.uk/entry.php?30-Notes-behind-Magic-Morse
//    http://www.instructables.com/id/Morse-Code-Magic-An-Algorithm/
//    http://www.instructables.com/id/Orb-Flashes-Temperature-in-Morse-Code/
//    http://www.instructables.com/id/Zero-to-Morse-in-6-hours/

boolean SignalFlag       = true;
boolean SingleSpace      = true;
boolean CurrentKeyState  = LOW;
boolean MorseKeyState    = LOW;
boolean PreviousKeyState = LOW;
const byte MaxElement    = 6;    // Prosigns are 6 elements
byte Elements;   // Working var reset to MaxElement during decode
byte MMpoint;    // Magic Morse pointer
byte MMcount;    // Magic Morse pointer
int DitCount;    // diagnostics dits in word counter for average
int DahCount;    // diagnostics dahs in word counter for average
long DitSum;     // diagnostics mS of dits in word
long DahSum;     // diagnostics mS of dahs in word
long DITmS;      // morse dot time length in mS
long DAHmS;      // morse dah time length in mS
long quarterDIT; // DITmS/4
long halfDIT;    // DITmS/2
long halfDAH;    // DAHmS/2
long DITDAH;     // DAHmS + DITmS
long DiDiDi;     // 3* DITmS
// long wordSpace;  // defined as 7 * dot time in mS
long wordBreak;  // in Magic Morse, same as Character break = 3*DITmS
long keyDOWN;    // misc. timer
long keyUP;      // misc. timer
long DeBounce;   // misc. timer - mS debounce delay
long TimeStamp;  // misc. timer - millis() real-time reading
char temp;
// ITU (International Morse Code) decoding: The MM[] matrix is decoded in 6-elements to provide for prosigns
// http://upload.wikimedia.org/wikipedia/en/thumb/5/5a/Morse_comparison.svg/350px-Morse_comparison.svg.png
char MM[] PROGMEM = "_EISH5ee0TNDB6-0"    //   0 - 15      e == ERROR
                    "00ARLw0000MGZ700"    //  16 - 31      w == WAIT
                    "000UF0000i0KC000"    //  32 - 47      i == INVITE
                    "000WP000000O0800"    //  48 - 63
                    "0000Vu]00000X/00"    //  64 - 79      u == UNDERSTOOD  ] == End Of Work
                    "00000+.00000Q000"    //  80 - 95
                    "000000?00000Y()0"    //  96 - 111     () == Left/Right hand bracket
                    "0000J0000000e900"    // 112 - 127
                    "000004(c) M.R=BU"    // 128 - 143
                    "RNETTE'0000000,0"    // 144 - 159     ' @ [150] should be "
                    "00>0000000000[00"    // 160 - 175     [ == Starting Signal
                    "000000@000000000"    // 176 - 191
                    "0000030000000000"    // 192 - 207
                    "0000000000000000"    // 208 - 223
                    "0000020000000000"    // 224 - 239
                    "000001'000000000";   // 240 - 255


void ReadMorseKeyState()
{
    TimeStamp = millis();
    // Read state of Morse Code digital input
    MorseKeyState = !digitalRead(morseInPin);  // ActiveLow so inverse: key closed, MorseKeyState = true
    // Noise suspression
    if (MorseKeyState != PreviousKeyState) 
      DeBounce = TimeStamp; // reset timer
    // Debounce
    if ((TimeStamp - DeBounce) > debounceDelay)
      {
      // take whatever the current state reading is after the debounce time
      CurrentKeyState = MorseKeyState;
        // differentiante keyDOWN and keyUP times
        if (CurrentKeyState) {
          tone(toneOutPin, toneHz);
          keyDOWN = DeBounce; 
          SignalFlag   = false;
          SingleSpace  = false;
        } //end if (CurrentKeyState)
        else {
            keyUP = DeBounce;
            noTone(toneOutPin); } // else
      } //end if ((TimeStamp - DeBounce) > debounceDelay)
} // end ReadMorseKeyState


int MagicMorse()
{
  // Decode morse code using Magic Morse algorithm, (c) 2011, 2012, 2013 by M.R. Burnette
  // char() value is returned by this function for the decoded Morse elements
  if (!CurrentKeyState) {    // do not decode when Morse Key is active (closed)
      // If the key is NOT down AND if there are elements AND IF sufficient time has passed since last element...
      // A valid time period for decoding is the Key is UP for GT 50% of a DIT or the Key was DOWN for GT 25% DOT time
    if (!SignalFlag && ((Elements > 0) &&  ((TimeStamp - keyUP > halfDIT) || (keyUP-keyDOWN > quarterDIT))))   {
      // if signal for less than half a dash, take it as a dot
      if (keyUP-keyDOWN < halfDAH) {  // DIT? That is, less that 150% of a DIT?
         SignalFlag = true;
         noTone(toneOutPin);
         // Magic Morse: increment count for Dot
         LEDflasher(LED_GREEN);
         ++MMcount;  --Elements;
         DitSum += (keyUP-keyDOWN); ++ DitCount;
         if (Verbose) showtime(1);
         // end acceptance for DIT
         } else if (keyUP-keyDOWN < DITDAH) {  // DAH? That is, less that a DIT + DAH?
              SignalFlag    = true;
              // Magic Morse: increment count for Dash
              LEDflasher(LED_RED);
              ++MMcount; --Elements;
              DahSum += (keyUP-keyDOWN); ++ DahCount;
              if (Verbose) showtime(2);
              // Magic Morse Dash weighting based on element position
              switch (MMcount) {
                case 1:  MMpoint = MMpoint | B00001000 ;  break;
                case 2:  MMpoint = MMpoint | B00010000 ;  break;
                case 3:  MMpoint = MMpoint | B00100000 ;  break;
                case 4:  MMpoint = MMpoint | B01000000 ;  break;
                case 5:  MMpoint = MMpoint | B10000000 ;  break;
                }  // switch case                 
          } // end acceptance for DAH
     } // end element validity check DOT or DASH, that is a state change has occurred
    // RETURN character if 2 x dot time has expired (2/3 of a dash)
    if ((TimeStamp-keyUP >= (DiDiDi)) && (Elements < MaxElement)) {
      noTone(toneOutPin);
      // Magic Morse
      MMpoint += MMcount;
      temp    = pgm_read_word(&MM[MMpoint]);
      // Prosign adjustments done here to ease decoding
      switch (MMpoint) {
          case  62:  temp = char(58) ;  break;  // colon
          case 150:  temp = char(34) ;  break;  // double quotes
          case 246:  temp = char(39) ;  break;  // apostrophe
          }
      MMpoint    = 0; MMcount    = 0; Elements = MaxElement;
      noTone(toneOutPin);
      if (temp > char(31) && temp < char(127))  // Legal Morse character set
        {
          return temp;
        } // end if (temp > char(31) && temp < char(127))
    } //end if ((TimeStamp-keyUP >= (DITmS*2)) && (Elements < 6))
    // RETURN a word space once and default back to singlespace
    if ((TimeStamp-keyUP >= wordBreak) && SingleSpace == false)  {
      SingleSpace = true ; // space written-flag, don't do it again until after next valid element
      temp = char(32);
      MMpoint    = 0; MMcount    = 0; Elements = MaxElement;
      if (Verbose) showtime(3);
      noTone(toneOutPin);
      return temp;
    } //end if (TimeStamp-keyUP > (wordSpace*2/3) && SingleSpace == false)
  } //end if (!CurrentKeyState) that is, Morse Key is 'down' and mS are being accumulated
  // update last input state to current value
  PreviousKeyState = MorseKeyState;
  return 0;
} // end decode()

