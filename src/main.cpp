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
Button leftSpin(3);
Button rightSpin(2);
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
  leftSpin.begin();
  rightSpin.begin();
  Keyboard.begin();
}

void process()
{
  light();
  buzz();
}

void loop()
{
  // handle serial input
  if (Serial.available() > 0)
  {
    serialIn = Serial.read();
    keycode = serialIn;
    process();
  }

  // handle key press
  keySwitch.read();
  if (keySwitch.wasReleased()) {
    Serial.println("key");
    Serial.println(keycode);
    serialIn = keycode;
    Keyboard.write(keycode);
    process();
  }

  // handle rotary spin
  bool l = leftSpin.read();
  bool r = rightSpin.read();
  if (leftSpin.wasReleased() || leftSpin.wasPressed()) {
    if (l == r) {
      keycode--;
    } else {
      keycode++;
    }
    Serial.println(keycode);
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
