#ifndef PDMDAC_H
#define PDMDAC_H
#include "Arduino.h"

class SDM {
public:
  //Constructor
  SDM();
  // oversample X 32
  uint32_t o4_os32(int16_t sig) {

    uint32_t out = 0;
    int32_t d11 = -98304 - sig;
    int32_t d1 = d11*128;
    

    for (uint8_t j = 0; j < 32; j++) {
      // the feedback is composed of deltas(digital output - digital input) and errors
      // this could probably be more efficient and is the bottleneck
      int32_t deltas = -409 * d[0] - 273 * d[2] + 498 * d[1] + 57 * d[3];
      int32_t ecmp = (e[0] + e[2]) * 4 - 6 * e[1] - e[3];
      int32_t etmp = d1 + deltas + ecmp;
   
      // update values
      e[3] = e[2];
      e[2] = e[1];
      e[1] = e[0];
      e[0] = etmp;
      d[3] = d[2];
      d[2] = d[1];
      d[1] = d[0];
      d[0] = d11;

      // checks if current error minises sum of squares error
      // if not it changes the deltas and errors.
      if (etmp < 0) {
        e[0] += 25165824;
        d[0] += 196608;
        out += (1 << j);
      }
    }
   
    return out;
  }



private:
  int32_t e[4] = { 0, 0, 0, 0 };
  int32_t d[4] = { 0, 0, 0, 0 };
};


#endif