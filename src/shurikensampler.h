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

#ifndef SHURIKENSAMPLER_H
#define SHURIKENSAMPLER_H

#include "JuceHeader.h"
#include "samplebuffer.h"

//==============================================================================
/**
A subclass of SynthesiserSound that represents a sampled audio clip.

To use it, create a Synthesiser, add some ShurikenSamplerVoice objects to it, then
give it some SampledSound objects to play.

@see SamplerVoice, Synthesiser, SynthesiserSound
*/
class ShurikenSamplerSound : public SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.

This will attempt to load the audio from the source into memory and store
it in this object.

@param name a name for the sample
@param sampleBuffer this object can be safely deleted by the
caller after this constructor returns
@param midiNotes the set of midi keys that this sound should be played on. This
is used by the SynthesiserSound::appliesToNote() method
@param midiNoteForNormalPitch the midi note at which the sample should be played
with its natural rate. All other notes will be pitched
up or down relative to this one
*/
    ShurikenSamplerSound( const String& name,
                  const SharedSampleBuffer sampleBuffer,
                  const double sampleRate,
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch );

    /** Destructor. */
    ~ShurikenSamplerSound();

    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const noexcept { return mName; }

    /** Returns the audio sample data.
This could return nullptr if there was a problem loading the data.
*/
    SharedSampleBuffer getAudioData() const noexcept { return mData; }

    // startFrame inclusive, endFrame exclusive
    void setPlaybackRange( const int startFrame, const int endFrame );


    //==============================================================================
    bool appliesToNote( const int midiNoteNumber ) override;
    bool appliesToChannel( const int midiChannel ) override;


private:
    //==============================================================================
    friend class ShurikenSamplerVoice;

    String mName;
    SharedSampleBuffer mData;
    double mSourceSampleRate;
    BigInteger mMidiNotes;
    int mLength, mAttackSamples, mReleaseSamples;
    int mMidiRootNote;

    volatile int mStartFrame, mEndFrame;

    JUCE_LEAK_DETECTOR( ShurikenSamplerSound )
};


//==============================================================================
/**
A subclass of SynthesiserVoice that can play a ShurikenSamplerSound.

To use it, create a Synthesiser, add some ShurikenSamplerVoice objects to it, then
give it some SampledSound objects to play.

@see SamplerSound, Synthesiser, SynthesiserVoice
*/
class ShurikenSamplerVoice : public SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a ShurikenSamplerVoice. */
    ShurikenSamplerVoice();

    /** Destructor. */
    ~ShurikenSamplerVoice();

    //==============================================================================
    bool canPlaySound( SynthesiserSound* ) override;

    void startNote( int midiNoteNumber, float velocity, SynthesiserSound*, int pitchWheel ) override;
    void stopNote( bool allowTailOff ) override;

    void pitchWheelMoved( int newValue );
    void controllerMoved( int controllerNumber, int newValue ) override;

    void renderNextBlock( AudioSampleBuffer&, int startSample, int numSamples ) override;


private:
    //==============================================================================
    double mPitchRatio;
    double mSourceSamplePosition;
    float mLeftGain, mRightGain, mAttackReleaseLevel, mAttackDelta, mReleaseDelta;
    bool mIsInAttack, mIsInRelease;

    JUCE_LEAK_DETECTOR( ShurikenSamplerVoice )
};

#endif // SHURIKENSAMPLER_H
