struct Keycode
{
  KeyboardKeycode keyboard;
  ConsumerKeycode consumer;
};

void buzzTone(unsigned int freq);
void displayText(const char *text);
void printKey(int key);
void printKey(int key, const char *info);
void sendKey(Keycode k);
void sendKey(Keycode k, bool release);
void sendKey(int key);
void sendKey(int key, bool release);
void setupSettings();
void loopMenuMode();
void loopNormalMode();
void loopGenericMode();
void printMode(int m);
void selectMode(int m);
void cancelMode();
void displayMenu(const char *text);