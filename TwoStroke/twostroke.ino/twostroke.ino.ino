
//By: Chuck Kozik

//Notes Timer0 used for timer functions delay(), millis() and micros()
//The Servo Library will not be used
//Therefore, 16 bit timers Timer1, Timer3, Timer4, Timer5 will be used to
//generate pulse outputs.
//Timer1 will control PB5 (PULSE1)
//Timer3 will control PE4 (PULSE2)
//Timer4 will control PH3 (PULSE3)
//Timer5 will control PL4 (PULSE4)

#include <SPI.h>
//be careful which Ethernet library to use. Arduino.org vs Arduino.cc
//#include <Ethernet.h>//use for older older ethernet shield
#include <Ethernet2.h> //included in version 1.7.10 of ide
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr
//reference c:ProgramFiles(86)-Arduino-hardware-tools-avr-avr-include-avr-iomxx0_1.h
//be sure Tools-Board is set to correct device

//--------------------Defines--------------------

#define RISE_EDGE        1
#define FALL_EDGE        2

#define MAX_FREQ_HZ          (unsigned long)500

#define POF_NORMAL_MODE   0
#define POF_LEAK_MODE     1
#define POF_CLEAR_COUNTS  2

#define MAX_DHCP_RETRIES      2


// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x84, 0x74 };//byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xCC, 0xBA };
EthernetServer server(80);  // create a server at port 80
String HTTP_req;            // stores the HTTP request
void check_MI1_adjust(void);
void check_MI2_adjust(void);
void Ethernet_Control(EthernetClient client);
//Pulse Out 1
int MtrInjOut1 = 11;//PB5 (arduino digital pin # 11)
int MtrInjOut2 = 12;//PB6 (arduino digital pin # 12)
int MtrInjOut3 = 5;//PE4 (arduino digital pin # 2)
int MtrInjOut4 = 2;//PE3 (arduino digital pin # 5)
int MtrInjSol1 = 48;
int MtrInjSol2 = 49;
int MtrInjSol3 = 21;
int MtrInjSol4 = 20;



//Pulse Out 1
unsigned long pulse1Count;
unsigned long pulse1Rem = 0;
unsigned long pulse1Amt = 100;
unsigned char inj1Cntrl = 0;
int pulse1State;
unsigned int pulse1Freq;
unsigned int pulse1MaxFreq;
int pulse1OutFunctionReq;
int pulse1OutFunction;
unsigned int pulse1OCRA;
bool inj_1_pos = 0;


//Pulse Out 2
unsigned long pulse2Count;
unsigned long pulse2Rem = 0;
unsigned long pulse2Amt = 100;
unsigned char inj2Cntrl = 0;
int pulse2State;
unsigned int pulse2Freq;
unsigned int pulse2MaxFreq;
int pulse2OutFunctionReq;
int pulse2OutFunction;
unsigned int pulse2OCRA;
bool inj_2_pos = 0;

//Pulse Out 3
unsigned long pulse3Count;
unsigned long pulse3Rem = 0;
unsigned long pulse3Amt = 100;
unsigned char inj3Cntrl = 0;
int pulse3State;
unsigned int pulse3Srate;
unsigned int pulse3Freq;
unsigned int pulse3MaxFreq;
int pulse3OutFunctionReq;
int pulse3OutFunction;
unsigned int pulse3OCRA;
bool inj_3_pos = 0;

//Pulse Out 4
unsigned long pulse4Count;
unsigned long pulse4Rem = 0;
unsigned long pulse4Amt = 100;
unsigned char inj4Cntrl = 0;
int pulse4State;
unsigned int pulse4Srate;
unsigned int pulse4Freq;
unsigned int pulse4MaxFreq;
int pulse4OutFunctionReq;
int pulse4OutFunction;
unsigned int pulse4OCRA;
bool inj_4_pos = 0;

const char *prompt =
  "    ______  ___      ___   _______\r\n"
  "   |   ___||   \\    /   | /       |\r\n"
  " __|  |__  |    \\  /    ||   _____|\r\n"
  "(__    __) |     \\/     ||  (_____\r\n"
  "   |  |    |  |\\    /|  ||        |\r\n"
  "   |__|    |__| \\__/ |  | \\_______| Technologies\r\n";

bool ethernet_ready = true;

void setup()
{
  char retries = 0;

  //to use Ethernet 2 set pin 53 as an output
  pinMode(53, OUTPUT);
  Serial.begin(115200);

  //Print Device Information


  Serial.print(prompt);
  Serial.println("\nAccuTest Board - Two Stroke Injector Simulator\n");

  Serial.print("MAC Address: ");
  for (byte i = 0; i < sizeof(mac); i++)
  {
    Serial.print(mac[i], HEX);

    if (i < (sizeof(mac) - 1))
    {
      Serial.print("-");
    }
    else
    {
      Serial.print("\n");
    }
  }

  while (!Ethernet.begin(mac))
  {
    retries++;
    if (retries >= MAX_DHCP_RETRIES)
    {
      Serial.println("Failed to Get DHCP Address.");
      Serial.println("Starting in Manual Mode.");
      ethernet_ready = false;
      break;
    }
    Serial.println("Failed to Get DHCP Address, Trying Again");
    Serial.print("Attempt ");
    Serial.println(retries);

    delay(2000);
  }

  if (ethernet_ready)
  {
    Serial.print("IP Address:  ");
    for (byte i = 0; i < 4; i++)
    {
      Serial.print(Ethernet.localIP()[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
      else
      {
        Serial.print("\n");
      }
    }

    server.begin();           // start to listen for clients
  }

  pinMode(MtrInjOut1, OUTPUT);
  pinMode(MtrInjOut2, OUTPUT);
  pinMode(MtrInjOut3, OUTPUT);
  pinMode(MtrInjOut4, OUTPUT);

  pinMode(MtrInjSol1, INPUT);
  pinMode(MtrInjSol2, INPUT);
  pinMode(MtrInjSol3, INPUT);
  pinMode(MtrInjSol4, INPUT);

  pulse1Count = 0;
  pulse1State = RISE_EDGE;
  pulse1Freq = 0;
  pulse1MaxFreq = 100;

  //TIMER0 is used for internal Arduino functionality e.g millis()
  //diconnect port pins from timer since they will be used by application
  TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
  TCCR1A = 0;
  //set control register B for 256 bit prescale, CTC mode
  TCCR1B = ((1 << WGM12) | (1 << CS12));
  pulse1OCRA = OCR1A = 0x00ff;
  TIMSK1 |= (1 << OCIE1A); //enable interript for Timer/Counter1 Output Compare Match A

  //set up for pulse output 2, uses timer 3
  pulse2Count = 0;
  pulse2State = RISE_EDGE;
  pulse2Freq = 0;
  pulse2MaxFreq = 100;

  TCCR3A = 0;
  //set control register B for 256 bit prescale, CTC mode
  TCCR3B = ((1 << WGM32) | (1 << CS32));
  pulse2OCRA = OCR3A = 0x00ff;
  TIMSK3 |= (1 << OCIE3A); //enable interript for Timer/Counter 3 Output Compare Match A

  //set up for pulse output 3, uses timer 4
  pulse3Count = 0;
  pulse3State = RISE_EDGE;
  pulse3Freq = 0;
  pulse3MaxFreq = 100;
  TCCR4A = 0;

  //set control register B for 256 bit prescale, CTC mode
  TCCR4B = ((1 << WGM42) | (1 << CS42));
  pulse3OCRA = OCR4A = 0x00ff;
  TIMSK4 |= (1 << OCIE4A); //enable interript for Timer/Counter 4 Output Compare Match A

  //set up for pulse output 4, uses timer 5
  pulse4Count = 0;
  pulse4State = RISE_EDGE;
  pulse4Freq = 0;
  pulse4MaxFreq = 100;
  TCCR5A = 0;

  //set control register B for 256 bit prescale, CTC mode
  TCCR5B = ((1 << WGM52) | (1 << CS52));
  pulse4OCRA = OCR5A = 0x00ff;
  TIMSK5 |= (1 << OCIE5A); //enable interript for Timer/Counter 5 Output Compare Match A

  SREG |= 0b1000000;//global interrupt enable
}

void loop()
{
  EthernetClient client;

  check_MI1_adjust();
  check_MI2_adjust();
  check_MI3_adjust();
  check_MI4_adjust();

  if (ethernet_ready)
  {
    Ethernet_Control(client);
  }
}

///Mtr Inj 1
void check_MI1_adjust(void)
{
  unsigned int ocr;
  unsigned int i;
  int _on;
  static bool _was_on = false;
  unsigned int _freq;

  //Override the Digital Input if Needed
  if(inj1Cntrl == 2)
  {
    _on = 0;
  }
  else if(inj1Cntrl == 1)
  {
    _on = 1;
  }
  else
  {
    _on = digitalRead(MtrInjSol1);
  }
  
  if ((_on == 0) && (inj_1_pos == 0)) //H11L1 inverts
  {
    inj_1_pos = 1;
    pulse1Rem = pulse1Amt;
    //solenoid just went active
    _freq = pulse1MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK1 &= ~(1 << OCIE1A); //temporarily disable interrupt
    i++; //delay
    pulse1OCRA = ocr;
    TIMSK1 |= (1 << OCIE1A); //reenable interrupt
    _was_on = true;
    pulse1Freq = _freq;
  }
  else if ((_on == 1) && (inj_1_pos == 1))
  {
    inj_1_pos = 0;
    //solenoid  just went not active
    pulse1Rem = pulse1Amt;
    //solenoid just went active
    _freq = pulse1MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK1 &= ~(1 << OCIE1A); //temporarily disable interrupt
    i++; //delay
    pulse1OCRA = ocr;
    TIMSK1 |= (1 << OCIE1A); //reenable interrupt
    _was_on = false;
    pulse1Freq = _freq;
  }
  else if(pulse1Rem == 0)
  {
       pulse1Freq = 0;
  }

}
//pulse 2
void check_MI2_adjust(void)
{
  unsigned int ocr;
  unsigned int i;
  int _on;
  static bool _was_on = false;
  unsigned int _freq;

 //Override the Digital Input if Needed
  if(inj2Cntrl == 2)
  {
    _on = 0;
  }
  else if(inj2Cntrl == 1)
  {
    _on = 1;
  }
  else
  {
    _on = digitalRead(MtrInjSol2);
  }

  if ((_on == 0) && (inj_2_pos == 0)) //H11L1 inverts
  {
    inj_2_pos = 1;
    pulse2Rem = pulse2Amt;
    //solenoid just went active
    _freq = pulse2MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK3 &= ~(1 << OCIE3A); //temporarily disable interrupt
    i++; //delay
    pulse2OCRA = ocr;
    TIMSK3 |= (1 << OCIE3A); //reenable interrupt
    _was_on = true;
    pulse2Freq = _freq;
  }
  else if ((_on == 1) && (inj_2_pos == 1))
  {
    inj_2_pos = 0;

    pulse2Rem = pulse2Amt;
    //solenoid just went active
    _freq = pulse2MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK3 &= ~(1 << OCIE3A); //temporarily disable interrupt
    i++; //delay
    pulse2OCRA = ocr;
    TIMSK3 |= (1 << OCIE3A); //reenable interrupt
    _was_on = false;
    pulse2Freq = _freq;
  }
  else if(pulse2Rem == 0)
  {
       pulse2Freq = 0;
  }

}

//pulse 3
void check_MI3_adjust(void)
{
  unsigned int ocr;
  unsigned int i;
  int _on;
  static bool _was_on = false;
  unsigned int _freq;

 //Override the Digital Input if Needed
  if(inj3Cntrl == 2)
  {
    _on = 0;
  }
  else if(inj3Cntrl == 1)
  {
    _on = 1;
  }
  else
  {
    _on = digitalRead(MtrInjSol3);
  }
  if ((_on == 0) && (inj_3_pos == 0)) //H11L1 inverts
  {
    inj_3_pos = 1;

    pulse3Rem = pulse3Amt;
    //solenoid just went active
    _freq = pulse3MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK4 &= ~(1 << OCIE4A); //temporarily disable interrupt
    i++; //delay
    pulse3OCRA = ocr;
    TIMSK4 |= (1 << OCIE4A); //reenable interrupt
    _was_on = true;
    pulse3Freq = _freq;
  }
  else if ((_on == 1) && (inj_3_pos == 1))
  {
    inj_3_pos = 0;

   
    pulse3Rem = pulse3Amt;
    //solenoid just went active
    _freq = pulse3MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK4 &= ~(1 << OCIE4A); //temporarily disable interrupt
    i++; //delay
    pulse3OCRA = ocr;
    TIMSK4 |= (1 << OCIE4A); //reenable interrupt
    pulse3Freq = _freq;
    _was_on = false;
  }
  else if(pulse3Rem == 0)
  {
       pulse3Freq = 0;
  }
}

//pulse 4
void check_MI4_adjust(void)
{
  unsigned int ocr;
  unsigned int i;
  int _on;
  static bool _was_on = false;
  unsigned int _freq;

 //Override the Digital Input if Needed
  if(inj4Cntrl == 2)
  {
    _on = 0;
  }
  else if(inj4Cntrl == 1)
  {
    _on = 1;
  }
  else
  {
    _on = digitalRead(MtrInjSol4);
  }

  if ((_on == 0) && (inj_4_pos == 0)) //H11L1 inverts
  {
    inj_4_pos = 1;

    pulse4Rem = pulse4Amt;
    //solenoid just went active
    _freq = pulse4MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK5 &= ~(1 << OCIE5A); //temporarily disable interrupt
    i++; //delay
    pulse4OCRA = ocr;
    TIMSK5 |= (1 << OCIE5A); //reenable interrupt
    _was_on = true;
    pulse4Freq = _freq;
  }
  else if ((_on == 1) && (inj_4_pos == 1))
  {
    inj_4_pos = 0;

    pulse4Rem = pulse4Amt;
    //solenoid just went active
    _freq = pulse4MaxFreq;
    //use 8000000 since we are interrupting on rising and falling B
    ocr = 31250 / _freq; //8000000/256 = 31250. 256 is prescale
    TIMSK5 &= ~(1 << OCIE5A); //temporarily disable interrupt
    i++; //delay
    pulse4OCRA = ocr;
    TIMSK5 |= (1 << OCIE5A); //reenable interrupt
    pulse4Freq = _freq;
    _was_on = false;
  }
  else if(pulse4Rem == 0)
  {
       pulse4Freq = 0;
  }
}


/**********************************************************************************/
void Ethernet_Control(EthernetClient client)
{
  //Serial.print("Hi Chuck");
  client = server.available();  // try to get client
  int i = 0;

  if (client)
  { // got client?
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      { // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        HTTP_req += c;  // save the HTTP request 1 char at a time
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: keep-alive");
          client.println();

          // AJAX request for switch state
          if (HTTP_req.indexOf("ajax_pulse") > -1)
          {
            // read switch state and analog input
            GetAjaxData(client);
          }
          else if (HTTP_req.indexOf("m1sld.txt") > -1)
          {
            pulse1MaxFreq = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("m2sld.txt") > -1)
          {
            pulse2MaxFreq = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("m3sld.txt") > -1)
          {
            pulse3MaxFreq = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("m4sld.txt") > -1)
          {
            pulse4MaxFreq = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("m1pul.txt") > -1)
          {
            pulse1Amt = http_string_to_int(HTTP_req);
                   Serial.println(HTTP_req);
          }
          else if (HTTP_req.indexOf("m2pul.txt") > -1)
          {
            pulse2Amt = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("m3pul.txt") > -1)
          {
            pulse3Amt = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("m4pul.txt") > -1)
          {
            pulse4Amt = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("i1Pos.txt") > -1)
          {
            inj1Cntrl = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("i2Pos.txt") > -1)
          {
            inj2Cntrl = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("i3Pos.txt") > -1)
          {
            inj3Cntrl = http_string_to_int(HTTP_req);
          }
          else if (HTTP_req.indexOf("i4Pos.txt") > -1)
          {
           inj4Cntrl = http_string_to_int(HTTP_req);
          }
          else
          { // HTTP request for web page
            // send web page - contains JavaScript with AJAX calls
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head>");
                        //Dummy iframe to Prevent Page Redirects
            client.println("<iframe width=\"0\" height=\"0\" border=\"0\" name=\"dumframe\" id=\"dumframe\"></iframe>");
            client.println("<title>Injector 1-4 Control Web Page</title>");
            client.println("<script>");
            client.println("function GetPulseCountData() {");
            client.println(
              "nocache = \"&nocache=\" + Math.random() * 1000000;");
            client.println("var request = new XMLHttpRequest();");
            client.println("request.onreadystatechange = function() {");
            client.println("if (this.readyState == 4) {");
            client.println("if (this.status == 200) {");
            client.println("if (this.responseText != null) {");
            client.println("document.getElementById(\"pulse_data\").innerHTML = this.responseText;");
            client.println("}}}}");
            client.println(
              "request.open(\"GET\", \"ajax_pulse\" + nocache, true);");
            client.println("request.send(null);");
            client.println("setTimeout('GetPulseCountData()', 1000);");
            client.println("}");
            /***/
            //start of meter 1 slider()
            client.println("function updateSlider1(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider1\");");
            client.println("document.getElementById(\"slider1\").innerHTML= \"Meter 1 = \"+slideAmount+\" Hz\";");
            client.println("}");

            client.println("function updateFreq1(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider1\");");
            client.println("xmlhttp.open(\"GET\",\"m1sld.txt?q=\"+slideAmount,true);");
            client.println("xmlhttp.send();");
            client.println("}");
            //end of meter 1 slider

            //start of meter 2 slider()
            client.println("function updateSlider2(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider2\");");
            client.println("document.getElementById(\"slider2\").innerHTML= \"Meter 2 = \"+slideAmount+\" Hz\";");
            client.println("}");

            client.println("function updateFreq2(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider2\");");
            client.println("xmlhttp.open(\"GET\",\"m2sld.txt?q=\"+slideAmount,true);");
            client.println("xmlhttp.send();");
            client.println("}");
            //end of meter 2 slider

            //start of meter 3 slider()
            client.println("function updateSlider3(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider3\");");
            client.println("document.getElementById(\"slider3\").innerHTML= \"Meter 3 = \"+slideAmount+\" Hz\";");
            client.println("}");

            client.println("function updateFreq3(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider3\");");
            client.println("xmlhttp.open(\"GET\",\"m3sld.txt?q=\"+slideAmount,true);");
            client.println("xmlhttp.send();");
            client.println("}");
            //end of meter 3 slider

            //start of meter 4 slider()
            client.println("function updateSlider4(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider4\");");
            client.println("document.getElementById(\"slider4\").innerHTML= \"Meter 4 = \"+slideAmount+\" Hz\";");
            client.println("}");

            client.println("function updateFreq4(slideAmount)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("var sliderDiv = document.getElementById(\"slider4\");");
            client.println("xmlhttp.open(\"GET\",\"m4sld.txt?q=\"+slideAmount,true);");
            client.println("xmlhttp.send();");
            client.println("}");
            //end of meter 4 slider

            client.println("function updatePulCnt(element)");
            client.println("{");
            client.println("var xmlhttp= new XMLHttpRequest();");
            client.println("if (element.id = document.getElementById(\"pulCnt1\")){");
            client.println("xmlhttp.open(\"GET\",\"m1pul.txt?q=\"+element.value,true);}");
            client.println("else if (element.id = document.getElementById(\"pulCnt2\")){");
            client.println("xmlhttp.open(\"GET\",\"m2pul.txt?q=\"+element.value,true);}");
            client.println("else if (element.id = document.getElementById(\"pulCnt3\")){");
            client.println("xmlhttp.open(\"GET\",\"m3pul.txt?q=\"+element.value,true);}");
            client.println("else if (element.id = document.getElementById(\"pulCnt4\")){");
            client.println("xmlhttp.open(\"GET\",\"m4pul.txt?q=\"+element.value,true);}");
            client.println("xmlhttp.send();");
            client.println("}");
            
            

            /****/
            client.println("</script>");
            client.println("</head>");
            client.println("<body onload=\"GetPulseCountData()\">");
            client.println("<h1>Meter Pulse Data (1-4)</h1>");
            client.println("<div id=\"pulse_data\">");
            client.println("</div>");
            /****************************************/

            client.println("<table style=\"width:70%\">");
            client.println("<tr>");
            client.println("<td>");
            client.println("<p>Meter 1 Freq Adjust</p>");
            client.println("0 Hz <input id=\"slide\" type=\"range\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"500\" step=\"1\" value=\"100\"");
            client.println("oninput=\"updateSlider1(this.value)\" onchange=\"updateFreq1(this.value)\">");
            client.println("500 Hz");
            client.println("<div id=\"slider1\">Meter 1 = 100Hz</div>");
            client.println("</td>");
            //end slider meter 1

            //start meter 2 slide
            client.println("<td>");
            client.println("<p>Meter 2 Freq Adjust</p>");
            client.println("0 Hz <input id=\"slide\" type=\"range\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"500\" step=\"1\" value=\"100\"");
            client.println("oninput=\"updateSlider2(this.value)\" onchange=\"updateFreq2(this.value)\">");
            client.println("500 Hz");
            client.println("<div id=\"slider2\">Meter 2 = 100Hz</div>");
            client.println("</td>");
            //end meter 2 slide

            //start meter 3 slide
            client.println("<td>");
            client.println("<p>Meter 3 Freq Adjust</p>");
            client.println("0 Hz <input id=\"slide\" type=\"range\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"500\" step=\"1\" value=\"100\"");
            client.println("oninput=\"updateSlider3(this.value)\" onchange=\"updateFreq3(this.value)\">");
            client.println("500 Hz");
            client.println("<div id=\"slider3\">Meter 3 = 100Hz</div>");
            client.println("</td>");
            //end meter 3 slide

            //start meter 4 slide
            client.println("<td>");
            client.println("<p>Meter 4 Freq Adjust</p>");
            client.println("0 Hz <input id=\"slide\" type=\"range\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"500\" step=\"1\" value=\"100\"");
            client.println("oninput=\"updateSlider4(this.value)\" onchange=\"updateFreq4(this.value)\">");
            client.println("500 Hz");
            client.println("<div id=\"slider4\">Meter 4 = 100Hz</div>");
            client.println("</td>");
            //end meter 4 slide
            client.println("</tr>");

            client.println("<tr>");

            //Meter 1 Number of Pulses
            client.println("<td>");
            client.println("<p>Meter 1 # of Pulses</p>");
            client.println("<input id=\"pulCnt1\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"99999\" value=\"100\"");
            client.println("onchange=\"updatePulCnt(this)\">");
            client.println("</td>");

            //Meter 2 Number of Pulses
            client.println("<td>");
            client.println("<p>Meter 2 # of Pulses</p>");
            client.println("<input id=\"pulCnt2\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"99999\" value=\"100\"");
            client.println("onchange=\"updatePulCnt(this)\">");
            client.println("</td>");

            //Meter 3 Number of Pulses
            client.println("<td>");
            client.println("<p>Meter 3 # of Pulses</p>");
            client.println("<input id=\"pulCnt3\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"99999\" value=\"100\"");
            client.println("onchange=\"updatePulCnt(this)\">");
            client.println("</td>");

            //Meter 4 Number of Pulses
            client.println("<td>");
            client.println("<p>Meter 4 # of Pulses</p>");
            client.println("<input id=\"pulCnt4\" autocomplete=\"off\"");
            client.println("min=\"0\" max=\"99999\" value=\"100\"");
            client.println("onchange=\"updatePulCnt(this)\">");
            client.println("</td>");

            client.println("</tr>");

            client.println("</tr>");

            client.println("<td>");
            client.println("<form action=\"i1Pos.txt\" target=\"dumframe\" method=\"get\">");
            client.println("<p>Meter 1 Position</p>");
            client.println("<input name=\"q\" value=\"0\" type=\"radio\" onchange=\"this.form.submit()\" checked autocomplete=\"off\">Auto");
            client.println("<input name=\"q\" value=\"1\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">0");
            client.println("<input name=\"q\" value=\"2\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">1");
            client.println("</form>");
            client.println("</td>");

            client.println("<td>");
            client.println("<form action=\"i2Pos.txt\" target=\"dumframe\" method=\"get\">");
            client.println("<p>Meter 2 Position</p>");
            client.println("<input name=\"q\" value=\"0\" type=\"radio\" onchange=\"this.form.submit()\" checked autocomplete=\"off\">Auto");
            client.println("<input name=\"q\" value=\"1\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">0");
            client.println("<input name=\"q\" value=\"2\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">1");
            client.println("</form>");
            client.println("</td>");

            client.println("<td>");
            client.println("<form action=\"i3Pos.txt\" target=\"dumframe\" method=\"get\">");
            client.println("<p>Meter 3 Position</p>");
            client.println("<input name=\"q\" value=\"0\" type=\"radio\" onchange=\"this.form.submit()\" checked autocomplete=\"off\">Auto");
            client.println("<input name=\"q\" value=\"1\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">0");
            client.println("<input name=\"q\" value=\"2\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">1");
            client.println("</form>");
            client.println("</td>");

            client.println("<td>");
            client.println("<form action=\"i4Pos.txt\" target=\"dumframe\" method=\"get\">");
            client.println("<p>Meter 4 Position</p>");
            client.println("<input name=\"q\" value=\"0\" type=\"radio\" onchange=\"this.form.submit()\" checked autocomplete=\"off\">Auto");
            client.println("<input name=\"q\" value=\"1\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">0");
            client.println("<input name=\"q\" value=\"2\" type=\"radio\" onchange=\"this.form.submit()\" autocomplete=\"off\">1");
            client.println("</form>");
            client.println("</td>");

            client.println("</table>");

            client.println("</body>");
            //*******
            client.println("</html>");
          }
          // display received HTTP request on serial port
          //Serial.print(HTTP_req);
          HTTP_req = "";            // finished with request, empty string
          break;//while loop
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n')
        {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data, 1 millisecond
    client.stop(); // close the connection
  } // end if (client)
}

void GetAjaxData(EthernetClient cl)
{
  //cl.println("<table style="width:100"%>");
  cl.println("<table style=\"width:50%\">");
  cl.println("<tr>");
  cl.println("<td>");
  cl.println(" ");
  cl.println("</td>");
  cl.println("<td>");
  cl.println("<h3>Counts</h3>");
  cl.println("</td>");
  cl.print("<td>");
  cl.println("<h3>Pulse Freq</h3>");
  cl.println("</td>");
  cl.print("<td>");
  cl.println("<h3>State</h3>");
  cl.println("</td>");
  cl.println("</tr>");

  //Meter 1
  cl.println("<td>");
  cl.println("Meter 1:");
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse1Count);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse1Freq);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(inj_1_pos);
  cl.println("</td>");
  cl.println("</tr>");

  //Meter 2
  cl.println("<tr>");
  cl.println("<td>");
  cl.println("Meter 2:");
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse2Count);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse2Freq);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(inj_2_pos);
  cl.println("</td>");
  cl.println("</tr>");

  //Meter 3
  cl.println("<tr>");
  cl.println("<td>");
  cl.println("Meter 3:");
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse3Count);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse3Freq);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(inj_3_pos);
  cl.println("</td>");
  cl.println("</tr>");

  //Meter 4
  cl.println("<tr>");
  cl.println("<td>");
  cl.println("Meter 4:");
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse4Count);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(pulse4Freq);
  cl.println("</td>");
  cl.println("<td>");
  cl.println(inj_4_pos);
  cl.println("</td>");
  cl.println("</tr>");

  //close the table
  cl.println("</table>");

}
/********************************************************************************/

unsigned int http_string_to_int(String HTTP_req_arg)
{
  String my_string;//object
  char char_val = 0;
  int index = 17;
  unsigned int num = 0;
  bool valid = false;

  char_val = HTTP_req_arg.charAt(index);//get first character
  while ((char_val >= '0') && (char_val <= '9'))
  {
    my_string += char_val;//append
    valid = true;
    index++;
    char_val = HTTP_req_arg.charAt(index);//get successive characters
  }
  if (valid)
    num = (unsigned int)my_string.toInt();

  return (num);
}

/**********************************************************************************/
#define P1_RISE   PORTB |=0x20;//set PB5 high(11);
#define P1_FALL   PORTB &=0xdf;//clear PB5 (11) 
ISR(TIMER1_COMPA_vect)
{
  //static int local_pulse_delay;
  //interrupt flag automatically cleared upon interrupt execution
  switch (pulse1State)
  {
    case RISE_EDGE:
      if ((pulse1Freq > 0) && (((PORTL && 0x02) == 0) || (inj1Cntrl > 0)) && (pulse1Rem > 0)) //PL1 is solenoid 1
      {
        OCR1A = pulse1OCRA;
        pulse1State = FALL_EDGE;
        P1_RISE;//generates rising edge of a pulse at output
        pulse1Count += 1Ul;
        pulse1Rem -= 1Ul;
      }
      else
        OCR1A = 0x0c35;// a default value
      break;

    case FALL_EDGE:
      pulse1State = RISE_EDGE;//generates falling edge of a pulse at output
      P1_FALL;
      break;
  }
}
/***************************************************************************************/
#define P2_RISE   PORTB |= 0x40;//set PB6 high(12)
#define P2_FALL   PORTB &= 0xbf;//set PB5 high(11); 

ISR(TIMER3_COMPA_vect)
{
  //static int local_pulse_delay;
  //interrupt flag automatically cleared upon interrupt execution
  switch (pulse2State)
  {
    case RISE_EDGE:
      if ((pulse2Freq > 0) && (((PORTL && 0x01) == 0x00) || (inj2Cntrl > 0)) && (pulse2Rem > 0)) //PL0 is solenoid 2
      {
        OCR3A = pulse2OCRA; //used to set frequency
        pulse2State = FALL_EDGE;//setup for next time around
        P2_RISE;//generates rising edge of a pulse at output
        pulse2Count += 1Ul;
        pulse2Rem -= 1Ul;
      }
      else
        OCR3A = 0x0c35;// a default value
      break;

    case FALL_EDGE:
      pulse2State = RISE_EDGE;
      P2_FALL;//generates falling edge of a pulse output
      break;


  }
}
/***************************************************************************************/
#define P3_RISE   PORTE |= 0x08;//set PE4 high;
#define P3_FALL   PORTE &= 0xf7;// clear PE4

ISR(TIMER4_COMPA_vect)
{
  //static int local_pulse_delay;
  //interrupt flag automatically cleared upon interrupt execution
  switch (pulse3State)
  {
    case RISE_EDGE:
      if ((pulse3Freq > 0) && (((PORTD && 0x01) == 0x00) || (inj3Cntrl > 0)) && (pulse3Rem > 0)) //PD0 is inj3 solenoid
      {
        OCR4A = pulse3OCRA; //used to set frequency
        pulse3State = FALL_EDGE;//setup for next time around
        P3_RISE;//generates rising edge of a pulse at output
        pulse3Count += 1Ul;
        pulse3Rem -= 1Ul;
      }
      else
        OCR4A = 0x0c35;// a default value
      break;

    case FALL_EDGE:
      pulse3State = RISE_EDGE;
      P3_FALL;//generates falling edge of a pulse output
      break;


  }

}
/****************************************************************************************/


#define P4_RISE   PORTE |=0x10;//set PE3 high
#define P4_FALL   PORTE &= 0xef;//clear PE3

ISR(TIMER5_COMPA_vect)
{
  //static int local_pulse_delay;
  //interrupt flag automatically cleared upon interrupt execution
  switch (pulse4State)
  {
    case RISE_EDGE:
      if ((pulse4Freq > 0) && (((PORTD && 0x02) == 0x00) || (inj4Cntrl > 0)) && (pulse4Rem > 0)) //PD1 is solenoid 4
      {
        OCR5A = pulse4OCRA; //used to set frequency
        pulse4State = FALL_EDGE;//setup for next time around
        P4_RISE;//generates rising edge of a pulse at output
        pulse4Count += 1Ul;
        pulse4Rem -= 1Ul;
      }
      else
        OCR5A = 0x0c35;// a default value
      break;

    case FALL_EDGE:
      pulse4State = RISE_EDGE;
      P4_FALL;//generates falling edge of a pulse output
      break;

  }

}
