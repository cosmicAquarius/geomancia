#include "Arduino.h"
#include "WiFiMulti.h"
#include "AudioTools.h"

#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <MuxController.h>

// Digital I/O used
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

// Configuration audio
AudioInfo info(44100, 2, 16);               // 44.1kHz, stéréo, 16 bits
SineWaveGenerator<int16_t> sineWave(32000); // Amplitude
GeneratedSoundStream<int16_t> sound(sineWave);
I2SStream i2s;
StreamCopy copier(i2s, sound);

// Inclusion des classes
MuxController muxController;

// Handles des tâches FreeRTOS
TaskHandle_t audioTaskHandle = NULL;
TaskHandle_t muxTaskHandle = NULL;

// Variables partagées (protégées si nécessaire)
float filtered = 0.0f;
volatile bool audioRunning = false;

//==============================================
// TÂCHE AUDIO - Priorité élevée
//==============================================
void audioTask(void *parameter)
{
  Serial.println("Tâche Audio démarrée sur Core " + String(xPortGetCoreID()));

  audioRunning = true;

  while (audioRunning)
  {
    // Copie continue du flux audio
    copier.copy();

    // Petit yield pour éviter de monopoliser le CPU
    vTaskDelay(1); // 1ms
  }

  Serial.println("Tâche Audio terminée");
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

//==============================================
// TÂCHE MULTIPLEXEURS - Priorité normale
//==============================================
void muxTask(void *parameter)
{
  Serial.println("Tâche Multiplexeurs démarrée sur Core " + String(xPortGetCoreID()));

  while (true)
  {
    // Un seul readNext() toutes les 5ms
    muxController.readNext();

    // Pause de 5ms avant le prochain readNext()
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void setupAudio()
{
  Serial.println("Initialisation audio...");

  // Configuration I2S pour UDA1334A
  auto config = i2s.defaultConfig(TX_MODE);
  config.copyFrom(info);
  config.pin_bck = I2S_BCLK;
  config.pin_ws = I2S_LRC;
  config.pin_data = I2S_DOUT;
  config.i2s_format = I2S_STD_FORMAT;
  config.bits_per_sample = 16;

  // Démarrage I2S
  i2s.begin(config);

  // Configuration du générateur de son
  sound.begin(info);
  // 440 Hz
  sineWave.begin(info, N_A4);

  Serial.println("Audio initialisé - 440Hz stéréo");
}

void setupTasks()
{
  Serial.println("Création des tâches FreeRTOS...");

  // Tâche Audio - Priorité élevée, Core 1 (dédié)
  xTaskCreatePinnedToCore(
      audioTask,        // Fonction
      "AudioTask",      // Nom
      4096,             // Stack size
      NULL,             // Paramètres
      3,                // Priorité élevée (0-5, 5=max)
      &audioTaskHandle, // Handle
      1                 // Core 1 (Core 0 = WiFi/Bluetooth)
  );

  // Tâche Multiplexeurs - Priorité normale, Core 0
  xTaskCreatePinnedToCore(
      muxTask,
      "MuxTask",
      4096,
      NULL,
      1, // norma priority
      &muxTaskHandle,
      0 // Core 0
  );

  if (audioTaskHandle && muxTaskHandle)
  {
    Serial.println("✓ Tâches créées avec succès");
    Serial.println("  - AudioTask: Core 1, Priorité 3");
    Serial.println("  - MuxTask: Core 0, Priorité 1");
  }
  else
  {
    Serial.println("✗ Erreur création tâches");
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

  // wait for serail to stabilize
  delay(1000);
  Serial.println("\n=== ESP32 Audio Synthesizer avec FreeRTOS ===\n");

  // Setup ADC
  analogSetWidth(12);
  analogSetAttenuation(ADC_ATTENDB_MAX);

  // Setup audio
  setupAudio();

  // create tasks
  setupTasks();

  Serial.println("Setup terminé. Les tâches tournent en parallèle.\n");
}

void loop()
{

  static unsigned long lastMonitor = 0;
  // each 5 sec
  if (millis() - lastMonitor > 5000)
  {
    lastMonitor = millis();

    Serial.printf("=== STATUS ===\n");
    Serial.printf("Heap libre: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Filtered value: %.2f\n", filtered);

    // État des tâches
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

  for (uint8_t i = 0; i < 16; ++i)
  {
    float raw = muxController.get(0, 0);
    plotValues(1, (uint16_t)raw);
    delay(5);
  }
}