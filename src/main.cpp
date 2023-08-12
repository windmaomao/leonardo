#include <Arduino.h>

// Incoming serial data
int serialIn;
// Current lights
uint8_t lightsOn;
// Buzz pin
int buzzPin = 4;

void setup()
{
  // Ports for lights
  DDRB = 0xff;
  // Serial port
  Serial.begin(9600);
  // Buzz port
  pinMode(buzzPin, OUTPUT);
}

void onLights()
{
  lightsOn = (serialIn & 0xff) << 1;

  PORTB = lightsOn;
}

void onBuzz()
{
  if (serialIn > 35)
  {
    tone(buzzPin, serialIn << 2);
  }
  else
  {
    noTone(buzzPin);
  }
}

void loop()
{
  if (Serial.available() > 0)
  {
    serialIn = Serial.read();
  }

  onLights();
  onBuzz();
}