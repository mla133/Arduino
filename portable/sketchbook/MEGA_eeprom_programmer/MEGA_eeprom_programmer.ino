// AT28C EEPROM Programmer using Arduino MEGA

const int ADDR_BUS_WIDTH = 15;  //width in bits of address bus (AT28C64 has 13, AT28C256 has 15)
const int A[] = { 29, 30, 31, 28, 27, 26, 25, 24, 35, 34, 32, 33, 23, 36, 22 };  //address bus
const int I[] = { 13, 12, 11, 41, 40, 39, 38, 37 }; //data bus
const int WE = 44;  //write-enable (LOW active)
const int OE = 45;  //output-enable (LOW active)

void setup() {  
  Serial.begin(57600);
  digitalWrite(WE, HIGH);
  pinMode(WE, OUTPUT);
  pinMode(OE, OUTPUT);

  // Set address bus to outputs
  for (int i = 0; i < ADDR_BUS_WIDTH; i++) {
    pinMode(A[i], OUTPUT);
  }
}

void loop() 
{
  //printContents();
  delay(1000);
}

/************ FUNCTIONS BELOW ************/

// Set data bus to INPUT or OUTPUT
void setDataBusMode(int mode) {
  if (mode == INPUT || mode == OUTPUT) {
    for (int i=0; i < 8; i++) {
      pinMode(I[i], mode);
    }
  }
}

// Write an address to the address bus
void setAddress(int addr) {
  for (int i=0; i < ADDR_BUS_WIDTH; i++) {
    int a = (addr & (1 << i)) > 0;
    digitalWrite(A[i], a);
  }
}
//
//// Read a byte from the EEPROM at the specified address.
//byte readEEPROM(int address) {
//  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
//    pinMode(pin, INPUT);
//  }
//  setAddress(address, /*outputEnable*/ true);
//
//  byte data = 0;
//  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
//    data = (data << 1) + digitalRead(pin);
//  }
//  return data;
//}
//
///*
// * Write a byte to the EEPROM at the specified address.
// */
//void writeEEPROM(int address, byte data) {
//  setAddress(address, /*outputEnable*/ false);
//  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
//    pinMode(pin, OUTPUT);
//  }
//
//  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
//    digitalWrite(pin, data & 1);
//    data = data >> 1;
//  }
//  digitalWrite(EEPROM_WE, LOW);
//  delayMicroseconds(1);
//  digitalWrite(EEPROM_WE, HIGH);
//  delay(10);
//}
//
///*
// * Read the contents of the EEPROM and print them to the serial monitor.
// */
//void printContents() {
//  Serial.println("Reading EEPROM");
//  for (int base = 0; base <= 255; base += 16) {
//    byte data[16];
//    for (int offset = 0; offset <= 15; offset += 1) {
//      data[offset] = readEEPROM(base + offset);
//    }  
//    
//    char buf[80];
//    sprintf(buf, "%03X:  %02X %02X %02X %02X %02X %02X %02X %02X   %02X %02X %02X %02X %02X %02X %02X %02X",
//            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
//            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
//
//    Serial.println(buf);
//  }
//}
//
//void eraseEEPROM(int beginaddress, int endaddress) {
//  // Erase entire EEPROM
//  Serial.print("Erasing EEPROM");
//  for (int address = beginaddress; address <= endaddress; address += 1) {
//    writeEEPROM(address, 0xff);
//
//    if (address % 64 == 0) {
//      Serial.print(".");
//    }
//  }
//  Serial.println(" done");  
//}
//
//void programEEPROM() {
//  // Program data bytes
//  byte data[] = { 0x81, 0xcf, 0x92, 0x86, 0xcc, 0xa4, 0xa0, 0x8f, 0x80, 0x84, 0x88, 0xe0, 0xb1, 0xc2, 0xb0, 0xb8 };
//  
//  Serial.print("Programming EEPROM");
//  for (int address = 0; address < sizeof(data); address += 1) {
//    writeEEPROM(address, data[address]);
//
//    if (address % 64 == 0) {
//      Serial.print(".");
//    }
//  }
//  Serial.println(" done");  
//}

// 4-bit hex decoder for common anode 7-segment display
// byte data[] = { 0x81, 0xcf, 0x92, 0x86, 0xcc, 0xa4, 0xa0, 0x8f, 0x80, 0x84, 0x88, 0xe0, 0xb1, 0xc2, 0xb0, 0xb8 };

// 4-bit hex decoder for common cathode 7-segment display
// byte data[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47 };

// arduino (sticker) byte array as verified directly from probing outputs
// byte data[] = { 0xfc, 0x60, 0xda, 0xf2, 0x66, 0xb6, 0xbe, 0xe0, 0xfe, 0xe6, 0xee, 0x3e, 0x9c, 0x7a, 0x9e, 0x8e };

//#define EEPROM_A0 29
//#define EEPROM_A1 30
//#define EEPROM_A2 31 
//#define EEPROM_A3 28 
//#define EEPROM_A4 27
//#define EEPROM_A5 26 
//#define EEPROM_A6 25 
//#define EEPROM_A7 24 
//#define EEPROM_A8 35
//#define EEPROM_A9 34
//#define EEPROM_A10 32
//#define EEPROM_A11 33 
//#define EEPROM_A12 23
//#define EEPROM_A13 36 
//#define EEPROM_A14 22

//#define EEPROM_D0 13
//#define EEPROM_D1 12
//#define EEPROM_D2 11
//#define EEPROM_D3 41
//#define EEPROM_D4 40
//#define EEPROM_D5 39
//#define EEPROM_D6 38
//#define EEPROM_D7 37

//void setAddress(int address, bool outputEnable) {
//  digitalWrite(EEPROM_A0,  ((address&1)?HIGH:LOW));
//  digitalWrite(EEPROM_A1,  ((address&2)?HIGH:LOW));
//  digitalWrite(EEPROM_A2,  ((address&4)?HIGH:LOW));
//  digitalWrite(EEPROM_A3,  ((address&8)?HIGH:LOW));
//  digitalWrite(EEPROM_A4,  ((address&16)?HIGH:LOW));
//  digitalWrite(EEPROM_A5,  ((address&32)?HIGH:LOW));
//  digitalWrite(EEPROM_A6,  ((address&64)?HIGH:LOW));
//  digitalWrite(EEPROM_A7,  ((address&128)?HIGH:LOW));
//  digitalWrite(EEPROM_A8,  ((address&256)?HIGH:LOW));
//  digitalWrite(EEPROM_A9,  ((address&512)?HIGH:LOW));
//  digitalWrite(EEPROM_A10, ((address&1024)?HIGH:LOW));
//  digitalWrite(EEPROM_A11, ((address&2048)?HIGH:LOW));
//  digitalWrite(EEPROM_A12, ((address&4096)?HIGH:LOW));
//  digitalWrite(EEPROM_A13, ((address&8192)?HIGH:LOW));
//  digitalWrite(EEPROM_A14, ((address&16384)?HIGH:LOW));
//}
