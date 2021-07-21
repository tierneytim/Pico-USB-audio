#include "mbed.h"
#include "USBAudio.h"

#include "pdm.pio.h"
#include "pico/multicore.h"
#include "SDM.h"

PIO pio = pio1;
uint sm;

void core1_worker() {
  uint32_t a = 0;
  int16_t pinput = 0;

  SDM sdm;
  
  while (1) {
    //if fifo empty modulate the previous value to keep voltage constant
    // helps prevents clicks and pops I think

    if (pio_sm_is_tx_fifo_empty(pio, sm)) {
      pio->txf[sm] = sdm.o4_os32(pinput);
    }
    // if other core sends value
    if (multicore_fifo_rvalid()) {
      uint32_t rec = multicore_fifo_pop_blocking();
      
      //save previous value so we can stuff buffer with it if music paused.
      pinput = (int16_t)(rec);
      a = sdm.o4_os32(pinput);

      //write to state PIO when its not full
      while (pio_sm_is_tx_fifo_full(pio, sm)) {}
      pio->txf[sm] = a;
    }
  }
}




void setup() {
  // less noisy power supply
  _gpio_init(23);
  gpio_set_dir(23, GPIO_OUT);
  gpio_put(23, 1);


  // Set the appropriate clock
  set_sys_clock_khz(153600, false);
  uint offset = pio_add_program(pio, &pdm_program);
  sm = pio_claim_unused_sm(pio, true);
  pdm_program_init(pio, sm, offset, 14, 153600 * 1000 / (48000.0 * 32));

  USBAudio audio(true, 48000, 2, 48000, 2);


  static uint8_t buf[64];
  int16_t *in;
  int16_t outL;
  int16_t outR;
  int16_t mono;
  int16_t pmono;

  multicore_launch_core1(core1_worker);
  delay(2000);

  while (1) {
    if (audio.read(buf, sizeof(buf))) {
      in = (int16_t *)buf;
      for (int i = 0; i < 16; i++) {
        // the left value;
        outL = *in;
        in++;
        // the right value
        outR = *in;
        in++;
        //mono value
        mono = (outL + outR) / 2;   
        multicore_fifo_push_blocking((uint32_t)(mono));
        // save previous value in case we need to stuff buffer
        pmono=mono;
      }
    } else {
      // basically if you havent read anything stuff buffer with previous value
       multicore_fifo_push_blocking((uint32_t)(pmono));
      }
  }
}

void loop() {
}
