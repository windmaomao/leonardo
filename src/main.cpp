#include <Arduino.h>
#include "main.hpp"

// Incoming serial data
int serialIn;
// Current lights
uint8_t lightsOn;
// Buzz pin
int buzzPin = 4;
// Interrupt pin
int buttonPin = 2;

void setup()
{
  // Ports for lights
  DDRB = 0xff;
  // Serial port
  Serial.begin(9600);
  // Buzz pin
  pinMode(buzzPin, OUTPUT);
  // Button pin
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), onClick, CHANGE);
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

void onLights()
{
  lightsOn = (serialIn & 0xff) << 1;

  PORTB = lightsOn;
}

void onBuzz()
{
  if (serialIn < 2)
    return;
  noTone(buzzPin);
  tone(buzzPin, serialIn << 3, 100);
}

void onClick()
{
  if (digitalRead(buttonPin) == LOW)
  {
    serialIn = serialIn >> 1;
  }
  else
  {
    serialIn = serialIn << 1;
  }
}
