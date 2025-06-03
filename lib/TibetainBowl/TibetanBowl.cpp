#include <TibetanBowl.h>

TibetanBowl::TibetanBowl()
    : vco1(nullptr), vco2(nullptr), vco3(nullptr),
      stream1(nullptr), stream2(nullptr), stream3(nullptr), adsr(nullptr),
      info(44100, 2, 16), fundamental_freq(440.0f),
      vco1_level(1.0f),
      vco2_level(0.6f),
      vco3_level(0.3f),
      vco1_detune(12.0f), // Fundamental - no detune
      vco2_detune(5.0f),  // 2nd harmonic - slight sharp for slow beats
      vco3_detune(-4.2f), // 3rd harmonic - slight flat for complex interference
      current_sample(0)
{
}

void TibetanBowl::initializeComponents()

{
    // DÉTUNE ÉLECTRONIQUE - intervalles musicaux
    vco1_detune = 0.0f;  // Fondamentale propre
    vco2_detune = 12.0f; // Octave haute (lead)
    vco3_detune = 7.0f;  // Quinte parfaite (harmonique)

  // VCO1: Basse sub avec square wave pour punch
    vco1 = new audio_tools::SquareWaveGenerator<int16_t>(5000);
    // VCO2: Lead avec saw pour richesse harmonique
    vco2 = new audio_tools::SawToothGenerator<int16_t>(5000);    
    // VCO3: Texture avec triangle pour douceur
    vco3 = new audio_tools::SawToothGenerator<int16_t>(5000);

    // Create streams for each VCO
    stream1 = new audio_tools::GeneratedSoundStream<int16_t>(*vco1);
    stream2 = new audio_tools::GeneratedSoundStream<int16_t>(*vco2);
    stream3 = new audio_tools::GeneratedSoundStream<int16_t>(*vco3);

    vco1->begin(info);
    vco2->begin(info); // 2nd harmonic
    vco3->begin(info); // 3rd harmonic

    // Initialize all components
    stream1->begin(info);
    stream2->begin(info);
    stream3->begin(info);

    mixer.add(*stream1, vco1_level * 100); // 33% level for VCO1
    mixer.add(*stream2, vco2_level * 100); // 33% level for VCO2
    mixer.add(*stream3, vco3_level * 100); // 33% level for VCO3
    mixer.begin(info);

    adsr = new audio_tools::ADSRGain(
        0.001f, // Attack très rapide (1ms)
        0.01f,  // Decay rapide (10ms)
        0.8f,   // Sustain élevé (80%)
        0.05f   // Release rapide (100ms)
    );

    // Create ADSR with bowl-like envelope
    // Long attack, moderate decay, high sustain, very long release

    effectStream = new audio_tools::AudioEffectStream(mixer);
    effectStream->addEffect(adsr);
    effectStream->begin(info);
    Serial.printf("🔍 Effects in stream: %d\n", effectStream->size());
    Serial.printf("🔍 EffectStream pointer: %p\n", effectStream);
    Serial.printf("🔍 ADSR pointer: %p\n", adsr);
}

TibetanBowl::~TibetanBowl()
{

    if (adsr)
        delete adsr;
    if (stream3)
        delete stream3;
    if (stream2)
        delete stream2;
    if (stream1)
        delete stream1;
    if (vco3)
        delete vco3;
    if (vco2)
        delete vco2;
    if (vco1)
        delete vco1;
    if (effectStream)
        delete effectStream;
}

bool TibetanBowl::begin(audio_tools::AudioInfo audioInfo)
{
    info = audioInfo;

    Serial.println("TibetanBowl initialization...");

    initializeComponents();

    if (!vco1 || !vco2 || !vco3 || !adsr)
    {
        Serial.println("Error: Failed to initialize TibetanBowl components");
        return false;
    }

    Serial.println("TibetanBowl initialized successfully");
    return true;
}

void TibetanBowl::strike(float frequency, float velocity)
{
    fundamental_freq = frequency;

    // Update all VCO frequencies with harmonics and beating
    updateFrequencies();

    // Trigger ADSR envelope
    if (adsr)
    {
        adsr->keyOn(velocity);
    }
}

void TibetanBowl::release()
{
    if (adsr)
    {
        //  adsr->keyOff();
    }
}

void TibetanBowl::setHarmonicLevels(float vco1, float vco2, float vco3)
{
    vco1_level = vco1;
    vco2_level = vco2;
    vco3_level = vco3;
}

void TibetanBowl::setBeating(float vco1_cents, float vco2_cents, float vco3_cents)
{
    vco1_detune = vco1_cents; // Fundamental - no detune
    vco2_detune = vco2_cents;
    vco3_detune = vco3_cents;

    // Update frequencies if already playing
    if (fundamental_freq > 0)
    {
        updateFrequencies();
    }
}

bool TibetanBowl::isActive() const
{
    return adsr ? adsr->isActive() : false;
}

void TibetanBowl::setADSR(float attack, float decay, float sustain, float release)
{
    if (adsr)
    {
        adsr->setAttackRate(attack);
        adsr->setDecayRate(decay);
        adsr->setSustainLevel(sustain);
        adsr->setReleaseRate(release);
    }
}
void TibetanBowl::updateFrequencies()
{
    if (!vco1 || !vco2 || !vco3)
        return;

    // VCO1: Fundamental frequency
    float freq1 = fundamental_freq * 1.0f * centsToRatio(vco1_detune);
    ;

    // VCO2: 2nd harmonic with slight detuning for beating
    float freq2 = fundamental_freq * 2.0f * centsToRatio(vco2_detune);

    // VCO3: 3rd harmonic with slight detuning for beating
    float freq3 = fundamental_freq * 3.0f * centsToRatio(vco3_detune);

    // Update VCO frequencies
    vco1->setFrequency(freq1);
    vco2->setFrequency(freq2);
    vco3->setFrequency(freq3);

    Serial.printf("🎶 Frequencies: %.2f Hz\n", fundamental_freq);
}

float TibetanBowl::centsToRatio(float cents)
{
    // Convert cents to frequency ratio
    // 100 cents = 1 semitone = ratio of 2^(1/12)
    return pow(2.0f, cents / 1200.0f);
}

audio_tools::AudioStream *TibetanBowl::getAudioStream()
{
    Serial.printf("🎯 Returning effectStream: %p\n", effectStream);
    return &mixer;
}