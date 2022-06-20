#include "pdmAudio.h"

#if defined CONFIG_IDF_TARGET_ESP32S2

#include "driver/i2s.h"

SDM sdm2;

pdmAudio::pdmAudio() {
  float delta = (2.0*PI/6000.0);
    for (int i=0;i<6000;i++){
        sina[i]=(int16_t)(sin(i*delta)*32767);
    }
}

void pdmAudio::begin(uint pin) {
  delay(1000);

  
  static const i2s_config_t i2s_config = {
    .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 48000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,    
    .communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // default interrupt priority
    .dma_buf_count = 6,
    .dma_buf_len = 80,
    .use_apll = true,
    .tx_desc_auto_clear = true,
  };
  
  // i2s pinout
  static const i2s_pin_config_t pin_config = {
    .bck_io_num =12,//26
    .ws_io_num = 18, //27
    .data_out_num = pin, //25
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  // now configure i2s with constructed pinout and config
  Serial.println(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
  Serial.println(i2s_set_pin(I2S_NUM_0, &pin_config));

  delay(1000);
  
}

void pdmAudio::USB() {

}

void pdmAudio::USBwrite() {

}

void pdmAudio::write(int16_t mono) {
    
    uint32_t fy = sdm.o4_os32_df2(mono);
    size_t bytes_written = 0;
    i2s_write(I2S_NUM_0, (const char*)&fy, 4, &bytes_written,  10 );
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

void pdmAudio::bluetooth(){

}
#endif