#include "SDM.h"
const uint16_t sampleRate = 48000;
int16_t sig[sampleRate];

void setup() {
delay(3000);

  //begin serial after clocks set
  Serial.begin(115200);
  Serial.println("hello");

  // signal to be modulated
  for (int i = 1; i < (sampleRate + 1); i++) {
    sig[i - 1] = (int16_t)(sin(2.0 * PI * 1000 * i / sampleRate) * 32767 * 1);
  }

  volatile uint32_t result;
  uint32_t end;
  uint32_t beg;
  // the class
  SDM sdm;


// direct form 1 order 2;
  beg = micros();
  for (uint16_t i = 0; i < sampleRate; i++) {
    result = sdm.o2_os32(sig[i]);
  }
  Serial.println(micros() - beg);


  // direct form 2;
  beg = micros();
  for (uint16_t i = 0; i < sampleRate; i++) {
    result = sdm.o4_os32_df2(sig[i]);
  }
  Serial.println(micros() - beg);

  // direct  form 1
  beg = micros();
  for (uint16_t i = 0; i < sampleRate; i++) {
    result = sdm.o4_os32(sig[i]);
  }
  Serial.println(micros() - beg);

  // direct  form 2 64Xoversampling
  volatile uint32_t result1;
  volatile uint32_t result2;

  SDM sdm2;
  beg = micros();
  for (uint16_t i = 0; i < sampleRate; i++) {
    result1 = sdm.o4_os32_df2(sig[i]);
    result2 = sdm.o4_os32_df2(sig[i]);
  }
  Serial.println(micros() - beg);
}

void loop() {
}