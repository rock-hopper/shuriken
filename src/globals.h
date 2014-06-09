#ifndef GLOBALS_H
#define GLOBALS_H


#define APPLICATION_NAME            "Shuriken"
#define JUCE_ALSA_MIDI_INPUT_NAME   "Midi_In"
#define NUM_INPUT_CHANS             0
#define NUM_OUTPUT_CHANS            2
#define OUTPUT_CHAN_NAMES           "out_L out_R"   // Names must be separated by whitespace

#define AUDIO_CONFIG_FILE_PATH      "~/.shuriken/audioconfig.xml"

extern volatile double gCurrentJackBPM;


#endif // GLOBALS_H
