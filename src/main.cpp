#include <Arduino.h>
#include <Keyboard.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include "main.hpp"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ClickEncoder.h>
#include <TimerOne.h>

#define PIN_4 (4)   // buzzer
#define PIN_5 (20)  // rotary A
#define PIN_6 (14)  // rotary B
#define PIN_7 (7)   // key 1
#define PIN_8 (8)   // rotary click
#define PIN_9 (9)   // key 2
#define PIN_10 (10) // menu toggle

// Buzz pin
int buzzPin = PIN_4;

// Key switches
const int keysCount = 2;
Button keySwitches[keysCount] = {Button(PIN_7), Button(PIN_9)};
uint32_t lastPressTimes[2] = {0, 0};

// Oled display
#define DISPLAY_ROTATION 0
Adafruit_SSD1306 display(128, 32, &Wire, -1);

// Rotary control
ClickEncoder rotary(PIN_5, PIN_6, PIN_8);
void timerIsr()
{
  rotary.service();
}

// Current keycode
int keycode = KEY_ESC;

// Modes
#define MODE_COUNT 6
#define MENU_MODE (0)
int mode = MENU_MODE + 1;
int lastMode;
const char *modeLabels[] = {
    "MENU",
    "NORMAL",
    "READ",
    "MEDIA",
    "SCREEN",
    "RECORD"};
void (*modeLoops[])() = {
    loopMenuMode,
    loopNormalMode,
    loopGenericMode, // Read
    loopGenericMode, // Media
    loopGenericMode, // Screen
    loopGenericMode, // Record
};
int modeKeys[][5] = {
    // Menu
    {},
    // Normal
    {},
    // Read
    {KEY_UP_ARROW, KEY_DOWN_ARROW, 0, KEY_ESC, KEY_RETURN},
    // Media
    {KEY_F11, KEY_F12, 0, KEY_F8, KEY_F10},
    // Screen
    {KEY_F14, KEY_F15, 0, ' ', KEY_TAB},
    // Record
    {',', '.', 0, 'r', ' '},
};

// Menu button
Button menuToggle(PIN_10);
int menuSelect;

void setup()
{
  setupSettings();

  Serial.begin(9600);

  Keyboard.begin();
  for (int i = 0; i < keysCount; i++)
  {
    keySwitches[i].begin();
  }

  pinMode(buzzPin, OUTPUT);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  rotary.setAccelerationEnabled(1);

  menuToggle.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(DISPLAY_ROTATION);
}

void loop()
{
  // handle menu toggle
  menuToggle.read();
  if (menuToggle.wasPressed())
  {
    if (mode == MENU_MODE)
    {
      selectMode(menuSelect);
    }
    else
    {
      lastMode = mode;
      menuSelect = lastMode;
      selectMode(MENU_MODE);
    }
  }

  // handle each mode
  modeLoops[mode]();
}

void loopMenuMode()
{
  // handle key switches
  keySwitches[0].read();
  if (keySwitches[0].wasPressed())
  {
    selectMode(menuSelect);
  }
  keySwitches[1].read();
  if (keySwitches[1].wasPressed())
  {
    cancelMode();
  }

  // handle rotary
  int inc = rotary.getValue();
  if (inc != 0)
  {
    menuSelect += inc;
    menuSelect = constrain(menuSelect, 1, MODE_COUNT - 1);
    displayMenu(modeLabels[menuSelect]);
  }
}

void loopNormalMode()
{
  // handle key switches
  int tmpCode;
  for (int i = 0; i < keysCount; i++)
  {
    keySwitches[i].read();
    if (keySwitches[i].wasPressed())
    {
      tmpCode = EEPROM.read(i);
      sendKey(tmpCode, false);
      lastPressTimes[i] = millis();
    }
    if (keySwitches[i].wasReleased())
    {
      // long press
      if (millis() - lastPressTimes[i] > 1000)
      {
        EEPROM.update(i, keycode);
        buzzTone(500);
      }
      else
      {
        tmpCode = EEPROM.read(i);
        sendKey(tmpCode, true);
      }
    }
  }

  // handle rotary
  int inc = rotary.getValue();
  if (inc != 0)
  {
    keycode += inc;
    keycode = constrain(keycode, 1, 255);
    printKey(keycode, "");
  }

  ClickEncoder::Button b = rotary.getButton();
  if (b == ClickEncoder::Clicked)
  {
    Keyboard.write(keycode);
  }
}

void loopGenericMode()
{
  // handle volume knob
  int inc = rotary.getValue();
  if (inc != 0)
  {
    if (inc < 0)
    {
      for (int i = 0; i > inc; i--)
      {
        sendKey(modeKeys[mode][0]);
      }
    }
    else
    {
      for (int i = 0; i < inc; i++)
      {
        sendKey(modeKeys[mode][1]);
      }
    }
  }

  // handle switches
  for (int i = 0; i < keysCount; i++)
  {
    keySwitches[i].read();
    if (keySwitches[i].wasPressed())
    {
      sendKey(modeKeys[mode][3 + i], false);
    }
    if (keySwitches[i].wasReleased())
    {
      sendKey(modeKeys[mode][3 + i], true);
    }
  }
}

void sendKey(int key)
{
  Serial.println(key);
  Keyboard.write(key);
  printKey(key);
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
  printKey(key, release ? "" : "|");
}

void buzzTone(unsigned int freq)
{
  Serial.println("Beep");
  tone(buzzPin, freq, 20);
}

void displayText(const char *text)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 5);
  display.println(text);
  display.display();
}

void printKey(int key)
{
  char str[10];
  sprintf(str, "%c %d", key, key);
  displayText(str);
}
void printKey(int key, const char *info)
{
  char str[10];
  sprintf(str, "%c %d %s", key, key, info);
  displayText(str);
}

void printMode(int m)
{
  displayText(modeLabels[m]);
}

void cancelMode()
{
  mode = lastMode;
  displayText(modeLabels[mode]);
  buzzTone(500);
}

void selectMode(int m)
{
  mode = m;
  if (mode == MENU_MODE)
  {
    displayMenu(modeLabels[menuSelect]);
  }
  else
  {
    displayText(modeLabels[m]);
  }
  buzzTone(1500);
}

void displayMenu(const char *text)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("MENU");
  display.setCursor(0, 15);
  display.print("> ");
  display.println(text);
  display.display();
}

void setupSettings()
{
  int storedValue = EEPROM.read(0);
  if (storedValue == 0xff)
  {
    for (int i = 0; i < keysCount; i++)
    {

      EEPROM.update(i, keycode);
    }
  }
}