#include <Arduino.h>

// Incoming serial data
int serialIn;
// Current lights
uint8_t lightsOn;
// Buzz pin
int buzzPin = 4;
// Buzz keys & notes
const uint8_t keys[] = {'a', 'w', 's', 'e', 'd', 'r', 'f', 't', 'g'};
const int notes[] = {261, 277, 294, 311, 330, 349, 370, 392, 415};

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

void loop()
{
  if (Serial.available() > 0)
  {
    serialIn = Serial.read();
  }

  onLights();
}