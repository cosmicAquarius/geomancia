#include <SynthController.h>
#include <Arduino.h>
#include <AudioTools.h>
#include <Sequencer.h>
#include <TibetanBowl.h>

// Natural Minor Pentatonic Scale (Em pentatonic) - bass and mid range only
const float SynthController::range[] = {
    // Bass foundation - heavy weight here
    N_E2, N_G2, N_A2, N_B2, N_D3,
    N_E2, N_G2, N_A2, N_B2, N_D3, // Duplicate for more bass probability
    N_E2, N_G2, N_A2, N_B2, N_D3, // Triple for emphasis

    // Em pentatonic scale - base octave (main range)
    N_E3, N_G3, N_A3, N_B3, N_D4,
    N_E3, N_G3, N_A3, N_B3, N_D4, // Duplicate for more mid probability
    N_E3, N_G3, N_A3, N_B3, N_D4, // Triple for emphasis

    // Power chords and intervals (bass/mid range)
    N_E2, N_B2, // Perfect 5th bass
    N_G2, N_D3, // Perfect 5th bass
    N_A2, N_E3, // Perfect 5th bass to mid
    N_E3, N_B3, // Perfect 5th mid
    N_G3, N_D4, // Perfect 5th mid

    // Octave pairs for resonance (bass/mid focused)
    N_E2, N_E3, // Bass octave E
    N_G2, N_G3, // Bass octave G
    N_A2, N_A3, // Bass octave A
    N_B2, N_B3, // Bass octave B
    N_D3, N_D4, // Bass octave D

    // Additional harmonic notes (lower register only)
    N_F2, N_C3, N_F3, N_C4, // Color tones in lower registers

    // Very low bass for deep foundation
    N_E1, N_G1, N_A1 // Sub-bass notes
};

const uint8_t SynthController::NUM_NOTES = sizeof(range) / sizeof(range[0]);

SynthController::SynthController()
    : sineWave(nullptr), sound(nullptr), tibetanBowl(nullptr), useBowlMode(true), info(44100, 2, 16)
{
}

SynthController::~SynthController()
{
    if (tibetanBowl)
    {
        delete tibetanBowl;
    }
    if (sound)
    {
        delete sound;
    }
    if (sineWave)
    {
        delete sineWave;
    }
}

bool SynthController::begin(audio_tools::AudioInfo audioInfo)
{
    info = audioInfo;

    Serial.println("SynthController initialization...");

    // Initialize audio components
    initializeAudioComponents();

    // Initialize Tibetan Bowl
    tibetanBowl = new TibetanBowl();
    if (!tibetanBowl->begin(info))
    {
        Serial.println("Warning: Failed to initialize TibetanBowl");
    }

    if (!sineWave || !sound)
    {
        Serial.println("Error: Failed to initialize audio components");
        return false;
    }

    // Connect both generators to sequencer
    sequencer.setAudioGenerator(sineWave); // CETTE LIGNE MANQUAIT !
    sequencer.setBowlGenerator(tibetanBowl);

    // Start in sine mode by default
    sequencer.setBowlMode(false);

    Serial.println("SynthController initialized successfully");
    return true;
}

void SynthController::update()
{
    // Update sequencer timing and note triggers
    sequencer.update();
}

void SynthController::createJazzPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating jazz pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0);
    }

    sequencer.setBPM(bpm);
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    for (uint8_t i = 0; i < numSteps; i++)
    {
        // Jazz swing pattern - skip some steps for syncopation
        bool active = (i % 4 != 3) || (random(100) < 25);

        if (active)
        {
            float note = range[random(NUM_NOTES)];
            uint8_t velocity = random(70, 128); // Dynamic variation
            uint8_t gate = random(20, 70);      // Staccato feel

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E4, 100, 50);
        }
    }

    Serial.printf("Jazz pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

void SynthController::createAfricanPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating African pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0);
    }

    sequencer.setBPM(bpm);
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    for (uint8_t i = 0; i < numSteps; i++)
    {
        // African polyrhythm - complex rhythm patterns
        bool active = (i % 3 != 2) || (random(100) < 35);

        if (active)
        {
            float note = range[random(NUM_NOTES)];
            uint8_t velocity = random(80, 120); // Consistent power
            uint8_t gate = random(60, 95);      // Sustained notes

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E4, 100, 50);
        }
    }

    Serial.printf("African pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

void SynthController::createBowlPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating Tibetan Bowl pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0);
    }

    sequencer.setBPM(bpm);
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    // Bowl frequencies - focus on perfect 5ths and octaves for resonance

    uint8_t numBowlFreqs = sizeof(range) / sizeof(range[0]);

    for (uint8_t i = 0; i < numSteps; i++)
    {
        // Bowl pattern: sparse, meditative spacing
        bool active = (i % 4 == 0) || (i % 7 == 0) || (random(100) < 60);

        if (active)
        {
            float note = range[random(numBowlFreqs)];
            uint8_t velocity = random(20, 60); // Gentle dynamics
            uint8_t gate = random(85, 99);     // Very long sustains

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E4, 100, 50);
        }
    }

    Serial.printf("Bowl pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

void SynthController::generateRandomPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Generating random pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0);
    }

    sequencer.setBPM(bpm);
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    for (uint8_t i = 0; i < numSteps; i++)
    {
        bool active = random(100) < 65; // 65% chance of active step

        if (active)
        {
            float note = range[random(NUM_NOTES)];
            uint8_t velocity = random(50, 128);
            uint8_t gate = random(10, 90);

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E4, 100, 50);
        }
    }

    Serial.printf("Random pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

void SynthController::enableBowlMode(bool enable)
{
    useBowlMode = enable;

    // Tell sequencer to use bowl mode
    sequencer.setBowlMode(enable);

    Serial.printf("Bowl mode: %s\n", enable ? "ENABLED" : "DISABLED");
}

void SynthController::strikeBowl(float frequency, float velocity)
{
    if (tibetanBowl && useBowlMode)
    {
        tibetanBowl->strike(frequency, velocity);
    }
}

void SynthController::configureBowl(float attack, float decay, float sustain, float release)
{
    if (tibetanBowl)
    {
        tibetanBowl->setADSR(attack, decay, sustain, release);
        Serial.printf("Bowl ADSR configured: A=%.2f D=%.2f S=%.2f R=%.2f\n",
                      attack, decay, sustain, release);
    }
}

void SynthController::playSequencer()
{
    sequencer.play();
    Serial.println("Sequencer started");
}

void SynthController::stopSequencer()
{
    sequencer.stop();
    Serial.println("Sequencer stopped");
}

void SynthController::pauseSequencer()
{
    sequencer.pause();
    Serial.println("Sequencer paused");
}

audio_tools::InputMixer<int16_t> *SynthController::getAudioStream()
{
    return tibetanBowl->getAudioStream(); // Toujours le bol
}

void SynthController::initializeAudioComponents()
{
    // Create sine wave generator with moderate amplitude
    sineWave = new audio_tools::SineWaveGenerator<int16_t>(20000);

    // Create sound stream
    sound = new audio_tools::GeneratedSoundStream<int16_t>(*sineWave);

    // Initialize components
    sound->begin(info);
    sineWave->begin(info, N_E4); // Default to E4 (pentatonic root)
}
