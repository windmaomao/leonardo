#include <Arduino.h>

void setup()
{
  DDRB = 0xff;
}

void loop()
{
  PORTB = 0b01100000;
}