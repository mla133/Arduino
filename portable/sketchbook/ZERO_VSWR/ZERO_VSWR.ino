// UART bridge for data exchange between
// RigExpert AA-30 ZERO antenna & cable analyzer and Arduino Uno
//
// Receives from the Arduino, sends to AA-30 ZERO.
// Receives from AA-30 ZERO, sends to the Arduino.
//
// 26 June 2017, Rig Expert Ukraine Ltd.
//
#include <SoftwareSerial.h>

SoftwareSerial ZERO(4, 7); // RX, TX
//
// Modified by WB9RAA Ed March
// January 6, 2018
//
// talks to AA-30 Zero at 38,400 baud and
// make data suitable for a spreadsheet plot
//
// Sweep all ham bands in about 10K Freq. steps
// Computes SWR on the fly so each line of text
// sent is: "Freq,R,X,SWR \r\n"
// intermixed with OK and fq sw frx commands too
// Compiled with Arduino IDE 1.8.5
// total sweep time is about 25 seconds for 160M-10M bands.
//
// Faster plots with "frx3" on all bands so Only Edge and Center Freqs are printed.
// Other ideas, sweep 1..30 Mhz and only print F,R,X,SWR when swr is under 2.0
//

int j;
char HamBands[][35] = {
  { "\r\nver\r\n" } , //0
  { "fq1900000\r\nsw200000\r\nfrx20\r\n" }, //1

  { "fq3750000\r\nsw500000\r\nfrx50\r\n" }, //2

  { "fq5331900\r\nsw2800\r\nfrx3\r\n" }, //3
  { "fq5347900\r\nsw2800\r\nfrx3\r\n" }, //4
  { "fq5358900\r\nsw2800\r\nfrx3\r\n" }, //5
  { "fq5404900\r\nsw2800\r\nfrx3\r\n" }, //6

  { "fq7150000\r\nsw300000\r\nfrx35\r\n" }, //7

  { "fq10125000\r\nsw50000\r\nfrx10\r\n" }, //8

  { "fq14150000\r\nsw300000\r\nfrx30\r\n" }, //9

  { "fq18118000\r\nsw100000\r\nfrx10\r\n" }, //10

  { "fq21225000\r\nsw450000\r\nfrx45\r\n" }, //11

  { "fq24940000\r\nsw100000\r\nfrx10\r\n" }, //12

  { "fq28985000\r\nsw1970000\r\nfrx50\r\n" }, //13
  { "" }, // End marker //14
  //

};

void setup()
{
  ZERO.begin(38400); // init AA side UART
  ZERO.flush();
  ZERO.setTimeout(2500);
  Serial.begin(38400); // init PC side UART
  Serial.flush();
  delay(50);
  j = 0;
  Serial.println("\r\n\nThe AA-30Zero Project Ver.20180107A by Ed March WB9RAA\n");
  Serial.println("\nUsing the RigExpert 'AA-30 Zero' Antenna Analyzer\nScan all 1..30Mhz Ham bands.\nWhen done Press / to run again. Otherwise type commands to AA-30 Zero\n");
  Serial.println("AA-30 Zero Commands");
  Serial.println("ver : GET VERSION OF AA FIRMWARE");
  Serial.println("fq1234567 : SET CENTER FREQUENCY IN HERTZ ex. fq7150000");
  Serial.println("sw300000 : SET SWEEP RANGE IN HERTZ ex. sw300000 sweeps 300Khz from Begin To End");
  Serial.println("frx123 : START SWEEPING returning F,R,X Total of 123 lines. frx3 returns 3 lines BEGIN,CENTER,END\n");
}
long tmo = 250;

void loop()
{
  if (ZERO.available())
  {
    String s = ZERO.readStringUntil('\n');
    s.replace("\r", "");
    Serial.write(s.c_str()); // data stream from AA to PC

    int i = s.indexOf(',');
    if (i > 0)
    {
      // Parse string into floats R & X
      i++;
      float R = s.substring(i).toFloat();
      int ii = s.substring(i).indexOf(',');
      float X = s.substring(i + ii + 1).toFloat();
      //
      // Compute SWR from R & X
      //
      float XX = X * X;
      float Rm = (R - 50) * (R - 50);
      float Rp = (R + 50) * (R + 50);
      float N = sqrt(Rm + XX);
      float D = sqrt(Rp + XX);
      float G = N / D;
      float vswr = (1 + G) / (1 - G);
      // Since we can not print floats we get int & fraction as INT's
      // if swr is 4.12 then v1=4 and v2=12 -- 1.04 then 1 and 4 printed as 1.04 using %d.%02d
      int v1 = vswr;
      int v2 = (vswr - v1) * 100;
      if (v1 < 0)
      {
        v1 = 99;
        v2 = 0;
      }
      char z[50];
      sprintf(z, ",%d.%02d", v1, v2); // comput swr as string
      Serial.write(z); // append to string
    }
    Serial.write("\r\n"); // and terninate as it was with CR LF
    tmo = 250;
  }
  if (Serial.available())
  {
    char c1 = Serial.read();
    if (c1 == '/')
    {
      j = 0;
    }
    else
    {
      ZERO.write(c1); // data stream from PC to AA
    }
    tmo = 250;
  }
  delay(1);
  if (--tmo < 0)
  {
    if (HamBands[j][0] != 0)
    {
      for (int i = 0; HamBands[j][i] != 0; i++)
      {
        int ch = HamBands[j][i];
        Serial.write(ch);
        ZERO.write(ch);
        if (ch == '\n')
        {
          for (int d = 0; d < 50; d++)
          {
            delay(1);
            if (ZERO.available())
            {
              Serial.write(ZERO.read()); // data stream from AA to PC
            }
          }
        }
      }
      tmo = 250;
      j++;
    }
  }
}
