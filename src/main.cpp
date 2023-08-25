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
int keycode = KEY_ESC;

void setup()
{
  DDRB = 0xff;
  Serial.begin(9600);
  keySwitch.begin();
  pinMode(buzzPin, OUTPUT);
  leftSpin.begin();
  rightSpin.begin();
  Keyboard.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  print("Ready.");
}

void process()
{
  light((serialIn & 0xff) << 1);
  buzz(serialIn << 3);
}

void printKey(int key)
{
  char str[10];
  sprintf(str, "%c %d", key, key);
  print(str);
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
    }
    else
    {
      keycode++;
    }
    printKey(keycode);
  }

  unsigned long currTime = millis();
  if (currTime - prevTime >= 100)
  {
    prevTime = currTime;
    serialIn = serialIn >> 1;
    process();
  }
}

void light(uint8_t on)
{
  PORTB = on;
}

void buzz(unsigned int freq)
{
  tone(buzzPin, freq, 10);
}

void press(int key)
{
  Serial.println("key");
  Serial.println(key);
  Keyboard.write(key);
  serialIn = key;
}

void print(const char *text)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.println(text);
  display.display();
}