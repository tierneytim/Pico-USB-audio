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
  
  //startup pop suppression
  for(int i =-32767;i<0;i++){
      a = sdm.o2_os32(i);
      while (pio_sm_is_tx_fifo_full(pio, sm)) {}
      pio->txf[sm] = a;
  }
  
  while (1) {
    //if fifo empty modulate the previous value to keep voltage constant
    // helps prevents clicks and pops I think

    if (pio_sm_is_tx_fifo_empty(pio, sm)) {
      pio->txf[sm] = a;
      pio->txf[sm] = a;
      pio->txf[sm] = a;
      pio->txf[sm] = a;
      pio->txf[sm] = a;
      pio->txf[sm] = a;
      pio->txf[sm] = a;
      pio->txf[sm] = a;
    }
    // if other core sends value
    //if (false) {
    if (multicore_fifo_rvalid()) {
      uint32_t rec = multicore_fifo_pop_blocking();

      //save previous value so we can stuff buffer with it if music paused.
      pinput = (int16_t)(rec);
      a = sdm.o4_os32_df2(pinput);
      //a = sdm.o1_os32(pinput);

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
  set_sys_clock_khz(115200, false);
  uint offset = pio_add_program(pio, &pdm_program);
  sm = pio_claim_unused_sm(pio, true);

  // PIO configuration
  pio_sm_config c = pdm_program_get_default_config(offset);
  sm_config_set_out_pins(&c, 14, 1);
  pio_gpio_init(pio, 14);
  pio_sm_set_consecutive_pindirs(pio, sm, 14, 1, true);
  
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&c, 115200 * 1000 / (48000.0 * 32));
  sm_config_set_out_shift(&c, true, true, 32);
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);

  // USB audio 
  USBAudio audio(true, 48000, 2, 48000, 2);
  uint8_t buf[96];
  int16_t *in;
  multicore_launch_core1(core1_worker);
  delay(2000);
  

  //
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
        multicore_fifo_push_blocking((uint32_t)(mono));
      }
    }
  }
}

void loop() {
}
