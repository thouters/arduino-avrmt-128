#ifndef _SONCESTEP_H
#define _SONCESTEP_H
#include <stdint.h>
class SonceStepper 
{
public:
    SonceStepper(uint16_t steps, uint8_t boost_pin, uint8_t dir_pin, uint8_t pulse_pin);

    void setSpeed(uint16_t rpm);
    void setDirection(uint8_t direction);

    void Step(long count);
    void turnRevs(uint8_t revolutions);
    uint32_t _stepdelay; 
private:
    void _doMath(void);

    uint8_t _pulse_pin;
    uint8_t _dir_pin;
    uint8_t _boost_pin; 

    uint16_t _steps; 

    uint8_t _direction; 
    uint16_t _speed; 

};
#define STEPFORWARD 0
#define STEPBACKWARD 1
#define STEPNORMAL 0
#define STEPBOOST 1

#endif
