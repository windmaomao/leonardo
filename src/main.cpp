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
#define PIN_5 (5)   // rotary A
#define PIN_6 (6)   // rotary B
#define PIN_7 (7)   // key 1
#define PIN_8 (8)   // rotary click
#define PIN_9 (9)   // key 2
#define PIN_10 (10) // menu button

// Buzz pin
int buzzPin = PIN_4;

// Key switches
const int keysCount = 2;
Button keySwitches[keysCount] = {Button(PIN_7), Button(PIN_9)};
uint32_t lastPressTimes[2] = {0, 0};

// Oled display
Adafruit_SSD1306 display(128, 32, &Wire, -1);

// Rotary control
ClickEncoder rotary(PIN_5, PIN_6, PIN_8);
void timerIsr()
{
  rotary.service();
}

// Current keycode
int keycode = KEY_ESC;

// Page modes
#define MODE_COUNT 2
#define MENU_MODE (-1)
#define NORMAL_MODE (0)
#define MEDIA_MODE (1)
int mode = NORMAL_MODE;

// Menu button
Button modeSwitch(PIN_10);

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

  modeSwitch.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  printMode(mode);
}

void loop()
{
  // handle mode switch
  modeSwitch.read();
  if (modeSwitch.wasPressed())
  {
    mode++;
    if (mode >= 1)
    {
      mode = 0;
    }

    buzzTone(2000);
    printMode(mode);
  }

  // handle each mode
  switch (mode)
  {
  case 1:
    loopMediaMode();
    break;
  default:
    loopNormalMode();
  }
}

void loopNormalMode()
{
  // handle key switches
  char keyId[3];
  int tmpCode;
  for (int i = 0; i < keysCount; i++)
  {
    sprintf(keyId, "<%d>", i + 1);
    keySwitches[i].read();
    if (keySwitches[i].wasPressed())
    {
      tmpCode = EEPROM.read(i);
      sendKey(tmpCode, false);
      printKey(tmpCode, "|");
      lastPressTimes[i] = millis();
    }
    if (keySwitches[i].wasReleased())
    {
      // long press
      if (millis() - lastPressTimes[i] > 1000)
      {
        EEPROM.update(i, keycode);
        printKey(keycode, "o");
        buzzTone(500);
      }
      else
      {
        tmpCode = EEPROM.read(i);
        sendKey(tmpCode, true);
        printKey(tmpCode, "");
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

void loopMediaMode()
{
  // handle stop/resume button
  keySwitches[0].read();
  if (keySwitches[0].wasPressed())
  {
    Keyboard.write(201);
  }

  // handle mute
  keySwitches[1].read();
  if (keySwitches[1].wasPressed())
  {
    Keyboard.write(203);
  }

  // handle volume knob
  int inc = rotary.getValue();
  if (inc != 0)
  {
    if (inc < 0)
    {
      for (int i = 0; i > inc; i--)
      {
        Keyboard.write(204);
      }
    }
    else
    {
      for (int i = 0; i < inc; i++)
      {
        Keyboard.write(205);
      }
    }
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

void printKey(int key, const char *info)
{
  char str[10];
  sprintf(str, "%c %d %s", key, key, info);
  displayText(str);
}

void printMode(int m)
{
  switch (m)
  {
  case 1:
    displayText("2> MEDIA");
    break;
  default:
    displayText("1> NORMAL");
  }
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