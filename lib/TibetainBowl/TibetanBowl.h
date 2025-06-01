#ifndef TIBETAN_BOWL_H
#define TIBETAN_BOWL_H

#include <Arduino.h>
#include <AudioTools.h>

class TibetanBowl
{
private:
    // Three VCOs for harmonic content
    audio_tools::SineWaveGenerator<int16_t> *vco1; // Fundamental
    audio_tools::SineWaveGenerator<int16_t> *vco2; // 2nd harmonic + beating
    audio_tools::SineWaveGenerator<int16_t> *vco3; // 3rd harmonic + beating

    // Audio streams for each VCO
    audio_tools::GeneratedSoundStream<int16_t> *stream1;
    audio_tools::GeneratedSoundStream<int16_t> *stream2;
    audio_tools::GeneratedSoundStream<int16_t> *stream3;

    // ADSR envelope
    audio_tools::ADSRGain *adsr;

    // Audio configuration
    audio_tools::AudioInfo info;
    audio_tools::AudioEffectStream* effectStream;

    // Bowl parameters
    float fundamental_freq;
    float vco1_level;
    float vco2_level;
    float vco3_level;

    // Beating parameters (slight detuning)
    float vco2_detune; // Cents detuning for beating effect
    float vco3_detune;
    InputMixer<int16_t> mixer;

    // Current mixed sample
    int16_t current_sample;

public:
    TibetanBowl();
    ~TibetanBowl();

    // Initialization
    bool begin(audio_tools::AudioInfo audioInfo);

    // Bowl control
    void strike(float frequency = 440.0f, float velocity = 1.0f);
    void release();

    // Get mixed audio sample
    int16_t readSample();



    // Configuration
    void setADSR(float attack = 0.1f, float decay = 0.2f, float sustain = 0.7f, float release = 8.0f);
    void setHarmonicLevels(float vco1 = 1.0f, float vco2 = 0.6f, float vco3 = 0.3f);
    void setBeating(float vco2_cents = 3.0f, float vco3_cents = -2.5f);
   // audio_tools::InputMixer<int16_t>* getAudioStream();
    audio_tools::AudioEffectStream* getAudioStream();

    // Status
    bool isActive() const;

private:
    void initializeComponents();
    void updateFrequencies();
    float centsToRatio(float cents);
};

#endif // TIBETAN_BOWL_H