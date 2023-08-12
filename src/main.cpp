#include <Arduino.h>

// Incoming serial data
int serialIn;
// Current lights
uint8_t lightsOn;

void setup()
{
  // Ports for lights
  DDRB = 0xff;
  // Serial port
  Serial.begin(9600);
}

void loop()
{
  // lightsOn = rand() % 256;

  if (Serial.available() > 0)
  {
    serialIn = Serial.read();
    lightsOn = (serialIn & 0xff) << 1;
  }

  PORTB = lightsOn;
}