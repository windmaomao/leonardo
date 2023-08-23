#include <Arduino.h>
#include <Keyboard.h>
#include <JC_Button.h>
#include "main.hpp"
#include "notes.hpp"

// Serial byte
int serialIn;
// Light array
uint8_t lightsOn;
// Buzz pin
int buzzPin = 4;
// Switch pin
Button keySwitch(7);
// Rotary pins
int leftPin = 3;
int rightPin = 2;
int lastEncoder;
// Previous time
unsigned long prevTime = 0;
// Keycode
char keycode = 'A';

void setup()
{
  DDRB = 0xff;
  Serial.begin(9600);
  keySwitch.begin();
  pinMode(buzzPin, OUTPUT);
  // Button pin
  // pinMode(buttonPin, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(buttonPin), click, CHANGE);
  // Rotary pins
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
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
  keySwitch.read();
  if (keySwitch.wasReleased()) {
    serialIn = keycode;
    Keyboard.write(keycode);
    process();
  }

  if (Serial.available() > 0)
  {
    serialIn = Serial.read();
    keycode = serialIn;
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
