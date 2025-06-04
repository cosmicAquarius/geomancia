#include "Arduino.h"
#include "AudioTools.h"
#include <DriverUDA1334A.h>

using namespace audio_tools;

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

// Audio configuration
AudioInfo info(44100, 2, 16);

// === COMPOSANTS AUDIO (GLOBAUX comme dans l'exemple) ===
// 3 VCOs
SquareWaveGenerator<int16_t> vco1(1000);    // Basse
SineWaveGenerator<int16_t> vco2(6000);      // Lead  
SineWaveGenerator<int16_t> vco3(4000);      // Harmonique

// Streams pour chaque VCO
GeneratedSoundStream<int16_t> stream1(vco1);
GeneratedSoundStream<int16_t> stream2(vco2); 
GeneratedSoundStream<int16_t> stream3(vco3);

// Mixer pour combiner les 3 VCOs
InputMixer<int16_t> mixer;

// ✅ ADSR COMME DANS L'EXEMPLE - PAS DE POINTEUR !
ADSRGain adsr(0.0001, 1.0000, 0.9, 0.00001, 1.0);

// ✅ EFFECT STREAM AVEC LE MIXER (pas avec un seul VCO)
AudioEffectStream effects(mixer);

// Driver audio
DriverUDA1334A driverUDA1334A;

// ✅ STREAMCOPY DEPUIS EFFECTS (comme l'exemple)
StreamCopy copier(driverUDA1334A.getStream(), effects);

// === VARIABLES DE TEST ===
unsigned long lastNoteTime = 0;
const unsigned long NOTE_INTERVAL = 1000; // 1000ms entre chaque note
int currentNote = 0;

// Fréquences de test
float testFrequencies[] = {
    N_C4,   // Do
    N_A4,   // La
    N_E4,   // Mi
    N_G4,   // Sol
    N_C5    // Do aigu
};
const int NUM_TEST_NOTES = 5;

// ✅ ACTIONS COMME DANS L'EXEMPLE
void actionKeyOn(bool active, int pin, void* ptr) {
    Serial.println("🎹 KeyOn");
    float freq = *((float*)ptr);
    
    // Mettre à jour les fréquences des 3 VCOs
    vco1.setFrequency(freq);           // Fondamentale
    vco2.setFrequency(freq * 2.0f);    // Octave
    vco3.setFrequency(freq * 1.5f);    // Quinte
    
    // ✅ ADSR keyOn SANS PARAMÈTRE (comme l'exemple)
    adsr.keyOn();
    
    Serial.printf("   Frequencies: %.1f | %.1f | %.1f Hz\n", 
                  freq, freq * 2.0f, freq * 1.5f);
}

void actionKeyOff(bool active, int pin, void* ptr) {
    Serial.println("🎵 KeyOff");
    // ✅ ADSR keyOff SANS PARAMÈTRE (comme l'exemple)
    adsr.keyOff();
}

void setup() 
{
    Serial.begin(115200);
    AudioLogger::instance().begin(Serial, AudioLogger::Warning);
    
    delay(1000);
    Serial.println("\n🚀 === ESP32 ADSR TEST - STYLE AUDIOTOOLS DOC ===\n");
    
    // ✅ SETUP EFFECTS (comme l'exemple)
    effects.addEffect(adsr);
    Serial.println("✅ ADSR added to effects");
    
    // ✅ SETUP DRIVER (remplace AudioKit)
    if (!driverUDA1334A.begin(info)) {
        Serial.println("❌ Error: Cannot initialize UDA1334A");
        return;
    }
    Serial.println("✅ Driver initialized");
    
    // ✅ SETUP MIXER
    mixer.add(stream1, 80);  // VCO1 - 80%
    mixer.add(stream2, 60);  // VCO2 - 60%  
    mixer.add(stream3, 40);  // VCO3 - 40%
    Serial.println("✅ Mixer configured");
    
    // ✅ SETUP SOUND GENERATION (comme l'exemple)
    vco1.begin(info, 0);      // Pas de fréquence initiale
    vco2.begin(info, 0);
    vco3.begin(info, 0);
    stream1.begin(info);
    stream2.begin(info);
    stream3.begin(info);
    mixer.begin(info);
    effects.begin(info);      // ✅ IMPORTANT !
    
    Serial.println("✅ All components initialized");
    Serial.println("🎼 Chain: VCOs → Mixer → ADSR → Output");
    Serial.println("🧪 Auto test: Note every 1000ms, release after 600ms\n");
    
    // Première note
    static float firstNote = N_C4;
    actionKeyOn(true, 0, &firstNote);
    lastNoteTime = millis();
}

void loop() 
{
    unsigned long currentTime = millis();
    
    // ✅ COPY DATA (comme l'exemple)
    copier.copy();
    
    // === TEST AUTOMATIQUE ===
    
    // Release après 600ms
    static bool released = false;
    if (!released && currentTime - lastNoteTime >= 500) {
        actionKeyOff(true, 0, nullptr);
        released = true;
    }
    
    // Nouvelle note après 1000ms
    if (currentTime - lastNoteTime >= NOTE_INTERVAL) {
        released = false;
        
        // Passer à la note suivante
        currentNote = (currentNote + 1) % NUM_TEST_NOTES;
        
        // Déclencher la nouvelle note
        actionKeyOn(true, 0, &testFrequencies[currentNote]);
        
        lastNoteTime = currentTime;
    }
    
    // === STATUS DEBUG (toutes les 2 secondes) ===
    static unsigned long lastDebug = 0;
    if (currentTime - lastDebug >= 2000) {
        lastDebug = currentTime;
        
        Serial.printf("🔍 Note %d/5 (%.1f Hz) | ADSR: %s | Heap: %d\n",
                      currentNote + 1,
                      testFrequencies[currentNote],
                      adsr.isActive() ? "ACTIVE" : "IDLE",
                      ESP.getFreeHeap());
    }
}