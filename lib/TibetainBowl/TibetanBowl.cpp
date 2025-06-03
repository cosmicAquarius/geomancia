#include <TibetanBowl.h>

TibetanBowl::TibetanBowl()
    : vco1(nullptr), vco2(nullptr), vco3(nullptr),
      stream1(nullptr), stream2(nullptr), stream3(nullptr), 
      mixer(nullptr), adsr(nullptr), effectStream(nullptr),
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

    // Créer le mixer en tant que pointeur
    mixer = new audio_tools::InputMixer<int16_t>();
    mixer->add(*stream1, vco1_level * 100); // level for VCO1
    mixer->add(*stream2, vco2_level * 100); // level for VCO2
    mixer->add(*stream3, vco3_level * 100); // level for VCO3
    mixer->begin(info);

    adsr = new audio_tools::ADSRGain(
        0.001f, // Attack très rapide (1ms)
        0.01f,  // Decay rapide (10ms)
        0.8f,   // Sustain élevé (80%)
        0.05f   // Release rapide (100ms)
    );

    // Create ADSR with bowl-like envelope
    // Long attack, moderate decay, high sustain, very long release

    effectStream = new audio_tools::AudioEffectStream(*mixer);
    effectStream->addEffect(adsr);
    effectStream->begin(info);
    Serial.printf("🔍 Effects in stream: %d\n", effectStream->size());
    Serial.printf("🔍 EffectStream pointer: %p\n", effectStream);
    Serial.printf("🔍 ADSR pointer: %p\n", adsr);
    Serial.printf("🔍 Mixer pointer: %p\n", mixer);
}

TibetanBowl::~TibetanBowl()
{
    if (effectStream) {
        effectStream->end();
        delete effectStream;
    }
    if (adsr)
        delete adsr;
    if (mixer) {
        mixer->end();
        delete mixer;
    }
    if (stream3) {
        stream3->end();
        delete stream3;
    }
    if (stream2) {
        stream2->end();
        delete stream2;
    }
    if (stream1) {
        stream1->end();
        delete stream1;
    }
    if (vco3)
        delete vco3;
    if (vco2)
        delete vco2;
    if (vco1)
        delete vco1;
}

bool TibetanBowl::begin(audio_tools::AudioInfo audioInfo)
{
    info = audioInfo;

    Serial.println("TibetanBowl initialization...");

    initializeComponents();

    if (!vco1 || !vco2 || !vco3 || !adsr || !mixer)
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

    // VCO2: 2nd harmonic with slight detuning for beating
    float freq2 = fundamental_freq * 2.0f * centsToRatio(vco2_detune);

    // VCO3: 3rd harmonic with slight detuning for beating
    float freq3 = fundamental_freq * 3.0f * centsToRatio(vco3_detune);

    // Update VCO frequencies
    vco1->setFrequency(freq1);
    vco2->setFrequency(freq2);
    vco3->setFrequency(freq3);

   // Serial.printf("🎶 Frequencies: %.2f Hz\n", fundamental_freq);
}

float TibetanBowl::centsToRatio(float cents)
{
    // Convert cents to frequency ratio
    // 100 cents = 1 semitone = ratio of 2^(1/12)
    return pow(2.0f, cents / 1200.0f);
}

audio_tools::AudioStream *TibetanBowl::getAudioStream()
{
    Serial.printf("🎯 Returning mixer: %p\n", mixer);
    return mixer;
}

void TibetanBowl::setupVCOs(const String& style)
{
    Serial.printf("🎛️ Setting up VCOs for style: %s\n", style.c_str());
    
    if (style == "acid") {
        // ACID TECHNO AMBIENT Configuration
        Serial.println("🔊 Configuring ACID TECHNO AMBIENT preset");
        
        // === FRÉQUENCES ET DÉTUNE ===
        vco1_detune = 0.0f;     // Fondamentale stable
        vco2_detune = -8.5f;    // Légèrement désaccordé pour battements lents
        vco3_detune = 15.2f;    // Désaccordé vers le haut pour tension harmonique
        
        // === NIVEAUX DES OSCILLATEURS ===
        vco1_level = 0.85f;     // Basse forte (85%)
        vco2_level = 0.65f;     // Mid-range présent (65%)
        vco3_level = 0.45f;     // Harmoniques subtiles (45%)
        
        // === CONFIGURATION ADSR ACID ===
        if (adsr) {
            adsr->setAttackRate(0.002f);   // Attaque très rapide (2ms) - punch acid
            adsr->setDecayRate(0.08f);     // Decay modéré (80ms) - caractère acid
            adsr->setSustainLevel(0.6f);   // Sustain à 60% - maintien du groove
            adsr->setReleaseRate(0.15f);   // Release plus long (150ms) - queue ambient
        }
        
        Serial.println("✅ ACID setup complete - Ready for squelchy basslines!");
    }
    else if (style == "ambient") {
        // AMBIENT Configuration douce et atmosphérique
        Serial.println("🌊 Configuring AMBIENT preset");
        
        vco1_detune = 0.0f;     // Fondamentale pure
        vco2_detune = 3.8f;     // Détune subtil pour richesse
        vco3_detune = -2.1f;    // Contre-détune léger
        
        vco1_level = 0.5f;      // Équilibré
        vco2_level = 0.4f;      // Doux
        vco3_level = 0.6f;      // Harmoniques proéminentes
        
        if (adsr) {
            adsr->setAttackRate(0.05f);    // Attaque lente (50ms)
            adsr->setDecayRate(0.2f);      // Decay long
            adsr->setSustainLevel(0.8f);   // Sustain élevé
            adsr->setReleaseRate(0.5f);    // Release très long
        }
        
        Serial.println("✅ AMBIENT setup complete - Ethereal soundscapes ready!");
    }
    else if (style == "tibetan") {
        // TIBETAN BOWL Configuration traditionnelle
        Serial.println("🎎 Configuring TIBETAN BOWL preset");
        
        vco1_detune = 0.0f;     // Fondamentale pure
        vco2_detune = 5.0f;     // 2ème harmonique légèrement sharp
        vco3_detune = -4.2f;    // 3ème harmonique légèrement flat
        
        vco1_level = 1.0f;      // Fondamentale forte
        vco2_level = 0.6f;      // Harmoniques naturelles
        vco3_level = 0.3f;      // Harmoniques subtiles
        
        if (adsr) {
            adsr->setAttackRate(0.001f);   // Attaque instantanée
            adsr->setDecayRate(0.01f);     // Decay rapide
            adsr->setSustainLevel(0.8f);   // Sustain élevé
            adsr->setReleaseRate(0.05f);   // Release naturel
        }
        
        Serial.println("✅ TIBETAN setup complete - Traditional bowl resonance!");
    }
    else {
        Serial.printf("⚠️ Unknown style: %s. Using default tibetan configuration.\n", style.c_str());
        setupVCOs("tibetan");  // Fallback vers configuration par défaut
        return;
    }
    
    // === RECONFIGURATION COMPLÈTE DU MIXER ===
    Serial.println("🔄 Reconfiguring mixer with new levels...");
    
    // 1. Arrêter et supprimer l'effect stream
    if (effectStream) {
        effectStream->end();
        delete effectStream;
        effectStream = nullptr;
    }
    
    // 2. Arrêter et supprimer l'ancien mixer
    if (mixer) {
        mixer->end();
        delete mixer;
        mixer = nullptr;
    }
    
    // 3. Créer un nouveau mixer avec les nouveaux niveaux
    mixer = new audio_tools::InputMixer<int16_t>();
    
    if (stream1 && stream2 && stream3) {
        mixer->add(*stream1, vco1_level * 100);
        mixer->add(*stream2, vco2_level * 100); 
        mixer->add(*stream3, vco3_level * 100);
        mixer->begin(info);
        
        Serial.printf("🎚️ Mixer reconfigured - VCO1: %.1f%%, VCO2: %.1f%%, VCO3: %.1f%%\n", 
                      vco1_level * 100, vco2_level * 100, vco3_level * 100);
    }
    
    // 4. Recréer l'effect stream avec le nouveau mixer
    if (mixer && adsr) {
        effectStream = new audio_tools::AudioEffectStream(*mixer);
        effectStream->addEffect(adsr);
        effectStream->begin(info);
        Serial.println("🎛️ Effect stream recreated successfully");
    }
    
    // === MISE À JOUR DES FRÉQUENCES ===
    // Appliquer les nouveaux réglages si une note est en cours
    if (fundamental_freq > 0) {
        updateFrequencies();
      //  Serial.printf("🎵 Frequencies updated for %s style\n", style.c_str());
    }
    
    Serial.printf("🎛️ VCO Setup complete - Style: %s\n", style.c_str());
    Serial.printf("   VCO1: %.1f%% (detune: %.1f cents)\n", vco1_level * 100, vco1_detune);
    Serial.printf("   VCO2: %.1f%% (detune: %.1f cents)\n", vco2_level * 100, vco2_detune);
    Serial.printf("   VCO3: %.1f%% (detune: %.1f cents)\n", vco3_level * 100, vco3_detune);
}

// Méthode utilitaire pour changer de style à la volée
void TibetanBowl::morphToStyle(const String& targetStyle, float morphTime) {
    // TODO: Implémentation future pour transition graduelle entre styles
    Serial.printf("🌀 Morphing to %s (instant for now)\n", targetStyle.c_str());
    setupVCOs(targetStyle);
}