#ifndef PDMAUDIO_H
#define PDMAUDIO_H

#include "Arduino.h"
#include "mbed.h"
#include "USBAudio.h"


void core1_worker();
 
class pdmAudio {
public:
  //Constructor
  pdmAudio();
  void begin(uint pin);
  void write(int16_t mono);
  void USB();
  void USBwrite();
  void tone(uint32_t freq, float duration = 1.0);
  //uint32_t sine(uint32_t freq);

private:
  USBAudio* audio;
  uint8_t myRawBuffer[96];
  float dt = 1.0/48000.0;
  uint32_t step=1;
  uint32_t nsamps;
  int16_t sina[6000];
  uint32_t currentStep=0;
};

#endif