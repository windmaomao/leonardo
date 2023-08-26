#include <Arduino.h>
#include <Keyboard.h>
#include <JC_Button.h>
#include "main.hpp"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Buzz pin
int buzzPin = 4;
// Switch pin
Button keySwitch(7);
// Rotary pins
Button leftSpin(6);
Button rightSpin(5);
// Oled display
Adafruit_SSD1306 display(128, 32, &Wire, -1);
// Keycode
int keycode = KEY_ESC;

void setup()
{
  Serial.begin(9600);
  Keyboard.begin();
  keySwitch.begin();
  pinMode(buzzPin, OUTPUT);
  leftSpin.begin();
  rightSpin.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displayText("Ready.");
}

void printKey(int key)
{
  char str[10];
  sprintf(str, "%c %d", key, key);
  displayText(str);
}

void loop()
{
  // handle key press
  keySwitch.read();
  if (keySwitch.wasPressed())
  {
    press(keycode);
    printKey(keycode);
  }

  // handle rotary spin
  bool l = leftSpin.read();
  bool r = rightSpin.read();
  if (leftSpin.wasReleased() || leftSpin.wasPressed())
  {
    if (l == r)
    {
      keycode--;
      buzzTone(2000);
    }
    else
    {
      keycode++;
      buzzTone(1000);
    }
    printKey(keycode);
  }
}

void press(int key)
{
  Serial.println(key);
  Keyboard.write(key);
  buzzTone(key);
}

void buzzTone(unsigned int freq)
{
  tone(buzzPin, freq, 20);
}

void displayText(const char *text)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.println(text);
  display.display();
}