#include <M5Core2.h>
#include <driver/i2s.h>

#define CONFIG_I2S_BCK_PIN 12 //Define I2S related ports.  
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0  // Define the speaker port.  

#define MODE_MIC 0  // Define the working mode.  
#define MODE_SPK 1
#define DATA_SIZE 1024

uint8_t microphonedata0[1024 * 100];
int data_offset = 0;

bool InitI2SSpeakOrMic(int mode){  //Init I2S.  
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(Speak_I2S_NUMBER); // Uninstall the I2S driver.  
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),  // Set the I2S operating mode.  
        .sample_rate = 44100, // Set the I2S sampling rate.  
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Fixed 12-bit stereo MSB.  
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // Set the channel format.  
        .communication_format = I2S_COMM_FORMAT_I2S,  // Set the format of the communication.  
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Set the interrupt flag.  
        .dma_buf_count = 2, //DMA buffer count.  
        .dma_buf_len = 128, //DMA buffer length.  
    };
    if (mode == MODE_MIC){
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }else{
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;  //I2S clock setup.
        i2s_config.tx_desc_auto_clear = true; // Enables auto-cleanup descriptors for understreams.  
    }
    // Install and drive I2S.  
    err += i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;  // Link the BCK to the CONFIG_I2S_BCK_PIN pin. 
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;  //          ...
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;  //       ...
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN; //      ...
    err += i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config); // Set the I2S pin number.  
    err += i2s_set_clk(Speak_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Set the clock and bitwidth used by I2S Rx and Tx. 
    return true;
}

void DisplayInit(void){ // Initialize the display. 
  M5.Lcd.fillScreen(WHITE); //Set the screen background color to white.  
  M5.Lcd.setTextColor(BLACK); //Set the text color to black.  
  M5.Lcd.setTextSize(2);  //Set font size to 2.  
}


void setup(){
  M5.begin(true, true, true, true); //Init M5Core2.  
  M5.Axp.SetSpkEnable(true);  //Enable speaker power.  
  DisplayInit();
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setCursor(10, 10); // Set the cursor at (10,10). 
  M5.Lcd.printf("Recorder!"); // The screen prints the formatted string and wraps it.  
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(10, 26);
  //M5.Lcd.printf("Press Left Button to recording!");
  delay(100); //delay 100ms.
}

void loop() {
  TouchPoint_t pos= M5.Touch.getPressPoint(); // Stores the touch coordinates in pos.  
  if(pos.y > 240){
    if(pos.x < 109){
       
        data_offset = 0;
        InitI2SSpeakOrMic(MODE_MIC);
        size_t byte_read;
        while (1){
            i2s_read(Speak_I2S_NUMBER, (char *)(microphonedata0 + data_offset), DATA_SIZE, &byte_read, (100 / portTICK_RATE_MS));
            data_offset += 1024;
             M5.Axp.SetLDOEnable(3,true);   //Open the vibration.  

            if(data_offset == 1024 * 100 || M5.Touch.ispressed() != true)
                   
              break;
        }
        
        size_t bytes_written;
        InitI2SSpeakOrMic(MODE_SPK);
         M5.Axp.SetLDOEnable(3,false);
        i2s_write(Speak_I2S_NUMBER, microphonedata0, data_offset, &bytes_written, portMAX_DELAY);
    }
  }
}
