// vim:ft=cpp
#include <SonceStepper.h>

SonceStepper Stepper(1600, 5, 4,9);

const uint8_t relPin = 45; 
const uint8_t butUp = 51;

void setup() 
{
    pinMode(butUp,INPUT);
}

void loop() 
{
    uint16_t speed;
    if (digitalRead(butUp)==0)
    {
        Stepper.setDirection(STEPFORWARD);
        Stepper.setSpeed(60);
	Stepper.turnRevs(4);
 	for (speed=60; speed < 600; speed++)
	{
	        Stepper.setSpeed(speed);
        	Stepper.Step(400+(speed-59)*10);
	}
        Stepper.turnRevs(4);
 	for (; speed > 60; speed--)
	{
	        Stepper.setSpeed(speed);
        	Stepper.Step(400+(speed-59)*10);
	}
        Stepper.turnRevs(4);

        delay(1000);
    }
}
