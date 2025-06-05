#include <SynthController.h>
#include <Arduino.h>
#include <AudioTools.h>
#include <Sequencer.h>
#include <Instrument.h>

// Natural Minor Pentatonic Scale (Em pentatonic) - bass and mid range only
const float SynthController::range[] = {
    // Gamme pentatonique minor (très électronique)
    N_A0, N_C0, N_D0, N_E0, N_G0,
    N_A0, N_C0, N_D0, N_E0, N_G0,
    N_A0, N_C0, N_D0, N_E0, N_G0,
    N_A1, N_C1, N_D1, N_E1, N_G1,
    N_A2, N_C2, N_D2, N_E2, N_G2,
   

    // Accords de techno (Am - F - C - G)
    N_A2, N_C3, N_E2, // Am
    N_F2, N_A3,  // F
    N_C3, N_E3, N_G3, // C
    N_G2, N_B3, // G

    // Basses percutantes
    N_A1, N_A2, N_E2, N_F2, N_G2,

    };

const uint8_t SynthController::NUM_NOTES = sizeof(range) / sizeof(range[0]);

SynthController::SynthController()
    : sineWave(nullptr), sound(nullptr), instrument(nullptr),  info(44100, 2, 16)
{
}

SynthController::~SynthController()
{
    if (instrument)
    {
        delete instrument;
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
    instrument = new Instrument();
    if (!instrument->begin(info))
    {
        Serial.println("Warning: Failed to initialize TibetanBowl");
    }

    // Connect both generators to sequencer
    sequencer.setAudioGenerator(sineWave); // CETTE LIGNE MANQUAIT !
    sequencer.setBowlGenerator(instrument);

    // Start in sine mode by default
    sequencer.setBowlMode(true);

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
            uint8_t gate = random(60, 70);      // Staccato feel

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E0, 100, 50);
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
            sequencer.setStep(i, false, N_E0, 100, 50);
        }
    }

    Serial.printf("African pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

// PATTERN ÉLECTRONIQUE
void SynthController::createElectronicPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating Electronic pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0) + millis();
    }
    Serial.printf("🎲 Random seed: %d\n", seedValue);

    sequencer.setBPM(bpm); // 120-140 BPM typique pour électronique
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    for (uint8_t i = 0; i < numSteps; i++)
    {
        bool active = true;

        // PATTERNS RYTHMIQUES ÉLECTRONIQUES
        if (i % 16 < 8)
        {
            // Première moitié: pattern dense
            active = (i % 2 == 0) || (random(100) < 70);
        }
        else
        {
            // Deuxième moitié: breakdown/build
            active = (i % 4 == 0) || (random(100) < 40);
        }

        if (active)
        {
            float note;
            uint8_t velocity;
            uint8_t gate;

            // RÉPARTITION PAR FRÉQUENCE
            uint8_t noteType = random(100);

            if (noteType < 40)
            {
                // 40% BASSES (kicks/sub-bass)
                note = range[random(0, 5)]; // Notes graves
                velocity = random(80, 127); // Fort
                gate = random(20, 40);      // Court et percutant
            }
            else if (noteType < 70)
            {
                // 30% ACCORDS/HARMONIES
                note = range[random(5, 20)]; // Notes moyennes
                velocity = random(40, 80);   // Moyen
                gate = random(60, 90);       // Soutenu
            }
            else
            {
                // 30% LEADS/MÉLODIES
                note = range[random(20, NUM_NOTES)]; // Notes aigues
                velocity = random(60, 100);          // Variable
                gate = random(70, 100);              // Long pour mélodie
            }

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_A2, 0, 0);
        }
    }

    Serial.printf("Electronic pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

void SynthController::createTechnoPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating Ambient Atmospheric pattern...");

    if (seedValue == 0) {
        seedValue = analogRead(A0) + millis();
    }
    Serial.printf("🌌 Random seed: %d\n", seedValue);

    sequencer.setBPM(72); // Tempo lent pour l'ambient
    sequencer.setNumSteps(32); // Pattern plus long
    randomSeed(seedValue);


    uint8_t scaleSize = sizeof(range) / sizeof(range[0]);

    // COUCHE 1: Pad de fond - notes longues et soutenues
    for (uint8_t i = 0; i < 32; i += 8) {
        if (random(100) < 85) { // 85% de chance d'activation
            float rootNote = range[random(8)]; // Notes basses/médium
            uint8_t velocity = random(45, 65); // Vélocité douce
            uint8_t gate = random(85, 100); // Notes très longues
            
            sequencer.setStep(i, true, rootNote, velocity, gate);
            
            // Accord - ajoute une quinte ou une octave
            if (i + 1 < 32 && random(100) < 60) {
                float harmonyNote = rootNote * 1.5; // Quinte parfaite
                if (harmonyNote > N_C6) harmonyNote = rootNote * 2.0; // Octave si trop haut
                sequencer.setStep(i + 1, true, harmonyNote, velocity - 10, gate);
            }
        }
    }

    // COUCHE 2: Mélodies éthérées - notes sporadiques hautes
    for (uint8_t i = 1; i < 32; i += 3) {
        if (random(100) < 35) { // 35% de chance - plus rare
            float highNote = range[random(12, scaleSize)]; // Notes hautes
            uint8_t velocity = random(25, 45); // Très doux
            uint8_t gate = random(20, 60); // Notes moyennes à courtes
            
            sequencer.setStep(i, true, highNote, velocity, gate);
        }
    }

    // COUCHE 3: Texture rythmique subtile
    for (uint8_t i = 4; i < 32; i += 6) {
        if (random(100) < 40) {
            float textureNote = range[random(8, 16)]; // Notes médium
            uint8_t velocity = random(30, 50);
            uint8_t gate = random(15, 35); // Notes courtes pour texture
            
            sequencer.setStep(i, true, textureNote, velocity, gate);
        }
    }

    // COUCHE 4: Basse drone - fondamentale
    for (uint8_t i = 0; i < 32; i += 16) {
        if (random(100) < 70) {
            float droneNote = range[random(5)]; // Notes très basses
            uint8_t velocity = random(55, 70);
            uint8_t gate = 100; // Note complètement soutenue
            
            sequencer.setStep(i, true, droneNote, velocity, gate);
        }
    }

    // COUCHE 5: Événements atmosphériques rares
    for (uint8_t i = 7; i < 32; i += 11) {
        if (random(100) < 20) { // Très rare - 20%
            float atmosphericNote = range[random(scaleSize)];
            uint8_t velocity = random(15, 35); // Très doux
            uint8_t gate = random(60, 90); // Notes longues
            
            sequencer.setStep(i, true, atmosphericNote, velocity, gate);
        }
    }
}

void SynthController::createDrumkitPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating Drumkit pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0) + millis();
    }
    Serial.printf("🎲 Random seed: %d\n", seedValue);

    sequencer.setBPM(bpm); // 128-140 BPM pour dancefloor
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    // NOTES LOCALES DRUMKIT
    // Kick (VCO1 = SINE -24 detune) - très graves
    float kickNotes[] = {N_C1, N_D1, N_E1, N_F1,N_C0, N_D0, N_E0, N_F0}; // Notes ultra-graves
    
    // Snare/Hi-hat (VCO2 = WHITE_NOISE) - moyennes/aigues  
    float snareNotes[] = {N_G3, N_A3, N_B3, N_C4, N_D4, N_E4, N_F4, N_G4};
    
    // Toms (VCO3 = SQUARE +12 detune) - moyennes
    float tomNotes[] = {N_C2, N_D2, N_E2, N_F2, N_G2, N_A2, N_B2, N_C3};

    for (uint8_t i = 0; i < numSteps; i++)
    {
        bool active = false;
        float note = N_C1;
        uint8_t velocity = 0;
        uint8_t gate = 0;

        // KICK PATTERN - temps forts pour faire bouger
        if (i % 16 == 0 || i % 16 == 8 || // Temps 1 et 3 (obligatoire)
            (i % 16 == 4 && random(100) < 60) || // Temps 2 (60% chance)
            (i % 16 == 12 && random(100) < 40))   // Temps 4 (40% chance)
        {
            active = true;
            note = kickNotes[random(0, 4)];
            velocity = random(100, 127); // KICK PUISSANT
            gate = random(15, 25);       // Court et punchy
        }
        // SNARE PATTERN - backbeat essentiel
        else if (i % 16 == 4 || i % 16 == 12 || // Temps 2 et 4 (backbeat)
                 (i % 16 == 6 && random(100) < 30) || // Ghost notes
                 (i % 16 == 14 && random(100) < 40))   // Fills
        {
            active = true;
            note = snareNotes[random(0, 8)];
            velocity = random(70, 100); // Snare medium-fort
            gate = random(10, 20);      // Claquant
        }
        // HI-HAT PATTERN - groove continu
        else if (i % 4 == 2 && random(100) < 80) // Offbeats principaux
        {
            active = true;
            note = snareNotes[random(4, 8)]; // Notes plus aigues pour hi-hat
            velocity = random(40, 70);       // Plus doux
            gate = random(5, 15);           // Très court
        }
        // TOM FILLS - transitions et énergie
        else if ((i % 32 >= 28) && random(100) < 50) // Fills en fin de phrases
        {
            active = true;
            note = tomNotes[random(0, 8)];
            velocity = random(60, 90);
            gate = random(20, 35);
        }
        // HI-HAT RAPIDE - pour l'énergie dancefloor
        else if (i % 2 == 1 && random(100) < 40) // 16th notes sporadiques
        {
            active = true;
            note = snareNotes[random(5, 8)]; // Hi-hat aigu
            velocity = random(30, 50);       // Subtil
            gate = random(3, 8);            // Ultra-court
        }

        if (active)
        {
            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_C1, 0, 0);
        }
    }

    Serial.printf("🥁 Drumkit pattern created: %d steps at %d BPM - Ready to move the dancefloor!\n", numSteps, bpm);
}

void SynthController::createAmbiantPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating Minimal Ambient pattern...");

    sequencer.setBPM(60); // Encore plus lent
    sequencer.setNumSteps(16);
    randomSeed(seedValue);

    // Échelle réduite pour plus de cohérence
    float minimalScale[] = {
        N_C2, N_F2, N_G2,     // Fondamentales
        N_C3, N_F3, N_G3,     // Médium
        N_C4, N_F4, N_G4      // Aigus
    };

    for (uint8_t i = 0; i < 16; i++) {
        // Pattern très espacé
        if (i % 4 == 0 || (i % 7 == 0 && random(100) < 60)) {
            float note = minimalScale[random(9)];
            uint8_t velocity = random(40, 60);
            uint8_t gate = random(70, 100);
            
            sequencer.setStep(i, true, note, velocity, gate);
        } else if (random(100) < 15) {
            // Événements rares
            float note = minimalScale[random(6, 9)]; // Notes plus hautes
            uint8_t velocity = random(20, 35);
            uint8_t gate = random(30, 70);
            
            sequencer.setStep(i, true, note, velocity, gate);
        }
    }

    Serial.println("Minimal ambient pattern created - perfect for meditation");
}


void SynthController::createBowlPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue)
{
    Serial.println("Creating Tibetan Bowl pattern...");

    if (seedValue == 0)
    {
        seedValue = analogRead(A0) + millis();
    }
    Serial.printf("🎲 Random seed: %d\n", seedValue);

    sequencer.setBPM(bpm);
    sequencer.setNumSteps(numSteps);
    randomSeed(seedValue);

    // Bowl frequencies - focus on perfect 5ths and octaves for resonance

    uint8_t numBowlFreqs = sizeof(range) / sizeof(range[0]);

    for (uint8_t i = 0; i < numSteps; i++)
    {
   
        bool active = (i % 3 != 2) || (random(100) < 15);

        if (active)
        {
            float note = range[random(NUM_NOTES)];
            uint8_t velocity = random(80, 90); 

            // Two gate ranges: 90% short notes, 10% long notes
            uint8_t gate;
            if (random(100) < 90)
            {
                gate = random(45, 50); // Short/rapid notes
            }
            else
            {
                gate = random(15, 100); // Long sustained notes
            }

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E1, 100, 50);
        }
    }

    Serial.printf("Bowl pattern created: %d steps at %d BPM\n", numSteps, bpm);
}

void SynthController::setBPM(uint16_t bpm)
{
    sequencer.setBPM(bpm);
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
        bool active = random(100) < 90; // 65% chance of active step

        if (active)
        {
            float note = range[random(NUM_NOTES)];
            uint8_t velocity = random(50, 128);
            uint8_t gate = random(10, 90);

            sequencer.setStep(i, true, note, velocity, gate);
        }
        else
        {
            sequencer.setStep(i, false, N_E0, 100, 50);
        }
    }

    Serial.printf("Random pattern created: %d steps at %d BPM\n", numSteps, bpm);
}



void SynthController::strikeBowl(float frequency, float velocity)
{
    if (instrument )
    {
        instrument->strike(frequency, velocity);
    }
}

void SynthController::configureBowl(float attack, float decay, float sustain, float release)
{
    if (instrument)
    {
        instrument->setADSR(attack, decay, sustain, release);
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

void SynthController::setupVCOs(const String &style)
{

    instrument->setupVCOs(style);
}

audio_tools::AudioStream *SynthController::getAudioStream()
{
    return instrument->getAudioStream(); // Toujours le bol
}
