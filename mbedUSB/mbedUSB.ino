#include "mbed.h"
#include "USBAudio.h"
#include "pdm.pio.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "pico/multicore.h"

void set_sys_clock_pll(uint32_t vco_freq, uint post_div1, uint post_div2) {
  if (!running_on_fpga()) {
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    48 * MHZ);

    pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
    uint32_t freq = vco_freq / (post_div1 * post_div2);

    // Configure clocks
    // CLK_REF = XOSC (12MHz) / 1 = 12MHz
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                    0,  // No aux mux
                    12 * MHZ,
                    12 * MHZ);

    // CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    freq, freq);

    clock_configure(clk_peri,
                    0,  // Only AUX mux on ADC
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    48 * MHZ,
                    48 * MHZ);
  }
}
bool check_sys_clock_khz(uint32_t freq_khz, uint *vco_out, uint *postdiv1_out, uint *postdiv_out) {
  uint crystal_freq_khz = clock_get_hz(clk_ref) / 1000;
  for (uint fbdiv = 320; fbdiv >= 16; fbdiv--) {
    uint vco = fbdiv * crystal_freq_khz;
    if (vco < 400000 || vco > 1600000) continue;
    for (uint postdiv1 = 7; postdiv1 >= 1; postdiv1--) {
      for (uint postdiv2 = postdiv1; postdiv2 >= 1; postdiv2--) {
        uint out = vco / (postdiv1 * postdiv2);
        if (out == freq_khz && !(vco % (postdiv1 * postdiv2))) {
          *vco_out = vco * 1000;
          *postdiv1_out = postdiv1;
          *postdiv_out = postdiv2;
          return true;
        }
      }
    }
  }
  return false;
}
static inline bool set_sys_clock_khz(uint32_t freq_khz, bool required) {
  uint vco, postdiv1, postdiv2;
  if (check_sys_clock_khz(freq_khz, &vco, &postdiv1, &postdiv2)) {
    set_sys_clock_pll(vco, postdiv1, postdiv2);
    return true;
  } else if (required) {
    panic("System clock of %u kHz cannot be exactly achieved", freq_khz);
  }
  return false;
}

PIO pio = pio1;
uint sm;

void core1_worker() {
  uint32_t a = pdm_4_os32(0);
  uint32_t mute = 0;
  int16_t pinput = 0;

  for (int i = -32767; i > 0; i++) {
    a = pdm_4_os32(i);
    pdm_write32(a, pio, sm);
    pdm_write32(a, pio, sm);
    pdm_write32(a, pio, sm);
  }


  while (1) {
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
    if (multicore_fifo_rvalid()) {
      uint32_t rec = multicore_fifo_pop_blocking();
      pinput = (int16_t)(rec);
      a = pdm_4_os32(pinput);
      while (pio_sm_is_tx_fifo_full(pio, sm)) {}
      pio->txf[sm] = a;
    }
  }
}




void setup() {
  //Audio object
  USBAudio audio(true, 48000, 2, 48000, 2);

  // less noisy power supply
  _gpio_init(23);
  gpio_set_dir(23, GPIO_OUT);
  gpio_put(23, 1);
  _gpio_init(25);
  gpio_set_dir(25, GPIO_OUT);
  // Set the appropriate clock
  set_sys_clock_khz(115200, false);
  uint offset = pio_add_program(pio, &pdm_program);
  sm = pio_claim_unused_sm(pio, true);
  pdm_program_init(pio, sm, offset, 14, 115200 * 1000 / (48000.0 * 32));

  // variables
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
      gpio_put(25, 1);
      for (int i = 0; i < 16; i++) {
        outL = *in;
        in++;
        outR = *in;
        in++;
        mono = (outL + outR) / 2;
        multicore_fifo_push_blocking((uint32_t)(mono));
        pmono = mono;
      }
    } else {
      multicore_fifo_push_blocking(pmono);
      gpio_put(25, 0);
    }
  }
}

void loop() {
}
