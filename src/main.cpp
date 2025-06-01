#include "Arduino.h"
#include "WiFiMulti.h"
#include "AudioTools.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <MuxController.h>

#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

// Audio configuration
// 44.1kHz, stereo, 16 bits
AudioInfo info(44100, 2, 16);

// Amplitude
SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> sound(sineWave);
I2SStream i2s;
StreamCopy copier(i2s, sound);
MuxController muxController;

// FreeRTOS task handles
TaskHandle_t audioTaskHandle = NULL;
TaskHandle_t muxTaskHandle = NULL;

volatile bool audioRunning = false;

/**
 * AUDIO TASK - High priority
 */
void audioTask(void *parameter)
{
  Serial.println("Audio Task started on Core " + String(xPortGetCoreID()));

  audioRunning = true;

  while (audioRunning)
  {
    // Continuous audio stream copy
    copier.copy();

    // Small yield to avoid monopolizing CPU
    // 1ms
    vTaskDelay(1);
  }

  Serial.println("Audio Task terminated");
  vTaskDelete(NULL);
}

void plotValues(uint8_t id, uint16_t value)
{
  Serial.print('>');
  Serial.print(id);
  Serial.print(':');
  Serial.print(value);
  Serial.println();
  Serial.flush();
}

/**
 * MULTIPLEXER TASK - Normal priority
 */
void muxTask(void *parameter)
{
  Serial.println("Multiplexer Task started on Core " + String(xPortGetCoreID()));

  while (true)
  {
    // Single readNext() every 5ms
    muxController.readNext();

    // 5ms pause before next readNext()
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void setupAudio()
{
  Serial.println("Audio initialization...");

  // I2S configuration for UDA1334A
  auto config = i2s.defaultConfig(TX_MODE);
  config.copyFrom(info);
  config.pin_bck = I2S_BCLK;
  config.pin_ws = I2S_LRC;
  config.pin_data = I2S_DOUT;
  config.i2s_format = I2S_STD_FORMAT;
  config.bits_per_sample = 16;

  // I2S startup
  i2s.begin(config);

  // Sound generator configuration
  sound.begin(info);
  // 440 Hz
  sineWave.begin(info, N_A4);

  Serial.println("Audio initialized - 440Hz stereo");
}

void setupTasks()
{
  Serial.println("Creating FreeRTOS tasks...");

  // Audio Task - High priority, Core 1 (dedicated)
  xTaskCreatePinnedToCore(
      audioTask,        // Function
      "AudioTask",      // Name
      4096,             // Stack size
      NULL,             // Parameters
      3,                // High priority (0-5, 5=max)
      &audioTaskHandle, // Handle
      1                 // Core 1 (Core 0 = WiFi/Bluetooth)
  );

  // Multiplexer Task - Normal priority, Core 0
  xTaskCreatePinnedToCore(
      muxTask,
      "MuxTask",
      4096,
      NULL,
      1, // normal priority
      &muxTaskHandle,
      0 // Core 0
  );

  if (audioTaskHandle && muxTaskHandle)
  {
    Serial.println("✓ Tasks created successfully");
    Serial.println("  - AudioTask: Core 1, Priority 3");
    Serial.println("  - MuxTask: Core 0, Priority 1");
  }
  else
  {
    Serial.println("✗ Task creation error");
  }
}

void setup()
{
  // Init hardware
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(SD_CS);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // wait for serial to stabilize
  delay(1000);
  Serial.println("\n=== ESP32 Audio Synthesizer with FreeRTOS ===\n");

  // Setup ADC
  analogSetWidth(12);
  analogSetAttenuation(ADC_ATTENDB_MAX);

  // Setup audio
  setupAudio();

  // create tasks
  setupTasks();

  Serial.println("Setup completed. Tasks running in parallel.\n");
}

void loop()
{

  static unsigned long lastMonitor = 0;
  // each 5 sec
  if (millis() - lastMonitor > 5000)
  {
    lastMonitor = millis();

    Serial.printf("=== STATUS ===\n");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

    // Task states
    if (audioTaskHandle)
    {
      Serial.printf("AudioTask state: %d\n", eTaskGetState(audioTaskHandle));
    }
    if (muxTaskHandle)
    {
      Serial.printf("MuxTask state: %d\n", eTaskGetState(muxTaskHandle));
    }
    Serial.println();
  }
  /*
    for (uint8_t i = 0; i < 16; ++i)
    {
      float raw = muxController.get(0, 0);
      plotValues(0, (uint16_t)raw);
      delay(5);
    }
  */
}