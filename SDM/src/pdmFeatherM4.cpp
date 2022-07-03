#include "pdmAudio.h"

#if defined(__SAMD51__)

#include "wiring_private.h"



pdmAudio::pdmAudio() {
  float delta = (2.0*PI/6000.0);
    for (int i=0;i<6000;i++){
        sina[i]=(int16_t)(sin(i*delta)*32767);
    }
}

void pdmAudio::begin(uint32_t pin) {
  pinPeripheral(11, PIO_I2S);

  I2S->CTRLA.bit.ENABLE = 0;

  // initialize clock control
  MCLK->APBDMASK.reg |= MCLK_APBDMASK_I2S;
  int mck_mult = 256;
  uint32_t fs = 48000;
  uint16_t num_slots = 1;
  uint32_t mckFreq = (fs * mck_mult);
  uint32_t sckFreq = fs * num_slots * (32);

  uint32_t gclkval = GCLK_PCHCTRL_GEN_GCLK1_Val;
  uint32_t gclkFreq = VARIANT_GCLK1_FREQ;
  uint8_t mckoutdiv = min((gclkFreq / mckFreq) - 1, 63);
  uint8_t mckdiv = min((gclkFreq / sckFreq) - 1, 63);

  if (((VARIANT_GCLK1_FREQ / mckFreq) - 1) > 63) {
    gclkval = GCLK_PCHCTRL_GEN_GCLK4_Val;
    gclkFreq = 12000000;
  }

  GCLK->PCHCTRL[I2S_GCLK_ID_0].reg = gclkval | (1 << GCLK_PCHCTRL_CHEN_Pos);
  GCLK->PCHCTRL[I2S_GCLK_ID_1].reg = gclkval | (1 << GCLK_PCHCTRL_CHEN_Pos);

  // software reset
  I2S->CTRLA.bit.SWRST = 1;
  while (I2S->SYNCBUSY.bit.SWRST || I2S->SYNCBUSY.bit.ENABLE) {}  // wait for sync

  // CLKCTRL[0] is used for the tx channel
  I2S->CLKCTRL[0].reg = I2S_CLKCTRL_MCKSEL_GCLK | I2S_CLKCTRL_MCKOUTDIV(mckoutdiv) | I2S_CLKCTRL_MCKDIV(mckdiv) | I2S_CLKCTRL_SCKSEL_MCKDIV | I2S_CLKCTRL_MCKEN | I2S_CLKCTRL_FSSEL_SCKDIV | I2S_CLKCTRL_BITDELAY_I2S | I2S_CLKCTRL_FSWIDTH_HALF | I2S_CLKCTRL_NBSLOTS(num_slots - 1) | I2S_CLKCTRL_SLOTSIZE(3);

  uint8_t wordSize = I2S_TXCTRL_DATASIZE_32_Val;


  I2S->TXCTRL.reg = I2S_TXCTRL_DMA_SINGLE | I2S_TXCTRL_MONO_STEREO | I2S_TXCTRL_BITREV_MSBIT | I2S_TXCTRL_EXTEND_ZERO | I2S_TXCTRL_WORDADJ_RIGHT | I2S_TXCTRL_DATASIZE(wordSize) | I2S_TXCTRL_TXSAME_ZERO | I2S_TXCTRL_TXDEFAULT_ZERO;
  I2S->RXCTRL.reg = I2S_RXCTRL_DMA_SINGLE | I2S_RXCTRL_MONO_STEREO | I2S_RXCTRL_BITREV_MSBIT | I2S_RXCTRL_EXTEND_ZERO | I2S_RXCTRL_WORDADJ_RIGHT | I2S_RXCTRL_DATASIZE(wordSize) | I2S_RXCTRL_SLOTADJ_RIGHT | I2S_RXCTRL_CLKSEL_CLK0 | I2S_RXCTRL_SERMODE_RX;

  while (I2S->SYNCBUSY.bit.ENABLE)
    ;  // wait for sync
  I2S->CTRLA.bit.ENABLE = 1;

  I2S->CTRLA.bit.CKEN0 = 1;
  while (I2S->SYNCBUSY.bit.CKEN0) {}

  I2S->CTRLA.bit.TXEN = 1;
  while (I2S->SYNCBUSY.bit.TXEN) {}
}

void pdmAudio::USB_UAC() {

}

void pdmAudio::USBwrite() {

}

void pdmAudio::write(int16_t mono) {
    uint32_t result = sdm.o4_os32_df2(mono);
    while ((!I2S->INTFLAG.bit.TXRDY0) || I2S->SYNCBUSY.bit.TXDATA) {}
    I2S->INTFLAG.bit.TXUR0 = 1;
    I2S->TXDATA.reg = result;
}

void pdmAudio::USBtransfer(int16_t left,int16_t right) {

}

int16_t pdmAudio::sine_lu(uint32_t freq){
  step = freq >> 3;
  currentStep = currentStep +step;
  if(currentStep>5999){
   currentStep = currentStep-6000;
  }
  int16_t val =  sina[currentStep];
  return val;
}

void pdmAudio::tone(uint32_t freq, float duration){
  step = freq >> 3;
  nsamps = (uint32_t)(duration/dt);
  for (uint32_t i = 0;i<nsamps;i++){
  currentStep = currentStep +step;
  if(currentStep>5999){
    currentStep = currentStep-6000;
  }
  int16_t val =  sina[currentStep];
  write(val);
  }
}


#endif