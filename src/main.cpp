#include <Arduino.h>
#include <HID-Project.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include "main.hpp"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ClickEncoder.h>
#include <TimerOne.h>

#define PIN_BUZZER (4) // buzzer
#define PIN_ROTARY_LEFT (20)
#define PIN_ROTARY_RIGHT (14) // rotary B
#define PIN_ROTARY_CLICK (8)  // rotary click
#define PIN_KEY_1 (7)         // key 1
#define PIN_KEY_2 (9)         // key 2
#define PIN_MENU (10)         // menu toggle

// Key switches
const int keysCount = 2;
Button keySwitches[keysCount] = {Button(PIN_KEY_1), Button(PIN_KEY_2)};
uint32_t lastPressTimes[2] = {0, 0};

// Oled display
#define DISPLAY_ROTATION 0
Adafruit_SSD1306 display(128, 32, &Wire, -1);

// Rotary control
ClickEncoder rotary(PIN_ROTARY_LEFT, PIN_ROTARY_RIGHT, PIN_ROTARY_CLICK);
void timerIsr()
{
  rotary.service();
}

// Current keycode
int keycode = KEY_ESC;

// Modes
#define MODE_COUNT 6
#define MENU_MODE (0)
#define KEY_BOUNDARY 0xa4
int mode = MENU_MODE + 2;
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
uint16_t modeKeys[][5] = {
    // Menu
    {},
    // Normal
    {},
    // Read
    {KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_HOME, KEY_PAGE_UP, KEY_PAGE_DOWN},
    // Media
    {MEDIA_VOLUME_DOWN, MEDIA_VOLUME_UP, MEDIA_VOLUME_MUTE, MEDIA_PLAY_PAUSE, MEDIA_VOLUME_MUTE},
    // Screen
    {CONSUMER_BRIGHTNESS_DOWN, CONSUMER_BRIGHTNESS_UP, CONSUMER_SCREENSAVER, KEY_F14, KEY_F15},
    // Record
    // {KEY_COMMA, KEY_PERIOD, 0, KEY_R, KEY_SPACE},
};

// Menu button
Button menuToggle(PIN_MENU);
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

  pinMode(PIN_BUZZER, OUTPUT);

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
      pressKey(tmpCode);
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
        releaseKey(tmpCode);
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
    writeKey(keycode);
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
        writeKey(modeKeys[mode][0]);
      }
    }
    else
    {
      for (int i = 0; i < inc; i++)
      {
        writeKey(modeKeys[mode][1]);
      }
    }
  }

  // handle switches
  for (int i = 0; i < keysCount; i++)
  {
    keySwitches[i].read();
    if (keySwitches[i].wasPressed())
    {
      pressKey(modeKeys[mode][3 + i]);
    }
    if (keySwitches[i].wasReleased())
    {
      releaseKey(modeKeys[mode][3 + i]);
    }
  }
}

void pressKey(uint16_t key)
{
  if (key < KEY_BOUNDARY)
  {
    Keyboard.press((KeyboardKeycode)key);
  }
  else
  {
    Keyboard.press((ConsumerKeycode)key);
  }
  printKey(key, "|");
}

void releaseKey(uint16_t key)
{
  if (key < KEY_BOUNDARY)
  {
    Keyboard.release((KeyboardKeycode)key);
  }
  else
  {
    Keyboard.release((ConsumerKeycode)key);
  }
  printKey(key, "");
}

void writeKey(uint16_t key)
{
  pressKey(key);
  releaseKey(key);
}

void buzzTone(unsigned int freq)
{
  Serial.println("Beep");
  tone(PIN_BUZZER, freq, 20);
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

void printKey(uint16_t key, const char *info)
{
  char str[10];
  sprintf(str, "Ox %X %s", key, info);
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