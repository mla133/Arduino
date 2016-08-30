void setup() 
{
  Serial.begin(9600);
}

float u, y;

void loop() 
{
  u = (float)analogRead(0)*5/1024; // Read Analog 0
  y = 0.945*y+0.0549*u;  // Filtered
  Serial.print(u);
  Serial.print("\t");
  Serial.print(y);
  Serial.print("\t");
  Serial.print(millis());
  Serial.println();
  delay(100);
}
