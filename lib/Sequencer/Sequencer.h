#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "Arduino.h"
#include "AudioTools.h"

// Forward declaration for TibetanBowl
class TibetanBowl;

class Sequencer {
public:
    static const uint8_t MAX_STEPS = 32;
    
    struct Step {
        bool active;
        float frequency;
        uint8_t velocity;
        uint8_t gate_length;
        
        Step() : active(false), frequency(440.0f), velocity(100), gate_length(50) {}
    };
    
    enum State {
        STOPPED,
        PLAYING,
        PAUSED
    };

private:
    Step steps[MAX_STEPS];
    uint8_t current_step;
    uint8_t num_steps;
    uint16_t bpm;
    uint32_t step_duration_ms;
    uint32_t last_step_time;
    uint32_t gate_off_time;
    State state;
    bool gate_active;
    
    // Audio generators
    audio_tools::SineWaveGenerator<int16_t>* audio_generator;
    TibetanBowl* bowl_generator;
    bool use_bowl_mode;
    
    // Note frequencies table
    static const float note_frequencies[];
    static const uint8_t NUM_NOTES;

public:
    Sequencer();
    
    // Configuration
    void setBPM(uint16_t bpm);
    void setNumSteps(uint8_t steps);
    void setAudioGenerator(audio_tools::SineWaveGenerator<int16_t>* generator);
    void setBowlGenerator(TibetanBowl* bowl);
    void setBowlMode(bool enable);
    
    // Playback control
    void play();
    void stop();
    void pause();
    void reset();
    
    // Step editing
    void setStep(uint8_t step_index, bool active, float frequency, uint8_t velocity = 100, uint8_t gate_length = 50);
    void setStepActive(uint8_t step_index, bool active);
    void setStepFrequency(uint8_t step_index, float frequency);
    void setStepVelocity(uint8_t step_index, uint8_t velocity);
    void setStepGateLength(uint8_t step_index, uint8_t gate_length);
    
    // Data access
    Step getStep(uint8_t step_index) const;
    uint8_t getCurrentStep() const { return current_step; }
    uint8_t getNumSteps() const { return num_steps; }
    uint16_t getBPM() const { return bpm; }
    State getState() const { return state; }
    bool isGateActive() const { return gate_active; }
    bool isBowlMode() const { return use_bowl_mode; }
    
    // Main update function
    void update();
    
    // Utilities
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