#ifndef PDMAUDIO_H
#define PDMAUDIO_H

#include "Arduino.h"

class pdmAudio {
public:
  //Constructor
  pdmAudio();
  void begin(uint pin);
  void write(int16_t mono);
private:
 

};


#endif