#include <Arduino.h>
#include <Keyboard.h>
#include <EEPROM.h>
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
Button keySwitch2(9);
// Rotary pins
Button leftSpin(6);
Button rightSpin(5);
Button confirmButton(8);
// Oled display
Adafruit_SSD1306 display(128, 32, &Wire, -1);
// Keycode
int keycode = KEY_ESC;
// EEPROM address
int address = 0;

void setup()
{
  setupSettings();
  Serial.begin(9600);
  Keyboard.begin();
  keySwitch.begin();
  keySwitch2.begin();
  pinMode(buzzPin, OUTPUT);
  leftSpin.begin();
  rightSpin.begin();
  confirmButton.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displayText("Ready.");
}

void printKey(int key, const char *info)
{
  char str[10];
  sprintf(str, "%c %d %s", key, key, info);
  displayText(str);
}

void loop()
{
  // handle key press
  keySwitch.read();
  if (keySwitch.wasPressed())
  {
    keycode = EEPROM.read(0);
    sendKey(keycode, false);
    printKey(keycode, "|");
  }
  if (keySwitch.wasReleased())
  {
    sendKey(keycode, true);
    printKey(keycode, "");
    address = 0;
  }

  // handle key press 2
  keySwitch2.read();
  if (keySwitch2.wasPressed())
  {
    keycode = EEPROM.read(1);
    sendKey(keycode, false);
    printKey(keycode, "||");
  }
  if (keySwitch2.wasReleased())
  {
    sendKey(keycode, true);
    printKey(keycode, "");
    address = 1;
  }

  // handle rotary spin
  bool l = leftSpin.read();
  bool r = rightSpin.read();
  if (leftSpin.wasReleased() || leftSpin.wasPressed())
  {
    if (l == r)
    {
      keycode--;
      if (keycode < 0)
      {
        keycode = 255;
      }
      buzzTone(1000);
      printKey(keycode, "<-");
    }
    else
    {
      keycode++;
      if (keycode > 255)
      {
        keycode = 0;
      }
      buzzTone(2000);
      printKey(keycode, "->");
    }
  }

  // handle confirm button
  confirmButton.read();
  if (confirmButton.wasReleased())
  {
    EEPROM.update(address, keycode);
  }
}

void sendKey(int key, bool release)
{
  Serial.println(key);
  if (release)
  {

    Keyboard.release(key);
  }
  else
  {

    Keyboard.press(key);
  }
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

void setupSettings()
{
  int storedValue = EEPROM.read(0);
  if (storedValue == 0xff)
  {
    EEPROM.update(0, keycode);
    EEPROM.update(1, keycode);
  }
}