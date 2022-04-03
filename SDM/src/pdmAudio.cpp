#include "pdmAudio.h"
#include "pdm.pio.h"

pdmAudio::pdmAudio() {
}

void pdmAudio::begin(uint pin) {
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
  multicore_launch_core1(core1_worker);
}
void pdmAudio::write(int16_t mono) {
	// less noisy power supply
    multicore_fifo_push_blocking((uint32_t)(mono));
}

