// By James Taverne, 3/15/2013
// Morse Code Signaller for Arduino
// Connect led to pin 13 or use built-in led.
// Open Serial Monitor and select Newline from drop-down at bottom. 
// Type in a  message and hit Send. 
// Your message will flash on the led in Morse code.
// The letters and corresponding dots and dashes should be displayed on the serial monitor as well.


int incomingByte = 0;
int morseOutput = 13; //Pin for receiving Morse output - attach LED to this pin
int interval = 120;   //length of time for one dot - basic unit of measurement;  one interval occurs between each dot or dash; dash is equivalent to 3 dots; 7 dots between words

String alphabet = "abcdefghijklmnopqrstuvwxyz1234567890 ";
String ALPHABET = alphabet;

String morseCode[] = { 
    ".-", // A
    "-...", // B
    "-.-.", // C
    "-..", // D
    ".", // E
    "..-.", // F
    "--.", // G
    "....", // H
    "..", // I
    ".---", // J
    "-.-", // K
    ".-..", // L
    "--", // M
    "-.", // N
    "---",  // O
    ".--.", // P
    "--.-", // Q
    ".-.", // R
    "...",  // S
    "-", // T
    "..-", // U
    "...-", // V
    ".--", // W
    "-..-", // X
    "-.--", // Y
    "--..", // Z
    ".----", // 1
    "..---", // 2
    "...--", // 3
    "....-", // 4
    ".....", // 5
    "-....", // 6
    "--...", // 7
    "---..", // 8
    "----.", // 9
    "-----", // 0
    " "   //space character
};

void setup() {
  
  Serial.begin(9600);
  pinMode(morseOutput, OUTPUT);
  
  ALPHABET.toUpperCase();

}

void loop() {
 if (Serial.available() > 0) {
    incomingByte = Serial.read();
    char c = (char) incomingByte;
    sendLetter(c);
 } 
    
}

void sendLetter (char c) {
   int i;
   for (i = 0; i < alphabet.length(); i = i + 1) {
       if (alphabet[i] == c || ALPHABET[i] == c) {
           Serial.print(c);
           sendMorseCode(morseCode[i]);
           return;
       }
   }
   if (c == '\n')
      Serial.println();
}

void sendMorseCode (String tokens) {
   int i;
   for (i = 0; i < tokens.length(); i = i + 1) {
       switch (tokens[i]) {
           case '-':
               sendDash();
               break;
           case '.':
               sendDot();
               break;
           case ' ':
               sendEndOfWord();
               break;
       }
   }
   morseOutputOff(2);
   Serial.print(" ");
}

void sendEndOfWord() {
   morseOutputOff(4);
   Serial.print("  ");
}

//basic functions - Morse code concepts  
void sendDot() {
   morseOutputOn(1);
   morseOutputOff(1);
   Serial.print(".");
}
void sendDash() {
   morseOutputOn(3);
   morseOutputOff(1);
   Serial.print("-");
}


//Low level functions - how the actions are accomplished
// n = number of intervals 
// interval is a fixed length of time determined at start, for example 200 milliseconds
void morseOutputOn (int n) {
//   example: morseOutputOn(1) means turn output on and keep it on for 1 interval 
//            morseOutputOn(3) means turn output on and keep it on for 3 intervals 
//   
   digitalWrite(morseOutput, HIGH);
   delay(n * interval);
}

void morseOutputOff (int n) {
   digitalWrite(morseOutput, LOW);
   delay(n * interval);
}

