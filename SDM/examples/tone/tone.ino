#include "pdmAudio.h"

// PDM object
pdmAudio pdm;

void setup() {
  pdm.begin(14);
}

void loop() {
  //each tone is governed by a frequency and duration(seconds)
  // zero frequency keeps voltage constant
  pdm.tone(262, .25);
  pdm.tone(0, .165);

  pdm.tone(196, 0.125);
  pdm.tone(0, .08);

  pdm.tone(196, 0.125);
  pdm.tone(0, .08);

  pdm.tone(220, 0.25);
  pdm.tone(0, .16);

  pdm.tone(196, 0.25);
  pdm.tone(0, .16);
  
  pdm.tone(0, 0.25);
  pdm.tone(0, .16);
  
  pdm.tone(247, 0.25);
  pdm.tone(0, .16);

  pdm.tone(262, 0.25);
  pdm.tone(0, 1);

}
