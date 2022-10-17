#include "pdmAudio.h"

pdmAudio pdm;

void setup() {
  pdm.begin(14);
  pdm.USB_UAC();
}

void loop() {
  // look up table sine-wave at 480 Hz
  int16_t left = pdm.sine_lu(480);
  int16_t right = left;
  // write  stereo data to computer
  pdm.USBtransfer(left, right);
}