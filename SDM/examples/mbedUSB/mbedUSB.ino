#include "mbed.h"
#include "USBAudio.h"
#include "pdmAudio.h"

void setup() {
  
  // USB audio
  USBAudio audio(true, 48000, 2, 48000, 2);
  uint8_t buf[96];
  int16_t *in;
  delay(2000);

  //setup pin 14 to outut
  pdmAudio pdm;
  pdm.begin(14);
  delay(2000);

  while (1) {
    if (audio.read(buf, sizeof(buf))) {
      in = (int16_t *)buf;
      for (int i = 0; i < 24; i++) {
        // the left value;
        int16_t outL = *in;
        in++;
        // the right value
        int16_t outR = *in;
        in++;
        //mono value
        int16_t mono = (outL + outR) / 2;
        pdm.write(mono);
      }
    }
  }
}

void loop() {
}
