/*
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

*/

#ifndef SHURIKENSAMPLER_H
#define SHURIKENSAMPLER_H

#include "JuceHeader.h"
#include "samplebuffer.h"


// A subclass of SynthesiserSound that represents a sampled audio clip.
// To use it, create a Synthesiser, add some ShurikenSamplerVoice objects to it, then
// give it some ShurikenSamplerSound objects to play.

class ShurikenSamplerSound : public SynthesiserSound
{
public:
    ShurikenSamplerSound( const SharedSampleBuffer sampleBuffer,
                          const qreal sampleRate,
                          const BigInteger& midiNotes,
                          const int midiNoteForNormalPitch );

    ~ShurikenSamplerSound();

    SharedSampleBuffer getAudioData() const     { return mData; }

    // Set temporary sample range; only lasts for duration of one note
    void setTempSampleRange( const SharedSampleRange sampleRange );

    bool appliesToNote( const int midiNoteNumber ) override;
    bool appliesToChannel( const int midiChannel ) override;


private:
    friend class ShurikenSamplerVoice;

    const SharedSampleBuffer mData;
    const int mOriginalStartFrame, mOriginalEndFrame;
    const qreal mSourceSampleRate;
    BigInteger mMidiNotes;
    int mMidiRootNote;
    int mAttackSamples, mReleaseSamples;

    volatile int mStartFrame, mEndFrame;
    volatile int mTempStartFrame, mTempEndFrame;
    volatile bool mIsTempSampleRangeSet;
};



// A subclass of SynthesiserVoice that can play a ShurikenSamplerSound.
// To use it, create a Synthesiser, add some ShurikenSamplerVoice objects to it, then
// give it some ShurikenSamplerSound objects to play.

class ShurikenSamplerVoice : public SynthesiserVoice
{
public:
    ShurikenSamplerVoice();
    ~ShurikenSamplerVoice();

    bool canPlaySound( SynthesiserSound* ) override;

    void startNote( int midiNoteNumber, float velocity, SynthesiserSound*, int pitchWheel ) override;
    void stopNote( float velocity, bool allowTailOff ) override;

    void pitchWheelMoved( int newValue );
    void controllerMoved( int controllerNumber, int newValue ) override;

    void renderNextBlock( AudioSampleBuffer&, int startSample, int numSamples ) override;

private:
    qreal mPitchRatio;
    qreal mSourceSamplePosition;
    float mLeftGain, mRightGain, mAttackReleaseLevel, mAttackDelta, mReleaseDelta;
    bool mIsInAttack, mIsInRelease;
};

#endif // SHURIKENSAMPLER_H
