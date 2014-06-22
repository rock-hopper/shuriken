/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA  02110-1301, USA.

  ==============================================================================

   This file contains code which forms part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef SIMPLESYNTH_H
#define SIMPLESYNTH_H

// Basic sine wave sound
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound() {}

    bool appliesToNote( const int /*midiNoteNumber*/ ) { return true; }
    bool appliesToChannel( const int /*midiChannel*/ ) { return true; }
};



// Simple synth voice that generates sine waves
class SineWaveVoice : public SynthesiserVoice
{
public:
    SineWaveVoice() : currentAngle(0), angleDelta(0), level(0), tailOff(0)
    {
    }

    bool canPlaySound( SynthesiserSound* sound )
    {
        return dynamic_cast<SineWaveSound*>( sound ) != NULL;
    }

    void startNote( const int midiNoteNumber, const float velocity,
                    SynthesiserSound* /*sound*/, const int /*currentPitchWheelPosition*/ )
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void stopNote( const bool allowTailOff )
    {
        if ( allowTailOff )
        {
            // Start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            // We only need to begin a tail-off if it's not already doing so - the
            // stopNote method could be called more than once.
            if ( tailOff == 0.0 )
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved( const int /*newValue*/ )
    {
        // Not implemented
    }

    void controllerMoved( const int /*controllerNumber*/, const int /*newValue*/ )
    {
        // Not implemented
    }

    void renderNextBlock( AudioSampleBuffer& outputBuffer, int startSample, int numSamples )
    {
        if ( angleDelta != 0.0 )
        {
            if ( tailOff > 0 )
            {
                while ( --numSamples >= 0 )
                {
                    const float currentSample = (float)( sin (currentAngle) * level * tailOff );

                    for ( int i = outputBuffer.getNumChannels(); --i >= 0; )
                        *outputBuffer.getSampleData (i, startSample) += currentSample;

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if ( tailOff <= 0.005 )
                    {
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while ( --numSamples >= 0 )
                {
                    const float currentSample = (float)( sin (currentAngle) * level );

                    for ( int i = outputBuffer.getNumChannels(); --i >= 0; )
                        *outputBuffer.getSampleData (i, startSample) += currentSample;

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle, angleDelta, level, tailOff;
};



// An audio source that streams the output of the sine wave synthesiser
class SynthAudioSource  : public AudioSource
{
public:
    // This collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    // The synth itself!
    Synthesiser synth;


    SynthAudioSource()
    {
        // Add the sine wave voice to the synth
        const int polyphony = 10;
        for ( int voiceNum = 0; voiceNum < polyphony; voiceNum++ )
        {
            synth.addVoice( new SineWaveVoice() );
        }

        // Add the sine wave sound
        synth.clearSounds();
        synth.addSound( new SineWaveSound() );
    }

    void prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
    {
        midiCollector.reset( sampleRate );

        synth.setCurrentPlaybackSampleRate( sampleRate );
    }

    void releaseResources()
    {
    }

    void getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
    {
        // The synth always adds its output to the audio buffer, so we have to clear it first
        bufferToFill.clearActiveBufferRegion();

        // Fill a MIDI buffer with incoming messages from the MIDI input
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages( incomingMidi, bufferToFill.numSamples );

        // Tell the synth to process the MIDI events and generate its output
        synth.renderNextBlock( *bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples );
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( SynthAudioSource );
};

#endif // SIMPLESYNTH_H
