#include "pdmAudio.h"

pdmAudio pdm;
void setup() {
  // put your setup code here, to run once:
  pdm.begin(14);
  pdm.bluetooth();
}

void loop() {
  // put your main code here, to run repeatedly:
}