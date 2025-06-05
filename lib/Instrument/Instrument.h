#ifndef TIBETAN_BOWL_H
#define TIBETAN_BOWL_H

#include <Arduino.h>
#include <AudioTools.h>

class Instrument
{
private:
    // Audio streams for each VCO

    // ADSR envelope
    audio_tools::ADSRGain *adsr;

    // Audio configuration
    audio_tools::AudioInfo info;
    audio_tools::AudioEffectStream *effectStream;

    // Bowl parameters
    float fundamental_freq;
    float vco1_level;
    float vco2_level;
    float vco3_level;

    // Beating parameters (slight detuning)
    float vco1_detune;
    float vco2_detune; // Cents detuning for beating effect
    float vco3_detune;
    InputMixer<int16_t> *mixer;

    // Current mixed sample
    int16_t current_sample;

    // Tous les VCO (13 générateurs différents)
    audio_tools::SineWaveGenerator<int16_t> *vco1;    // Sine classique
    audio_tools::FastSineGenerator<int16_t> *vco2;    // Sine rapide
    audio_tools::SquareWaveGenerator<int16_t> *vco3;  // Carré
    audio_tools::SawToothGenerator<int16_t> *vco4;    // Dent de scie
    audio_tools::WhiteNoiseGenerator<int16_t> *vco5;  // Bruit blanc
    audio_tools::PinkNoiseGenerator<int16_t> *vco6;   // Bruit rose
    audio_tools::SilenceGenerator<int16_t> *vco7;     // Silence
    audio_tools::GeneratorFromArray<int16_t> *vco8;   // Depuis tableau
    audio_tools::GeneratorFixedValue<int16_t> *vco9;  // Valeur fixe
    audio_tools::SineFromTable<int16_t> *vco10;       // Sine depuis table
    audio_tools::SineWaveGenerator<int16_t> *vco11;   // Sine extra
    audio_tools::SquareWaveGenerator<int16_t> *vco12; // Carré extra

    // Tous les streams correspondants avec noms descriptifs
    audio_tools::GeneratedSoundStream<int16_t> *stream_sine;         // Sine classique
    audio_tools::GeneratedSoundStream<int16_t> *stream_fast_sine;    // Sine rapide
    audio_tools::GeneratedSoundStream<int16_t> *stream_square;       // Carré
    audio_tools::GeneratedSoundStream<int16_t> *stream_sawtooth;     // Dent de scie
    audio_tools::GeneratedSoundStream<int16_t> *stream_white_noise;  // Bruit blanc
    audio_tools::GeneratedSoundStream<int16_t> *stream_pink_noise;   // Bruit rose
    audio_tools::GeneratedSoundStream<int16_t> *stream_silence;      // Silence
    audio_tools::GeneratedSoundStream<int16_t> *stream_array;        // Depuis tableau
    audio_tools::GeneratedSoundStream<int16_t> *stream_fixed_value;  // Valeur fixe
    audio_tools::GeneratedSoundStream<int16_t> *stream_sine_table;   // Sine depuis table
    audio_tools::GeneratedSoundStream<int16_t> *stream_sine_extra;   // Sine extra
    audio_tools::GeneratedSoundStream<int16_t> *stream_square_extra; // Carré extra
    enum VcoType
    {
        VCO_SINE = 1,
        VCO_FAST_SINE = 2,
        VCO_SQUARE = 3,
        VCO_SAWTOOTH = 4,
        VCO_WHITE_NOISE = 5,
        VCO_PINK_NOISE = 6,
        VCO_SILENCE = 7,
        VCO_ARRAY = 8,
        VCO_FIXED_VALUE = 9,
        VCO_SINE_TABLE = 10,
        VCO_SINE_EXTRA = 11,
        VCO_SQUARE_EXTRA = 12
    };
    VcoType currentVco1;
    VcoType currentVco2;
    VcoType currentVco3;

    VcoType previousVco1;
    VcoType previousVco2;
    VcoType previousVco3;

public:
    Instrument();
    ~Instrument();

    // Initialization
    bool begin(audio_tools::AudioInfo audioInfo);

    // Bowl control
    void strike(float frequency = 440.0f, float velocity = 1.0f);
    void release();
    void update();

    // Get mixed audio sample
    int16_t readSample();

    // Configuration
    void setADSR(float attack = 0.1f, float decay = 0.2f, float sustain = 0.7f, float release = 8.0f);
    void setVcoVolumes(float vco1 = 1.0f, float vco2 = 0.6f, float vco3 = 0.3f);
    void setBeating(float vco1_cents = 3.0f, float vco2_cents = 3.0f, float vco3_cents = -2.5f);
    // audio_tools::InputMixer<int16_t>* getAudioStream();
    audio_tools::AudioStream *getAudioStream();

    // Status
    bool isActive() const;
    void setupVCOs(const String &style);
    void morphToStyle(const String &targetStyle, float morphTime = 1.0f);

    void replaceVcoInMixer(int channel, int newVcoIndex);
    void setChannelWeight(int channel, int weight);
    
private:
    void initializeComponents();
    void updateFrequencies();
    float centsToRatio(float cents);
};

#endif // TIBETAN_BOWL_H