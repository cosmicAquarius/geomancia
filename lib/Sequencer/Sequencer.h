#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "Arduino.h"

// Forward declaration with AudioTools namespace
#include "AudioTools.h"

class Sequencer {
public:
    static const uint8_t MAX_STEPS = 32;
    
    // Structure pour un step du séquenceur
    struct Step {
        bool active;           // Step activé ou pas
        float frequency;       // Fréquence en Hz (utilise les defines AudioTools)
        uint8_t velocity;      // Vélocité 0-127 (pour plus tard)
        uint8_t gate_length;   // Longueur du gate en % (1-100)
        
        Step() : active(false), frequency(440.0f), velocity(100), gate_length(50) {}
    };
    
    // États du séquenceur
    enum State {
        STOPPED,
        PLAYING,
        PAUSED
    };

private:
    Step steps[MAX_STEPS];
    uint8_t current_step;
    uint8_t num_steps;          // Nombre de steps actifs (1-32)
    uint16_t bpm;               // Beats per minute
    uint32_t step_duration_ms;  // Durée d'un step en ms
    uint32_t last_step_time;    // Timestamp du dernier step
    uint32_t gate_off_time;     // Timestamp pour éteindre le gate
    State state;
    bool gate_active;           // État actuel du gate
    bool step_changed;          // Flag pour détecter les changements
    
    // Pointer to audio generator (injection)
    audio_tools::SineWaveGenerator<int16_t>* audio_generator;
    
    // Table des fréquences (reprend les defines AudioTools)
    static const float note_frequencies[];
    static const uint8_t NUM_NOTES;

public:
    Sequencer();
    
    // Configuration
    void setBPM(uint16_t bpm);
    void setNumSteps(uint8_t steps);
    void setAudioGenerator(audio_tools::SineWaveGenerator<int16_t>* generator);
    
    // Contrôle de lecture
    void play();
    void stop();
    void pause();
    void reset();
    
    // Édition des steps
    void setStep(uint8_t step_index, bool active, float frequency, uint8_t velocity = 100, uint8_t gate_length = 50);
    void setStepActive(uint8_t step_index, bool active);
    void setStepFrequency(uint8_t step_index, float frequency);
    void setStepVelocity(uint8_t step_index, uint8_t velocity);
    void setStepGateLength(uint8_t step_index, uint8_t gate_length);
    
    // Accès aux données
    Step getStep(uint8_t step_index) const;
    uint8_t getCurrentStep() const { return current_step; }
    uint8_t getNumSteps() const { return num_steps; }
    uint16_t getBPM() const { return bpm; }
    State getState() const { return state; }
    bool isGateActive() const { return gate_active; }
    
    // Fonction principale à appeler dans la boucle audio
    void update();
    
    // Utilitaires pour les notes
    static float getNoteFrequency(uint8_t note_index);
    static uint8_t getNumAvailableNotes() { return NUM_NOTES; }
    
    // Debug
    void printStatus() const;
    void printPattern() const;

private:
    void calculateStepDuration();
    void triggerStep();
    void stopGate();
    void nextStep();
};

#endif // SEQUENCER_H