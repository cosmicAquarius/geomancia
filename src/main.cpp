#include "Arduino.h"
#include "WiFiMulti.h"
#include "AudioTools.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <MuxController.h>
#include <DriverUDA1334A.h>
#include <SynthController.h>

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

// Audio configuration
AudioInfo info(44100, 2, 16);

// Driver UDA1334A (already contains I2SStream)
DriverUDA1334A driverUDA1334A;

// StreamCopy will be initialized after driver in setupAudio()
StreamCopy *copier = nullptr;

// SynthController instance
SynthController synthesizer;

MuxController muxController;

// FreeRTOS task handles
TaskHandle_t audioTaskHandle = NULL;
TaskHandle_t muxTaskHandle = NULL;

volatile bool audioRunning = false;

// PATTERN SWITCHING VARIABLES
enum PatternType
{
  PATTERN_BOWL = 0,
  PATTERN_ELECTRONIC,
  PATTERN_TECHNO,
  PATTERN_ACID,
  PATTERN_COUNT
};

PatternType currentPattern = PATTERN_BOWL;
unsigned long lastPatternChange = 0;
const unsigned long PATTERN_CHANGE_INTERVAL = 5000; // 20 secondes

// Pattern names for debug
const char *patternNames[] = {
    "Tibetan Bowl",
    "Electronic",
    "Techno",
    "Acid House"};

/**
 * AUDIO TASK - High priority
 */
void audioTask(void *parameter)
{
  Serial.println("Audio Task started on Core " + String(xPortGetCoreID()));

  if (!copier)
  {
    Serial.println("Error: copier not initialized");
    vTaskDelete(NULL);
    return;
  }

  audioRunning = true;

  while (audioRunning)
  {
    // Update synthesizer (includes sequencer)
    synthesizer.update();

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

/**
 * SWITCH TO NEXT PATTERN
 */
void switchToNextPattern()
{
  // Stop current pattern
  synthesizer.stopSequencer();

  // Move to next pattern
  currentPattern = (PatternType)((currentPattern + 1) % PATTERN_COUNT);

  // Generate random seed
  uint16_t seed = analogRead(A0) + millis() + muxController.get(0, 0);

  Serial.printf("🎵 Switching to pattern: %s (seed: %d)\n",
                patternNames[currentPattern], seed);

  uint16_t rawValue = muxController.get(0, 0);
  uint16_t bpm = map(rawValue, 0, 1500, 16, 200);
  bpm = constrain(bpm, 16, 200); // Sécurise les bornes

  // Create new pattern based on type
  switch (currentPattern)
  {
  case PATTERN_BOWL:

    synthesizer.setupVCOs("tibetan");
    synthesizer.createBowlPattern(64, bpm, seed);
    break;

  case PATTERN_ELECTRONIC:

    synthesizer.setupVCOs("acid");
    synthesizer.createElectronicPattern(64, bpm, seed);
    break;

  case PATTERN_TECHNO:

    synthesizer.setupVCOs("acid");
    synthesizer.createTechnoPattern(64, bpm);
    break;

  case PATTERN_ACID:

    synthesizer.setupVCOs("ambient");
    synthesizer.createAcidPattern(64, bpm);
    break;
  }

  // Start playing new pattern
  synthesizer.playSequencer();

  Serial.printf("✓ Pattern '%s' started\n", patternNames[currentPattern]);
}

void setupAudio()
{
  Serial.println("Audio initialization...");

  // Initialize driver UDA1334A
  if (!driverUDA1334A.begin(info))
  {
    Serial.println("Error: Cannot initialize UDA1334A");
    return;
  }

  Serial.println("Audio initialized successfully");
}

void setupSynthesizer()
{
  // Initialize synthesizer
  if (!synthesizer.begin(info))
  {
    Serial.println("Error: Cannot initialize synthesizer");
    return;
  }

  // Start with Bowl pattern

  synthesizer.createBowlPattern(64, 30, analogRead(A0) + millis() + muxController.get(0, 0));

  copier = new StreamCopy(driverUDA1334A.getStream(), *synthesizer.getAudioStream());

  Serial.println("Creating initial synthesizer pattern...");

  // Start playing immediately
  synthesizer.playSequencer();

  // Initialize pattern switching timer
  lastPatternChange = millis();
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
  Serial.println("\n=== ESP32 Audio Synthesizer with Auto Pattern Switching ===\n");

  // Setup ADC
  analogSetWidth(12);
  analogSetAttenuation(ADC_ATTENDB_MAX);

  // Setup audio and synthesizer
  setupAudio();

  // Setup synthesizer pattern
  setupSynthesizer();

  // create tasks
  setupTasks();

  Serial.println("Setup completed. Auto pattern switching every 20s.\n");
}

// Memory formatting utility function
String formatMemory(uint32_t bytes)
{
  if (bytes >= 1024 * 1024)
  {
    return String(bytes / 1024.0 / 1024.0, 1) + " MB";
  }
  else if (bytes >= 1024)
  {
    return String(bytes / 1024.0, 1) + " KB";
  }
  else
  {
    return String(bytes) + " B";
  }
}

// Main loop with pattern switching
void loop()
{
  static unsigned long lastMonitor = 0;

  // CHECK PATTERN SWITCHING (every 20 seconds)
  if (millis() - lastPatternChange > PATTERN_CHANGE_INTERVAL)
  {
    switchToNextPattern();
    lastPatternChange = millis();
  }
  if (millis() - lastPatternChange > 200)
  {
    uint16_t rawValue = muxController.get(0, 0);
    uint16_t bpm = map(rawValue, 0, 1500, 16, 200);
    bpm = constrain(bpm, 16, 200); // Sécurise les bornes
    synthesizer.setBPM(bpm);
  }

  // SYSTEM MONITORING (every 5 seconds)
  if (millis() - lastMonitor > 5000)
  {
    lastMonitor = millis();

    // Memory calculations
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
    if (audioTaskHandle)
    {
      Serial.printf("🎵 AudioTask: %s\n",
                    eTaskGetState(audioTaskHandle) == eRunning ? "Running" : "Stopped");
    }
    if (muxTaskHandle)
    {
      Serial.printf("🎛️  MuxTask: %s\n",
                    eTaskGetState(muxTaskHandle) == eRunning ? "Running" : "Stopped");
    }

    // Synthesizer status with current pattern
    Serial.printf("🎼 Pattern: %s | Step %d/%d | BPM %d | %s\n",
                  patternNames[currentPattern],
                  synthesizer.getCurrentStep() + 1,
                  synthesizer.getNumSteps(),
                  synthesizer.getBPM(),
                  synthesizer.isPlaying() ? "Playing" : "Stopped");

    // Time until next pattern change
    unsigned long timeUntilChange = PATTERN_CHANGE_INTERVAL - (millis() - lastPatternChange);
    Serial.printf("⏰ Next pattern in: %lu seconds\n", timeUntilChange / 1000);

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