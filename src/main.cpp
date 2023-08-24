#include <Arduino.h>
#include <Keyboard.h>
#include <JC_Button.h>
#include "main.hpp"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Serial byte
int serialIn;
// Light array
uint8_t lightsOn;
// Buzz pin
int buzzPin = 4;
// Switch pin
Button keySwitch(7);
// Rotary pins
Button leftSpin(6);
Button rightSpin(5);
// Oled display
Adafruit_SSD1306 display(128, 32, &Wire, -1);
// Previous time
unsigned long prevTime = 0;
// Keycode
int keycode = 100;

void setup()
{
  DDRB = 0xff;
  Serial.begin(9600);
  keySwitch.begin();
  pinMode(buzzPin, OUTPUT);
  leftSpin.begin();
  rightSpin.begin();
  Keyboard.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.print(".");
  }
  print("Ready.");
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
  if (keySwitch.wasPressed())
  {
    Serial.println("key");
    Serial.println(keycode);
    serialIn = keycode;
    Keyboard.write(keycode);
    process();
  }

  // handle rotary spin
  bool l = leftSpin.read();
  bool r = rightSpin.read();
  if (leftSpin.wasReleased() || leftSpin.wasPressed())
  {
    if (l == r)
    {
      keycode--;
    }
    else
    {
      keycode++;
    }
    Serial.println("rotate");
    Serial.println(keycode);
    Keyboard.write(keycode);
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

  tone(buzzPin, serialIn << 3, 50);
}

void print(const char *text)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.println(text);
  display.display();
}