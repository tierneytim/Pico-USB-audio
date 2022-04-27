#include "pdm.pio.h"
#include "SDM.h"

// PDM object
SDM sdm;
PIO Pio = pio1;
uint Sm;

void setup() {
  delay(1000);
// less noisy power supply
#ifdef ARDUINO_ARCH_MBED_RP2040
  _gpio_init(23);
#else ARDUINO_ARCH_RP2040
  gpio_init(23);
#endif

  gpio_set_dir(23, GPIO_OUT);
  gpio_put(23, 1);
  uint pin = 14;
  // Set the appropriate clock
  set_sys_clock_khz(115200, false);
  uint offset = pio_add_program(Pio, &pdm_program);
  Sm = pio_claim_unused_sm(Pio, true);

  // PIO configuration
  pio_sm_config c = pdm_program_get_default_config(offset);
  sm_config_set_out_pins(&c, pin, 1);
  pio_gpio_init(Pio, pin);
  pio_sm_set_consecutive_pindirs(Pio, Sm, pin, 1, true);

  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_clkdiv(&c, 115200 * 1000 / (48000.0 * 32));
  sm_config_set_out_shift(&c, true, true, 32);
  pio_sm_init(Pio, Sm, offset, &c);
  pio_sm_set_enabled(Pio, Sm, true);
  delay(1000);
}

uint32_t secs3 = 48000 * 3;
void loop() {
  //try a few different scalings
  for (uint16_t i = 2; i < 6; i++) {
    // peak output voltage is now ~ 1.65V+ 1.65/i
    sdm.attenuate(i);
    // keep peek voltage for 3 seconds
    for (int j = 0; j < secs3; j++) {
      uint32_t a = sdm.o4_os32_df2(32767);
      while (pio_sm_is_tx_fifo_full(Pio, Sm)) {}
      Pio->txf[Sm] = a;
    }
  }
  for (uint16_t i = 5; i > 1; i--) {
    // peak output voltage is now ~ 1.65V+ 1.65/i
    sdm.attenuate(i);
    // keep peek voltage for 3 seconds
    for (int j = 0; j < secs3; j++) {
      uint32_t a = sdm.o4_os32_df2(32767);
      while (pio_sm_is_tx_fifo_full(Pio, Sm)) {}
      Pio->txf[Sm] = a;
    }
  }
}