#include <Arduino.h>
#include <Keyboard.h>
#include "main.hpp"
#include "notes.hpp"

// Incoming serial data
int serialIn;
// Current lights
uint8_t lightsOn;
// Buzz pin
int buzzPin = 4;
// Interrupt pin
int buttonPin = 7;
// Previous time
unsigned long prevTime = 0;

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
  attachInterrupt(digitalPinToInterrupt(buttonPin), click, CHANGE);
  // Keyboard usb
  Keyboard.begin();
}

void process()
{
  light();
  buzz();
}

void loop()
{
  if (Serial.available() > 0)
  {
    serialIn = Serial.read();
    process();
  }

  unsigned long currTime = millis();
  if (currTime - prevTime >= 100)
  {
    prevTime = currTime;
    serialIn = serialIn >> 1;
    process();
  }
}

void light()
{
  lightsOn = (serialIn & 0xff) << 1;

  PORTB = lightsOn;
}

void buzz()
{
  noTone(buzzPin);
  if (serialIn < 5)
    return;

  int note = getKeyNote(serialIn);
  if (note > 0)
  {
    tone(buzzPin, note, 400);
  }
  else
  {
    tone(buzzPin, serialIn << 3, 200);
  }
}

void click()
{
  if (digitalRead(buttonPin) == LOW)
  {
    serialIn = serialIn >> 1;
  }
  else
  {
    serialIn = serialIn << 1;
  }

  Serial.println('A');
  Keyboard.write('A');
  process();
}
