#include "SonceStepper.h"
#include <inttypes.h>
#include "WProgram.h"

SonceStepper::SonceStepper(
    uint16_t steps,
    uint8_t boost_pin, 
    uint8_t dir_pin, 
    uint8_t pulse_pin
)
{
    this->_pulse_pin   = pulse_pin;
    this->_dir_pin     = dir_pin; 
    this->_boost_pin   = boost_pin; 
    digitalWrite(this->_pulse_pin,LOW);
    digitalWrite(this->_dir_pin,LOW);
    digitalWrite(this->_boost_pin,HIGH);
    pinMode(this->_pulse_pin,OUTPUT);
    pinMode(this->_dir_pin,OUTPUT);
    pinMode(this->_boost_pin,OUTPUT);

    this->_steps = steps;

    this->_speed = 1;
    this->_direction=STEPFORWARD;
}

void SonceStepper::_doMath(void)
{
    uint32_t x;
    x = 60000000UL / this->_steps;
    x /=2; // dutycycle 50%
    x /= this->_speed;

    this->_stepdelay = x;
}

void SonceStepper::setDirection(uint8_t direction)
{
    this->_direction = direction;
    this->_doMath();
}

void SonceStepper::setSpeed(uint16_t rpm)
{
    this->_speed = rpm;
    this->_doMath();
}

void SonceStepper::turnRevs(uint8_t revolutions)
{
    int i;
    for (i=0; i<revolutions;i++)
        this->Step(this->_steps);
}

void SonceStepper::Step(long count)
{
    uint32_t n;
    for (n=0; n < count; n++)
    {
        digitalWrite(this->_dir_pin,this->_direction);
        digitalWrite(this->_pulse_pin,HIGH);
        delayMicroseconds(this->_stepdelay);

        digitalWrite(this->_pulse_pin,LOW);
        digitalWrite(this->_dir_pin,LOW);
        delayMicroseconds(this->_stepdelay);
    }
}

