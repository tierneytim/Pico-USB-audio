#include "pdmAudio.h"

// PDM object
pdmAudio pdm;

void setup() {
  // set pin 14 to be the output
  pdm.begin(14);
  
  // intiate USB transfer
  pdm.USB();
}

void loop() {
   // write whatever is in the USB buffer to the PDM -DAC
   pdm.USBwrite();
}
