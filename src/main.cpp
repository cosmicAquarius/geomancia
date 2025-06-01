#include "Arduino.h"
#include "WiFiMulti.h"
#include "AudioTools.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <MuxController.h>
#include <DriverUDA1334A.h>
#include "Sequencer.h"

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

// Audio configuration
AudioInfo info(44100, 2, 16);

// Amplitude
SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> sound(sineWave);

// Driver UDA1334A (contient déjà l'I2SStream)
DriverUDA1334A driverUDA1334A;

// StreamCopy sera initialisé après le driver dans setupAudio()
StreamCopy* copier = nullptr;

// Sequencer instance
Sequencer sequencer;

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

  if (!copier) {
    Serial.println("Erreur: copier non initialisé");
    vTaskDelete(NULL);
    return;
  }

  audioRunning = true;

  while (audioRunning)
  {
    // Update sequencer before audio processing
    sequencer.update();
    
    // Continuous audio stream copy
    copier->copy();

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
 
  // Initialisez le driver UDA1334A
  if (!driverUDA1334A.begin(info)) {
    Serial.println("Erreur: Impossible d'initialiser UDA1334A");
    return;
  }

  // Sound generator configuration
  sound.begin(info);
  // Default frequency - will be controlled by sequencer
  sineWave.begin(info, N_A4);

  // Créez le StreamCopy APRÈS l'initialisation du driver
  copier = new StreamCopy(driverUDA1334A.getStream(), sound);

  Serial.println("Audio initialized - 440Hz stereo");
}

void setupSequencer()
{
  Serial.println("Sequencer initialization...");
  
  // Connect sequencer to audio generator
  sequencer.setAudioGenerator(&sineWave);
  
  // Jazz tempo
  sequencer.setBPM(140);
  sequencer.setNumSteps(32);
  
  // Jazz chord progression notes - Cmaj7, Am7, Dm7, G7
  float jazzNotes[] = {
    N_C4, N_E4, N_G4, N_B4,    // Cmaj7
    N_A3, N_C4, N_E4, N_G4,    // Am7
    N_D4, N_F4, N_A4, N_C5,    // Dm7
    N_G3, N_B3, N_D4, N_F4,    // G7
    N_C5, N_E5, N_G5,          // Higher octave variations
    N_A4, N_F4, N_D5
  };
  
  // Seed random generator
  randomSeed(analogRead(A0));
  
  // Create jazzy pattern with random elements
  for (uint8_t i = 0; i < 32; i++) {
    bool active = (i % 3 != 2) || (random(100) < 30); // Skip some steps for swing feel
    
    if (active) {
      // Pick random jazz note
      float note = jazzNotes[random(sizeof(jazzNotes)/sizeof(jazzNotes[0]))];
      
      // Random velocity (60-127 for dynamics)
      uint8_t velocity = random(60, 128);
      
      // Random gate length (20-90% for articulation variety)
      uint8_t gate = random(20, 91);
      
      sequencer.setStep(i, true, note, velocity, gate);
    } else {
      sequencer.setStep(i, false, N_C4, 100, 50);
    }
  }
  
  // Start playing immediately
  sequencer.play();
  
  Serial.println("Jazz sequencer initialized and playing");
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
  Serial.begin(115200);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // wait for serial to stabilize
  delay(1000);
  Serial.println("\n=== ESP32 Audio Synthesizer with Sequencer ===\n");

  // Setup ADC
  analogSetWidth(12);
  analogSetAttenuation(ADC_ATTENDB_MAX);

  // Setup audio
  setupAudio();
  
  // Setup sequencer
  setupSequencer();

  // create tasks
  setupTasks();

  Serial.println("Setup completed. Sequencer playing automatically.\n");
}

// Fonction utilitaire pour formater la mémoire
String formatMemory(uint32_t bytes) {
  if (bytes >= 1024 * 1024) {
    return String(bytes / 1024.0 / 1024.0, 1) + " MB";
  } else if (bytes >= 1024) {
    return String(bytes / 1024.0, 1) + " KB";
  } else {
    return String(bytes) + " B";
  }
}

// Dans votre loop() :
void loop() {
  static unsigned long lastMonitor = 0;
  
  if (millis() - lastMonitor > 5000) {
    lastMonitor = millis();

    // Calculs mémoire
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    float usedPercent = (usedHeap * 100.0) / totalHeap;

    Serial.printf("=== ⚙️  FREE RTOS STATUS  ⚙️  ===\n");
    Serial.printf("💾 Memory: %s / %s (%.1f%% used)\n", 
                  formatMemory(usedHeap).c_str(), 
                  formatMemory(totalHeap).c_str(), 
                  usedPercent);

    // Task states
    if (audioTaskHandle) {
      Serial.printf("🎵 AudioTask: %s\n", 
                    eTaskGetState(audioTaskHandle) == eRunning ? "Running" : "Stopped");
    }
    if (muxTaskHandle) {
      Serial.printf("🎛️  MuxTask: %s\n", 
                    eTaskGetState(muxTaskHandle) == eRunning ? "Running" : "Stopped");
    }
    
    // Sequencer status
    Serial.printf("🎼 Sequencer: Step %d/%d | BPM %d\n",
                  sequencer.getCurrentStep() + 1,
                  sequencer.getNumSteps(),
                  sequencer.getBPM());
    
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