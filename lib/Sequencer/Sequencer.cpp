#include <Sequencer.h>
#include <AudioTools.h>
#include <TibetanBowl.h>

// Note frequencies table based on AudioTools defines
const float Sequencer::note_frequencies[] = {
    N_C0, N_CS0, N_D0, N_DS0, N_E0, N_F0, N_FS0, N_G0, N_GS0, N_A0, N_AS0, N_B0,
    N_C1, N_CS1, N_D1, N_DS1, N_E1, N_F1, N_FS1, N_G1, N_GS1, N_A1, N_AS1, N_B1,
    N_C2, N_CS2, N_D2, N_DS2, N_E2, N_F2, N_FS2, N_G2, N_GS2, N_A2, N_AS2, N_B2,
    N_C3, N_CS3, N_D3, N_DS3, N_E3, N_F3, N_FS3, N_G3, N_GS3, N_A3, N_AS3, N_B3,
    N_C4, N_CS4, N_D4, N_DS4, N_E4, N_F4, N_FS4, N_G4, N_GS4, N_A4, N_AS4, N_B4,
    N_C5, N_CS5, N_D5, N_DS5, N_E5, N_F5, N_FS5, N_G5, N_GS5, N_A5, N_AS5, N_B5,
    N_C6, N_CS6, N_D6, N_DS6, N_E6, N_F6, N_FS6, N_G6, N_GS6, N_A6, N_AS6, N_B6,
    N_C7, N_CS7, N_D7, N_DS7, N_E7, N_F7, N_FS7, N_G7, N_GS7, N_A7, N_AS7, N_B7,
    N_C8, N_CS8, N_D8, N_DS8, N_E8, N_F8, N_FS8, N_G8, N_GS8, N_A8, N_AS8, N_B8
};

const uint8_t Sequencer::NUM_NOTES = sizeof(note_frequencies) / sizeof(note_frequencies[0]);

Sequencer::Sequencer() 
    : current_step(0)
    , num_steps(16)
    , bpm(120)
    , last_step_time(0)
    , gate_off_time(0)
    , state(STOPPED)
    , gate_active(false)
    , audio_generator(nullptr)
    , bowl_generator(nullptr)
    , use_bowl_mode(false)
{
    calculateStepDuration();
    
    // Initialize default pattern
    for (uint8_t i = 0; i < MAX_STEPS; i++) {
        steps[i].active = false;
        steps[i].frequency = N_C4;
        steps[i].velocity = 100;
        steps[i].gate_length = 50;
    }
}

void Sequencer::setBPM(uint16_t new_bpm) {
    if (new_bpm >= 15 && new_bpm <= 200) {
        bpm = new_bpm;
        calculateStepDuration();
    }
}

void Sequencer::setNumSteps(uint8_t steps) {
    if (steps >= 1 && steps <= MAX_STEPS) {
        num_steps = steps;
        if (current_step >= num_steps) {
            current_step = 0;
        }
    }
}

void Sequencer::setAudioGenerator(audio_tools::SineWaveGenerator<int16_t>* generator) {
    audio_generator = generator;
}

void Sequencer::setBowlGenerator(TibetanBowl* bowl) {
    bowl_generator = bowl;
    Serial.println("Bowl generator connected to sequencer");
}

void Sequencer::setBowlMode(bool enable) {
    use_bowl_mode = enable;
    Serial.printf("Sequencer bowl mode: %s\n", enable ? "ENABLED" : "DISABLED");
}

void Sequencer::play() {
    state = PLAYING;
    last_step_time = millis();
}

void Sequencer::stop() {
    state = STOPPED;
    current_step = 0;
    gate_active = false;
    stopGate();
}

void Sequencer::pause() {
    state = PAUSED;
    gate_active = false;
    stopGate();
}

void Sequencer::reset() {
    current_step = 0;
    gate_active = false;
    stopGate();
}

void Sequencer::setStep(uint8_t step_index, bool active, float frequency, uint8_t velocity, uint8_t gate_length) {
    if (step_index < MAX_STEPS) {
        steps[step_index].active = active;
        steps[step_index].frequency = frequency;
        steps[step_index].velocity = constrain(velocity, 0, 127);
        steps[step_index].gate_length = constrain(gate_length, 1, 100);
    }
}

void Sequencer::setStepActive(uint8_t step_index, bool active) {
    if (step_index < MAX_STEPS) {
        steps[step_index].active = active;
    }
}

void Sequencer::setStepFrequency(uint8_t step_index, float frequency) {
    if (step_index < MAX_STEPS) {
        steps[step_index].frequency = frequency;
    }
}

void Sequencer::setStepVelocity(uint8_t step_index, uint8_t velocity) {
    if (step_index < MAX_STEPS) {
        steps[step_index].velocity = constrain(velocity, 0, 127);
    }
}

void Sequencer::setStepGateLength(uint8_t step_index, uint8_t gate_length) {
    if (step_index < MAX_STEPS) {
        steps[step_index].gate_length = constrain(gate_length, 1, 100);
    }
}

Sequencer::Step Sequencer::getStep(uint8_t step_index) const {
    if (step_index < MAX_STEPS) {
        return steps[step_index];
    }
    return Step();
}

void Sequencer::update() {
    if (state != PLAYING) {
        return;
    }
    
    uint32_t current_time = millis();
    
    // Check if we need to advance to next step
    if (current_time - last_step_time >= step_duration_ms) {
        nextStep();
        triggerStep();
        last_step_time = current_time;
    }
    
    // Check if we need to turn off gate
    if (gate_active && current_time >= gate_off_time) {
        stopGate();
    }
}

float Sequencer::getNoteFrequency(uint8_t note_index) {
    if (note_index < NUM_NOTES) {
        return note_frequencies[note_index];
    }
    return N_A4;
}

void Sequencer::printStatus() const {
    Serial.println("=== SEQUENCER STATUS ===");
    Serial.printf("State: %s\n", 
        state == PLAYING ? "PLAYING" : 
        state == PAUSED ? "PAUSED" : "STOPPED");
    Serial.printf("BPM: %d\n", bpm);
    Serial.printf("Steps: %d/%d\n", num_steps, MAX_STEPS);
    Serial.printf("Current Step: %d\n", current_step);
    Serial.printf("Gate: %s\n", gate_active ? "ACTIVE" : "INACTIVE");
    Serial.printf("Bowl Mode: %s\n", use_bowl_mode ? "ON" : "OFF");
    Serial.printf("Step Duration: %d ms\n", step_duration_ms);
    Serial.println();
}

void Sequencer::printPattern() const {
    Serial.println("=== PATTERN ===");
    for (uint8_t i = 0; i < num_steps; i++) {
        Serial.printf("Step %2d: %s | %.2f Hz | V:%d | G:%d%%\n",
            i,
            steps[i].active ? "ON " : "OFF",
            steps[i].frequency,
            steps[i].velocity,
            steps[i].gate_length);
    }
    Serial.println();
}

void Sequencer::calculateStepDuration() {
    // For 16th notes: 60000ms / BPM / 4
    step_duration_ms = (60000 / bpm) / 4;
}

void Sequencer::triggerStep() {
    if (current_step >= num_steps) {
        return;
    }
    
    const Step& step = steps[current_step];
    
    if (step.active) {
        if (use_bowl_mode && bowl_generator) {
            // Use Tibetan Bowl
            float velocity_normalized = step.velocity / 127.0f;
            bowl_generator->strike(step.frequency, velocity_normalized);
          //  Serial.printf("🎌 Bowl Step %d: %.2f Hz\n", current_step, step.frequency);
        } else if (audio_generator) {
            // Use normal sine wave
            audio_generator->begin(audio_tools::AudioInfo(44100, 2, 16), step.frequency);
            Serial.printf("🎵 Sine Step %d: %.2f Hz\n", current_step, step.frequency);
        } else {
            Serial.println("⚠️ No audio generator set!");
            return;
        }
        
        gate_active = true;
        uint32_t gate_duration = (step_duration_ms * step.gate_length) / 100;
        gate_off_time = millis() + gate_duration;
    }
}

void Sequencer::stopGate() {
    gate_active = false;
    if (use_bowl_mode && bowl_generator) {
        bowl_generator->release(); 
    }
}

void Sequencer::nextStep() {
    current_step++;
    if (current_step >= num_steps) {
        current_step = 0;
    }
}