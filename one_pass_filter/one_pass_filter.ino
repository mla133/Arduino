float x,y;
float g,b;

void setup() {
  Serial.begin(9600);
  b=0.055;  // lowpass -> 0 < b < 1.0, highpass -> -1.0 < b < 0
}

void loop() 
{
  x=(float)analogRead(0)*5/1024;
  g=(1.0-abs(b));
  y=g*y+b*x;  
  Serial.print(x); 
  Serial.print("\t"); 
  Serial.print(y);
  Serial.print("\t");
  Serial.print(millis()); 
  Serial.println();

}
