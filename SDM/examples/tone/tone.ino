#include "pdm.pio.h"
#include "SDM.h"
#include "pico/multicore.h"


PIO pio = pio1;
uint sm;
SDM sdm;

void pico_setup_48000_32(uint pin) {
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
  sm_config_set_out_pins(&c, pin, 1);
  pio_gpio_init(pio, pin);
  pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&c, 115200 * 1000 / (48000.0 * 32));
  sm_config_set_out_shift(&c, true, true, 32);
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);
}

void core1_worker() {
  
  volatile uint32_t a = 0;
  //startup pop suppression
  for (int i = -32767; i < 0; i++) {
    a = sdm.o2_os32(i);
    while (pio_sm_is_tx_fifo_full(pio, sm)) {}
    pio->txf[sm] = a;
  }
  sdm.sine_set(0);

  while (1) {
     if (multicore_fifo_rvalid()) {
      uint32_t rec = multicore_fifo_pop_blocking();
      int32_t freq = (int32_t)(rec);
      sdm.sine_set((float)(freq));
      }
    a = sdm.sine_mod();
    while (pio_sm_is_tx_fifo_full(pio, sm)) {}
    pio->txf[sm] = a;
  }
}

void setup() {

  //setup pin 14 to outut
  pico_setup_48000_32(14);
  delay(2000);
  multicore_launch_core1(core1_worker);

  Serial.begin(115200);
  delay(2000);
  Serial.setTimeout(5);
}

void loop() {
  if (Serial.available()) {
    int32_t freq = Serial.parseInt();
    if(freq>(-1)){
    multicore_fifo_push_blocking((uint32_t)(freq));
    }
 }
}
