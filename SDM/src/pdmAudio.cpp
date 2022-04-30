#include "pdmAudio.h"
//#include "SDM.h"

#if defined ARDUINO_ARCH_ESP32
#include "driver/i2s.h"
#include "soc/rtc.h"
#endif 

#if defined ARDUINO_ARCH_MBED_RP2040 || defined ARDUINO_ARCH_RP2040 
#include "pdm.pio.h"
#include "pico/multicore.h"
PIO pio = pio1;
uint sm;

void core1_worker() {
  uint32_t a = 0;
  int16_t pinput = 0;
  
  SDM sdm;

  //startup pop suppression
  for (int i = -32767; i < 0; i++) {
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
#endif


pdmAudio::pdmAudio() {
  float delta = (2.0*PI/6000.0);
    for (int i=0;i<6000;i++){
        sina[i]=(int16_t)(sin(i*delta)*32767);
    }
}

void pdmAudio::begin(uint pin) {
  delay(1000);
	// less noisy power supply
  #ifdef ARDUINO_ARCH_MBED_RP2040 
  _gpio_init(23);
  #elif  defined ARDUINO_ARCH_RP2040 
  gpio_init(23);
  #endif
  
  #if defined ARDUINO_ARCH_MBED_RP2040 || defined ARDUINO_ARCH_RP2040 
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
  #endif
  
  #ifdef ARDUINO_ARCH_ESP32
    // i2s configuration
  static const i2s_config_t i2s_config = {
    .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 48000,
    //.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, 
    //.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,    
    .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // default interrupt priority
    .dma_buf_count = 4,
    .dma_buf_len = 24,
    .use_apll = true,
    .tx_desc_auto_clear = true,
  };
  
  // i2s pinout
  static const i2s_pin_config_t pin_config = {
    .bck_io_num =22,//26
    .ws_io_num = 19, //27
    .data_out_num = 14, //25
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  // now configure i2s with constructed pinout and config
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  #endif   
  
  delay(1000);
  
}

void pdmAudio::USB() {
 #ifdef ARDUINO_ARCH_MBED_RP2040
 audio= new USBAudio(true, 48000, 2, 48000, 2);
 #endif
}

void pdmAudio::USBwrite() {
    #ifdef ARDUINO_ARCH_MBED_RP2040
     if (audio->read(myRawBuffer, sizeof(myRawBuffer))) {
      int16_t *lessRawBuffer = (int16_t *)myRawBuffer;
      for (int i = 0; i < 24; i++) {
        // the left value;
        int16_t outL = *lessRawBuffer;
        lessRawBuffer++;
        // the right value
        int16_t outR = *lessRawBuffer;
        lessRawBuffer++;
        //mono value
        int16_t mono = (outL + outR) / 2;
        pdmAudio::write(mono);
      }
    }
    #endif
}

void pdmAudio::write(int16_t mono) {
	#if defined ARDUINO_ARCH_MBED_RP2040 || defined ARDUINO_ARCH_RP2040 
    multicore_fifo_push_blocking((uint32_t)(mono));
    #endif
    
    #if defined ARDUINO_ARCH_ESP32
    uint32_t fy = sdm.o4_os32_df2(mono);
    size_t bytes_written = 0;
    i2s_write(I2S_NUM_0, (const char*)&fy, 4, &bytes_written,  10 );
    #endif 
    
}

void pdmAudio::USBtransfer(int16_t left,int16_t right) {
  #ifdef ARDUINO_ARCH_MBED_RP2040
  if(nBytes>95){
    uint8_t *pcBuffer =  (uint8_t *)pcBuffer16;
    audio->write(pcBuffer, 96);
    pcCounter =0;
    nBytes =0;
  }
  pcBuffer16[pcCounter]=left;
  pcCounter++;
  nBytes+=2;
  
  pcBuffer16[pcCounter]=right;
  pcCounter++;
  nBytes+=2;
  #endif
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
  