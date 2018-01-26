#include "Queue.h"

void setup() 
{
	pinMode(13, OUTPUT);
	Serial.begin(9600);
	Serial.println("Alive");

	Queue myQueue;
	myQueue.scheduleFunction(testFunction, "Test", 1000, 2000);
	myQueue.scheduleFunction(testFunction2, "Test", 1000, 1000);

	while(1) 
	{
		myQueue.Run(millis());
		delay(10);
	}
}

int testFunction(unsigned long now)
{
	Serial.print("test1 (1000,2000): ");
	Serial.println(now);
}

int testFunction2(unsigned long now)
{
	Serial.print("test2 (1000,1000): ");
	Serial.println(now);
}
void loop() 
{
}
