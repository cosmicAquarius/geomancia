#include <Instrument.h>

Instrument::Instrument()
    : vco1(nullptr), vco2(nullptr), vco3(nullptr), vco4(nullptr),
      vco5(nullptr), vco6(nullptr), vco7(nullptr), vco8(nullptr),
      vco9(nullptr), vco10(nullptr), vco11(nullptr), vco12(nullptr),

      stream_sine(nullptr), stream_fast_sine(nullptr), stream_square(nullptr),
      stream_sawtooth(nullptr), stream_white_noise(nullptr), stream_pink_noise(nullptr),
      stream_silence(nullptr), stream_array(nullptr), stream_fixed_value(nullptr),
      stream_sine_table(nullptr), stream_sine_extra(nullptr), stream_square_extra(nullptr),

      mixer(nullptr), adsr(nullptr), effectStream(nullptr),
      info(44100, 2, 16), fundamental_freq(440.0f),
      vco1_level(1.0f),
      vco2_level(0.6f),
      vco3_level(0.3f),
      vco1_detune(12.0f), // Fundamental - no detune
      vco2_detune(5.0f),  // 2nd harmonic - slight sharp for slow beats
      vco3_detune(-4.2f), // 3rd harmonic - slight flat for complex interference
      current_sample(0),

      currentVco1(VCO_SINE), currentVco2(VCO_FAST_SINE), currentVco3(VCO_SQUARE),
      previousVco1(VCO_SINE), previousVco2(VCO_FAST_SINE), previousVco3(VCO_SQUARE)
{
}

void Instrument::initializeComponents()
{
    // DÉTUNE ÉLECTRONIQUE - intervalles musicaux
    vco1_detune = 0.0f;  // Fondamentale propre
    vco2_detune = 12.0f; // Octave haute (lead)
    vco3_detune = 0.0f;  // Quinte parfaite (harmonique)

    // Créer tous les VCO avec leurs générateurs respectifs
    vco1 = new audio_tools::SineWaveGenerator<int16_t>(5000);
    vco2 = new audio_tools::FastSineGenerator<int16_t>(5000);
    vco3 = new audio_tools::SquareWaveGenerator<int16_t>(5000);
    vco4 = new audio_tools::SawToothGenerator<int16_t>(5000);
    vco5 = new audio_tools::WhiteNoiseGenerator<int16_t>(5000);
    vco6 = new audio_tools::PinkNoiseGenerator<int16_t>(5000);
    vco7 = new audio_tools::SilenceGenerator<int16_t>(0);
    vco8 = new audio_tools::GeneratorFromArray<int16_t>();
    vco9 = new audio_tools::GeneratorFixedValue<int16_t>();
    vco10 = new audio_tools::SineFromTable<int16_t>(5000);
    vco11 = new audio_tools::SineWaveGenerator<int16_t>(3000);   // Extra Sine
    vco12 = new audio_tools::SquareWaveGenerator<int16_t>(4000); // Extra Square

    // Créer les streams pour chaque VCO
    stream_sine = new audio_tools::GeneratedSoundStream<int16_t>(*vco1);
    stream_fast_sine = new audio_tools::GeneratedSoundStream<int16_t>(*vco2);
    stream_square = new audio_tools::GeneratedSoundStream<int16_t>(*vco3);
    stream_sawtooth = new audio_tools::GeneratedSoundStream<int16_t>(*vco4);
    stream_white_noise = new audio_tools::GeneratedSoundStream<int16_t>(*vco5);
    stream_pink_noise = new audio_tools::GeneratedSoundStream<int16_t>(*vco6);
    stream_silence = new audio_tools::GeneratedSoundStream<int16_t>(*vco7);
    stream_array = new audio_tools::GeneratedSoundStream<int16_t>(*vco8);
    stream_fixed_value = new audio_tools::GeneratedSoundStream<int16_t>(*vco9);
    stream_sine_table = new audio_tools::GeneratedSoundStream<int16_t>(*vco10);
    stream_sine_extra = new audio_tools::GeneratedSoundStream<int16_t>(*vco11);
    stream_square_extra = new audio_tools::GeneratedSoundStream<int16_t>(*vco12);

    vco1->begin(info);
    vco2->begin(info); // 2nd harmonic
    vco3->begin(info); // 3rd harmonic

    stream_sine->begin(info);
    stream_fast_sine->begin(info);
    stream_square->begin(info);
    stream_sawtooth->begin(info);
    stream_white_noise->begin(info);
    stream_pink_noise->begin(info);
    stream_silence->begin(info);
    stream_array->begin(info);
    stream_fixed_value->begin(info);
    stream_sine_table->begin(info);
    stream_sine_extra->begin(info);
    stream_square_extra->begin(info);

    // Créer le mixer en tant que pointeur
    mixer = new audio_tools::InputMixer<int16_t>();

    mixer->add(*stream_sine, 100);       // SineWave
    mixer->add(*stream_fast_sine, 100);  // FastSine
    mixer->add(*stream_square, 100);     // SquareWave
    mixer->add(*stream_sawtooth, 0);     // SawTooth
    mixer->add(*stream_white_noise, 0);  // WhiteNoise
    mixer->add(*stream_pink_noise, 0);   // PinkNoise
    mixer->add(*stream_silence, 0);      // Silence
    mixer->add(*stream_array, 0);        // Array
    mixer->add(*stream_fixed_value, 0);  // FixedValue
    mixer->add(*stream_sine_table, 0);   // SineFromTable
    mixer->add(*stream_sine_extra, 0);   // Extra Sine
    mixer->add(*stream_square_extra, 0); // Extra Square

    mixer->begin(info);

    adsr = new audio_tools::ADSRGain(0.0001, 1.0000, 0.9, 0.00001, 1.0);

    // Create ADSR with bowl-like envelope
    // Long attack, moderate decay, high sustain, very long release
    //  adsr->setActive(true);
    effectStream = new audio_tools::AudioEffectStream(*mixer);
    effectStream->addEffect(adsr);
    effectStream->begin(info);

    Serial.printf("🔍 Effects in stream: %d\n", effectStream->size());
    Serial.printf("🔍 EffectStream pointer: %p\n", effectStream);
    Serial.printf("🔍 ADSR pointer: %p\n", adsr);
    Serial.printf("🔍 Mixer pointer: %p\n", mixer);
}
Instrument::~Instrument()
{
    if (effectStream)
    {
        effectStream->end();
        delete effectStream;
    }
    if (adsr)
        delete adsr;

    if (mixer)
    {
        mixer->end();
        delete mixer;
    }

    // Delete tous les streams
    if (stream_square_extra)
    {
        stream_square_extra->end();
        delete stream_square_extra;
    }
    if (stream_sine_extra)
    {
        stream_sine_extra->end();
        delete stream_sine_extra;
    }
    if (stream_sine_table)
    {
        stream_sine_table->end();
        delete stream_sine_table;
    }
    if (stream_fixed_value)
    {
        stream_fixed_value->end();
        delete stream_fixed_value;
    }
    if (stream_array)
    {
        stream_array->end();
        delete stream_array;
    }
    if (stream_silence)
    {
        stream_silence->end();
        delete stream_silence;
    }
    if (stream_pink_noise)
    {
        stream_pink_noise->end();
        delete stream_pink_noise;
    }
    if (stream_white_noise)
    {
        stream_white_noise->end();
        delete stream_white_noise;
    }
    if (stream_sawtooth)
    {
        stream_sawtooth->end();
        delete stream_sawtooth;
    }
    if (stream_square)
    {
        stream_square->end();
        delete stream_square;
    }
    if (stream_fast_sine)
    {
        stream_fast_sine->end();
        delete stream_fast_sine;
    }
    if (stream_sine)
    {
        stream_sine->end();
        delete stream_sine;
    }

    // Delete tous les VCO
    if (vco12)
        delete vco12;
    if (vco11)
        delete vco12;
    if (vco10)
        delete vco10;
    if (vco9)
        delete vco9;
    if (vco8)
        delete vco8;
    if (vco7)
        delete vco7;
    if (vco6)
        delete vco6;
    if (vco5)
        delete vco5;
    if (vco4)
        delete vco4;
    if (vco3)
        delete vco3;
    if (vco2)
        delete vco2;
    if (vco1)
        delete vco1;
}

bool Instrument::begin(audio_tools::AudioInfo audioInfo)
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

void Instrument::strike(float frequency, float velocity)
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

void Instrument::release()
{
    if (adsr)
    {
        adsr->keyOff();
    }
}

void Instrument::setVcoVolumes(float vco1, float vco2, float vco3)
{
    vco1_level = vco1;
    vco2_level = vco2;
    vco3_level = vco3;
}

void Instrument::setBeating(float vco1_cents, float vco2_cents, float vco3_cents)
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

bool Instrument::isActive() const
{
    return adsr ? adsr->isActive() : false;
}

void Instrument::setADSR(float attack, float decay, float sustain, float release)
{
    if (adsr)
    {
        adsr->setAttackRate(attack);
        adsr->setDecayRate(decay);
        adsr->setSustainLevel(sustain);
        adsr->setReleaseRate(release);
    }
}

void Instrument::updateFrequencies()
{
    if (!vco1 || !vco2 || !vco3)
        return;

    // VCO1: Fundamental frequency
    float freq1 = fundamental_freq * centsToRatio(vco1_detune);

    // VCO2: 2nd harmonic with slight detuning for beating
    float freq2 = fundamental_freq * -3.0f * centsToRatio(vco2_detune);

    // VCO3: 3rd harmonic with slight detuning for beating
    float freq3 = fundamental_freq * 3.0f * centsToRatio(vco3_detune);

    // Update VCO frequencies
    vco1->setFrequency(freq1);
    vco2->setFrequency(freq2);
    vco3->setFrequency(freq3);

    // Serial.printf("🎶 Frequencies: %.2f Hz\n", fundamental_freq);
}

float Instrument::centsToRatio(float cents)
{
    // Convert cents to frequency ratio
    // 100 cents = 1 semitone = ratio of 2^(1/12)
    return pow(2.0f, cents / 1200.0f);
}

void Instrument::setupVCOs(const String &style)
{

    previousVco1 = currentVco1;
    previousVco2 = currentVco2;
    previousVco3 = currentVco3;

    Serial.printf("🎛️ Setting up VCOs for style: %s\n", style.c_str());
    if (style == "tibetan")
    {
        // TIBETAN BOWL Configuration traditionnelle

        currentVco1 = VCO_SQUARE; // Basse
        currentVco2 = VCO_SINE;   // Lead
        currentVco3 = VCO_SINE;   // Harmon
        // currentVco3 = VCO_WHITE_NOISE; // Snare/Hi-hat

        vco1_detune = 0.0f;  // Fondamentale pure
        vco2_detune = 3.0f;  // 2ème harmonique légèrement sharp
        vco3_detune = -3.0f; // 3ème harmonique légèrement flat

        vco1_level = 1.0f; // Fondamentale forte
        vco2_level = 0.6f; // Harmoniques naturelles
        vco3_level = 0.7f; // Harmoniques subtiles

        if (adsr)
        {
            // pour mémoire voici ce qu'on doit obtenir avec le tibetan
            // ADSRGain adsr(0.0001, 1.0000, 0.9, 0.00001, 1.0);
            // const unsigned long NOTE_INTERVAL = 5000;
            // donc tempo très lent
            // durée de note 500-100
            // SquareWaveGenerator<int16_t> vco1(1000);    // Basse
            // SineWaveGenerator<int16_t> vco2(6000);      // Lead
            // SineWaveGenerator<int16_t> vco3(4000);      // Harmonique
            adsr->setAttackRate(0.0001f);   // ✅
            adsr->setDecayRate(1.0000f);    // ✅
            adsr->setSustainLevel(0.9f);    // ✅
            adsr->setReleaseRate(0.00001f); // ✅
        }

        Serial.println("✅ TIBETAN setup complete - Traditional bowl resonance!");
    }
    else if (style == "acid")
    {
        // ACID TECHNO AMBIENT Configuration

        currentVco1 = VCO_SAWTOOTH;
        currentVco2 = VCO_SQUARE;
        currentVco3 = VCO_SAWTOOTH;

        // === FRÉQUENCES ET DÉTUNE ===
        vco1_detune = 0.0f;   // Fondamentale stable
        vco2_detune = -12.0f; // Légèrement désaccordé pour battements lents
        vco3_detune = 12.01f; // Désaccordé vers le haut pour tension harmonique

        // === NIVEAUX DES OSCILLATEURS ===
        vco1_level = 0.85f; // Basse forte (85%)
        vco2_level = 0.65f; // Mid-range présent (65%)
        vco3_level = 0.45f; // Harmoniques subtiles (45%)

        // === CONFIGURATION ADSR ACID ===
        if (adsr)
        {
            adsr->setAttackRate(0.0001f);   // ✅
            adsr->setDecayRate(1.0000f);    // ✅
            adsr->setSustainLevel(0.9f);    // ✅
            adsr->setReleaseRate(0.00001f); // ✅
        }

        Serial.println("✅ ACID setup complete - Traditional bowl resonance!");
    }
    else if (style == "techno")
    {

        currentVco1 = VCO_SQUARE;
        currentVco2 = VCO_SAWTOOTH;
        currentVco3 = VCO_SQUARE_EXTRA;

        // === FRÉQUENCES ET DÉTUNE ===
        vco1_detune = -12.0f; // Fondamentale stable
        vco2_detune = -6.0f;  // Légèrement désaccordé pour battements lents
        vco3_detune = 12.01f; // Désaccordé vers le haut pour tension harmonique

        // === NIVEAUX DES OSCILLATEURS ===
        vco1_level = 0.85f; // Basse forte (85%)
        vco2_level = 0.65f; // Mid-range présent (65%)
        vco3_level = 0.45f; // Harmoniques subtiles (45%)

        // === CONFIGURATION ADSR ACID ===
        if (adsr)
        {
            adsr->setAttackRate(0.0001f);   // ✅
            adsr->setDecayRate(1.0000f);    // ✅
            adsr->setSustainLevel(0.9f);    // ✅
            adsr->setReleaseRate(0.00001f); // ✅
        }

        Serial.println("✅ TECHNO setup complete - Traditional bowl resonance!");
    }
    else if (style == "ambient")
    {
        // AMBIENT Configuration douce et atmosphérique
        Serial.println("🌊 Configuring AMBIENT preset");

        currentVco1 = VCO_SINE;   // Kick (basse fréquence)
        currentVco2 = VCO_SINE;   // Snare/Hi-hat
        currentVco3 = VCO_SQUARE; // Tom/percussion

        vco1_detune = -36.0f; // Fondamentale pure
        vco2_detune = -12.0f; // Détune subtil pour richesse
        vco3_detune = -6.0f;  // Contre-détune léger

        vco1_level = 0.9f;   // Équilibré
        vco2_level = 0.005f; // Doux
        vco3_level = 0.8f;   // Harmoniques proéminentes

        if (adsr)
        {
            adsr->setAttackRate(0.0001f);   // ✅
            adsr->setDecayRate(1.0000f);    // ✅
            adsr->setSustainLevel(0.9f);    // ✅
            adsr->setReleaseRate(0.00001f); // ✅
        }

        Serial.println("✅ AMBIENT setup complete - Ethereal soundscapes ready!");
    }

    else if (style == "drumkit")
    {


        currentVco1 = VCO_SINE;        // Kick (basse fréquence)
        currentVco2 = VCO_WHITE_NOISE; // Snare/Hi-hat
        currentVco3 = VCO_SQUARE;      // Tom/percussion

        vco1_detune = -36.0f;  // Kick très grave
        vco2_detune = 0.0001f; // Noise sans détune
        vco3_detune = 6.0f;    // Tom aigu

        vco1_level = 1.0f;  // Kick fort
        vco2_level = 1.0f;  // Snare présent
        vco3_level = 0.05f; // Tom modéré

        if (adsr)
        {
            adsr->setAttackRate(0.0011f);   // Attack très rapide (punch)
            adsr->setDecayRate(0.1f);       // Decay rapide (son percussif)
            adsr->setSustainLevel(1.0f);    // Sustain très bas
            adsr->setReleaseRate(0.00005f); // Release court
        }

        Serial.println("✅ DRUMKIT setup complete - Ready to bang!");
    }

    else
    {
        Serial.printf("⚠️ Unknown style: %s. Using default tibetan configuration.\n", style.c_str());
        setupVCOs("tibetan"); // Fallback vers configuration par défaut
        return;
    }

    // === RECONFIGURATION COMPLÈTE DU MIXER ===
    Serial.println("🔄 Reconfiguring mixer with new levels...");

    // === MISE À JOUR DES FRÉQUENCES ===
    // Appliquer les nouveaux réglages si une note est en cours
    if (fundamental_freq > 0)
    {
        updateFrequencies();
        //  Serial.printf("🎵 Frequencies updated for %s style\n", style.c_str());
    }

    Serial.printf("🎛️ VCO Setup complete - Style: %s\n", style.c_str());
    Serial.printf("   VCO1: %.1f%% (detune: %.1f cents)\n", vco1_level * 100, vco1_detune);
    Serial.printf("   VCO2: %.1f%% (detune: %.1f cents)\n", vco2_level * 100, vco2_detune);
    Serial.printf("   VCO3: %.1f%% (detune: %.1f cents)\n", vco3_level * 100, vco3_detune);
}

void Instrument::update()
{
    static bool morphingInProgress = false;
    static uint32_t morphingStartTick = 0;
    // attention this is not time in ms but in ticks
    static const uint32_t morphingDurationTicks = 20000;
    static bool morphingSetup = false;

    // Détecter changement (check rapide)
    bool needsMorphing = (currentVco1 != previousVco1) ||
                         (currentVco2 != previousVco2) ||
                         (currentVco3 != previousVco3);

    // Commencer le morphing
    if (needsMorphing && !morphingInProgress)
    {
        morphingInProgress = true;
        morphingStartTick = xTaskGetTickCount();
        morphingSetup = false;
        // Pas de Serial.println ici (trop lent pour audioTask)
    }

    // Processus de morphing (seulement si actif)
    if (morphingInProgress)
    {
        uint32_t elapsedTicks = xTaskGetTickCount() - morphingStartTick;

        // Setup une seule fois
        if (!morphingSetup)
        {
            if (currentVco1 != previousVco1)
            {
                replaceVcoInMixer(3, currentVco1);
                setChannelWeight(3, 0);
            }
            if (currentVco2 != previousVco2)
            {
                replaceVcoInMixer(4, currentVco2);
                setChannelWeight(4, 0);
            }
            if (currentVco3 != previousVco3)
            {
                replaceVcoInMixer(5, currentVco3);
                setChannelWeight(5, 0);
            }
            morphingSetup = true;
        }

        // Crossfade (calcul optimisé)
        if (elapsedTicks < morphingDurationTicks)
        {
            float progress = (float)elapsedTicks / morphingDurationTicks;
            float oldVol = 1.0f - progress;
            float newVol = progress;

            // Mise à jour volumes (rapide)
            setChannelWeight(0, vco1_level * 100 * oldVol);
            setChannelWeight(1, vco2_level * 100 * oldVol);
            setChannelWeight(2, vco3_level * 100 * oldVol);

            if (currentVco1 != previousVco1)
                setChannelWeight(3, vco1_level * 100 * newVol);
            if (currentVco2 != previousVco2)
                setChannelWeight(4, vco2_level * 100 * newVol);
            if (currentVco3 != previousVco3)
                setChannelWeight(5, vco3_level * 100 * newVol);
        }
        else
        {
            // Morphing terminé - finalisation rapide
            if (currentVco1 != previousVco1)
            {
                replaceVcoInMixer(0, currentVco1);
                setChannelWeight(3, 0);
            }
            if (currentVco2 != previousVco2)
            {
                replaceVcoInMixer(1, currentVco2);
                setChannelWeight(4, 0);
            }
            if (currentVco3 != previousVco3)
            {
                replaceVcoInMixer(2, currentVco3);
                setChannelWeight(5, 0);
            }

            setChannelWeight(0, vco1_level * 100);
            setChannelWeight(1, vco2_level * 100);
            setChannelWeight(2, vco3_level * 100);

            previousVco1 = currentVco1;
            previousVco2 = currentVco2;
            previousVco3 = currentVco3;

            morphingInProgress = false;
        }
    }
}

void Instrument::replaceVcoInMixer(int channel, int newVcoIndex)
{
    if (!mixer)
    {
        Serial.printf("Error: mixer not initialized\n");
        return;
    }

    if (channel >= 0 && channel < 12)
    {
        switch (newVcoIndex)
        {
        case 1:
            mixer->set(channel, *stream_sine);
            break; // SineWave
        case 2:
            mixer->set(channel, *stream_fast_sine);
            break; // FastSine
        case 3:
            mixer->set(channel, *stream_square);
            break; // SquareWave
        case 4:
            mixer->set(channel, *stream_sawtooth);
            break; // SawTooth
        case 5:
            mixer->set(channel, *stream_white_noise);
            break; // WhiteNoise
        case 6:
            mixer->set(channel, *stream_pink_noise);
            break; // PinkNoise
        case 7:
            mixer->set(channel, *stream_silence);
            break; // Silence
        case 8:
            mixer->set(channel, *stream_array);
            break; // Array
        case 9:
            mixer->set(channel, *stream_fixed_value);
            break; // FixedValue
        case 10:
            mixer->set(channel, *stream_sine_table);
            break; // SineFromTable
        case 12:
            mixer->set(channel, *stream_sine_extra);
            break; // Extra Sine
        case 13:
            mixer->set(channel, *stream_square_extra);
            break; // Extra Square
        default:
            Serial.printf("VCO %d does not exist\n", newVcoIndex);
            return;
        }

        VcoType newVcoType = static_cast<VcoType>(newVcoIndex);

        // Mettre à jour les variables de tracking pour les 3 premiers canaux
        switch (channel)
        {
        case 0:
            previousVco1 = currentVco1;
            currentVco1 = newVcoType;
            break;
        case 1:
            previousVco2 = currentVco2;
            currentVco2 = newVcoType;
            break;
        case 2:
            previousVco3 = currentVco3;
            currentVco3 = newVcoType;
            break;
        }

        Serial.printf("Channel %d now uses VCO %d\n", channel, newVcoIndex);
    }
    else
    {
        Serial.printf("Invalid channel %d - max is 11\n", channel);
    }
}

// Changer le niveau (weight) d'un canal spécifique
void Instrument::setChannelWeight(int channel, int weight)
{
    if (!mixer)
    {
        Serial.printf("Error: mixer not initialized\n");
        return;
    }

    mixer->setWeight(channel, weight);
}

// Méthode utilitaire pour changer de style à la volée
void Instrument::morphToStyle(const String &targetStyle, float morphTime)
{
    // TODO: Implémentation future pour transition graduelle entre styles
    Serial.printf("🌀 Morphing to %s (instant for now)\n", targetStyle.c_str());
    setupVCOs(targetStyle);
}

audio_tools::AudioStream *Instrument::getAudioStream()
{
    Serial.printf("🎯 Returning mixer: %p\n", mixer);
    // return &mixer;
    return effectStream;
}
