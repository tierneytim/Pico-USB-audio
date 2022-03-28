#ifndef PDMDAC_H
#define PDMDAC_H
#include "Arduino.h"

class SDM {
public:
  //Constructor
  SDM();
  uint32_t o1_os32(int16_t sig); 
  uint32_t o2_os32(int16_t sig); 
  uint32_t o4_os32(int16_t sig);
  uint32_t o4_os32_df2(int16_t sig); 
  void sine_set(float freq);
  uint32_t sine_mod();

private:
  int32_t buff[8] = { 0, 0, 0, 0 , 0, 0, 0, 0 };
  int32_t w[4] = { 0, 0, 0, 0};
  int32_t w64[4] = { 0, 0, 0, 0};
  uint32_t step=1;
  uint32_t nsamps;
  int16_t sina[8000];
  uint32_t currentStep=0;
};


#endif