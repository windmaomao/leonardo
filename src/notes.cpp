#include "notes.hpp"

const int keys[] = {'a', 'w', 's', 'e', 'd', 'f', 't', 'g', 'y', 'h', 'u', 'j', 'k'};
const int notes[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523};

int getKeyNote(int key)
{
  unsigned int i;
  for (i = 0; i < sizeof(keys); i++)
  {
    if (key == keys[i])
    {
      return notes[i];
    }
  }

  return 0;
}
