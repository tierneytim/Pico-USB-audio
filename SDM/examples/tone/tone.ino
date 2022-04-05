#include "pdmAudio.h"

// PDM object
pdmAudio pdm;

void setup() {
  // set pin 14 to output
  pdm.begin(14);
}

void loop() {
  //each tone is governed by a frequency and duration(seconds)
  pdm.tone(262, 0.25);
  delay(165);
  pdm.tone(196, 0.125);
  delay(80);
  pdm.tone(196, 0.125);
  delay(80);
  pdm.tone(220, 0.25);
  delay(160);
  pdm.tone(196, 0.25);
  delay(160);
  pdm.tone(0, 0.25);
  delay(160); 
  pdm.tone(247, 0.25);
  delay(160);
  pdm.tone(262, 0.25);
  delay(1000);
}
