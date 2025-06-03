#ifndef SYNTHCONTROLLER_H
#define SYNTHCONTROLLER_H

#include "Arduino.h"
#include "AudioTools.h"
#include <Sequencer.h>
#include <TibetanBowl.h>

class SynthController
{
private:
    // Audio components
    audio_tools::SineWaveGenerator<int16_t> *sineWave;
    audio_tools::GeneratedSoundStream<int16_t> *sound;

    // Sequencer
    Sequencer sequencer;

    // Tibetan Bowl
    TibetanBowl *tibetanBowl;
   

    // Audio configuration
    audio_tools::AudioInfo info;

    // Note scale for pattern generation
    static const float range[];
    static const uint8_t NUM_NOTES;

public:
    SynthController();
    ~SynthController();

    // Initialization
    bool begin(audio_tools::AudioInfo audioInfo);

    // Update method to be called in audio loop
    void update();

    // Sequencer control
    void createJazzPattern(uint8_t numSteps = 64, uint16_t bpm = 120, uint16_t seedValue = 0);
    void createAfricanPattern(uint8_t numSteps = 64, uint16_t bpm = 140, uint16_t seedValue = 0);
    void createBowlPattern(uint8_t numSteps = 16, uint16_t bpm = 45, uint16_t seedValue = 0);
    // Pattern Creation Methods
    void createElectronicPattern(uint8_t numSteps, uint16_t bpm, uint16_t seedValue = 0);
    void createTechnoPattern(uint8_t numSteps, uint16_t bpm);
    void createAcidPattern(uint8_t numSteps, uint16_t bpm);
    void generateRandomPattern(uint8_t numSteps = 64, uint16_t bpm = 80, uint16_t seedValue = 0);

    void playSequencer();
    void stopSequencer();
    void pauseSequencer();

   
    void strikeBowl(float frequency, float velocity = 1.0f);
    void configureBowl(float attack = 0.1f, float decay = 0.2f, float sustain = 0.7f, float release = 8.0f);
    audio_tools::AudioStream *getAudioStream();
    // Getters for status
    uint8_t getCurrentStep() const { return sequencer.getCurrentStep(); }
    uint8_t getNumSteps() const { return sequencer.getNumSteps(); }
    uint16_t getBPM() const { return sequencer.getBPM(); }
    bool isPlaying() const { return sequencer.getState() == Sequencer::PLAYING; }


    // Dans SynthController.h - Ajouter dans la section public:
    void setupVCOs(const String &style);

private:
    void initializeAudioComponents();
};

#endif // SYNTHCONTROLLER_H