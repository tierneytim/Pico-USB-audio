#include "pdmAudio.h"

#if defined CONFIG_IDF_TARGET_ESP32

#include "driver/i2s.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
SDM sdm2;
void i2sCallback(const uint8_t *data, uint32_t len){
 size_t i2s_bytes_write = 0; 
 int16_t* data16=(int16_t*)data; //playData doesnt want const
 int16_t fy[2];
  
  
 int jump =4; //how many bytes at a time get sent to buffer
 int  n = len/jump; // number of byte chunks	
 for(int i=0;i<n;i++){
  fy[0] = (*data16);
  data16++;
		 
  fy[1] = (*data16);
  data16++;
  int16_t mono = (fy[0]+fy[1])/2;
  uint32_t fy2 = sdm2.o4_os32_df2(mono);
  size_t bytes_written = 0;
  i2s_write(I2S_NUM_0, (const char*)&fy2, 4, &bytes_written,  10 );
  }
}

void a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t*param){
	esp_a2d_cb_param_t *a2d = (esp_a2d_cb_param_t *)(param);
	switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:{
        
        uint8_t* temp= a2d->conn_stat.remote_bda;
        break;
    }
	case ESP_A2D_AUDIO_CFG_EVT: {
        ESP_LOGI(BT_AV_TAG, "A2DP audio stream configuration, codec type %d", a2d->audio_cfg.mcc.type);
        // for now only SBC stream is supported
        if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
            uint32_t sampleRate = 16000;
            char oct0 = a2d->audio_cfg.mcc.cie.sbc[0];
            if (oct0 & (0x01 << 6)) {
                sampleRate = 32000;
            } else if (oct0 & (0x01 << 5)) {
                sampleRate = 44100;
            } else if (oct0 & (0x01 << 4)) {
                sampleRate = 48000;
            }
            ESP_LOGI(BT_AV_TAG, "Configure audio player %x-%x-%x-%x",
                     a2d->audio_cfg.mcc.cie.sbc[0],
                     a2d->audio_cfg.mcc.cie.sbc[1],
                     a2d->audio_cfg.mcc.cie.sbc[2],
                     a2d->audio_cfg.mcc.cie.sbc[3]);
					 if(i2s_set_sample_rates(I2S_NUM_0, sampleRate)==ESP_OK){
						ESP_LOGI(BT_AV_TAG, "Audio player configured, sample rate=%d", sampleRate);
					 }
		}
		
        break;
    }
    default:
        log_e("a2dp invalid cb event: %d", event);
        break;
    }
}

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
    .bck_io_num =22,//26
    .ws_io_num = 19, //27
    .data_out_num = 14, //25
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  // now configure i2s with constructed pinout and config
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

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
  //Arduino bluetooth initialisation
  btStart();

  // bluedroid  allows for bluetooth classic
  esp_bluedroid_init();
  esp_bluedroid_enable();
   
  //set up device name
  esp_bt_dev_set_device_name("ESP_PDM");
  
  // this sets up the audio receive
  esp_a2d_sink_init();
  esp_a2d_register_callback(a2d_cb);
  
  // set discoverable and connectable mode, wait to be connected
  #if ESP_IDF_VERSION_MAJOR > 3
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  #else
  esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
  #endif
  esp_a2d_sink_register_data_callback(i2sCallback); 
}
#endif