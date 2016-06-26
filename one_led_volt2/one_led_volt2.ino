/*
one led voltmeter or 'the voltmeter of the poor'
originally designed to measure the battery voltage drop while running
arduino in stand alone mode
the only hardware requirement is a divider bridge: the values will work for
divider by two (10k ohms between Vin and analog in 0, and 10k ohms between
analog in 0 and ground
count long lasting flashes for volts and short lasting flashes for hundreths 
of volts
the routine will also print value on serial com port

no rights, no warranty, no claim just fun
didier longueville, december 2007
*/
int ledPin = 13;           
int inputVoltagePin = 0;               

void setup(){
  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
  Serial.begin(9600);
}

void loop(){
  // read voltage value
  int voltage=analogRead(inputVoltagePin);
  int valueVolts=voltage/204;
  int valueHundrethsOfMilliVolts=((voltage % 204)*10)/204;
  // send formated value to serial com port
  Serial.print(valueVolts);
  Serial.print('.');
  Serial.print(valueHundrethsOfMilliVolts);
  Serial.println('V');
  // flash volts
  for(int i=0;i<valueVolts;i++){
    digitalWrite(ledPin,HIGH);  
    delay(500);                
    digitalWrite(ledPin,LOW);    
    delay(500);                  
  }
  delay(1000);                  
  // flash hundreths of millivolts
  for(int i=0;i<valueHundrethsOfMilliVolts;i++){
    digitalWrite(ledPin,HIGH);  
    delay(100);                
    digitalWrite(ledPin,LOW);    
    delay(500);                  
  }
  // pause between readings
  delay(5000);                  
}
