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

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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
    ShurikenSamplerSound( SharedSampleBuffer sampleBuffer,
                          qreal sampleRate,
                          const BigInteger& midiNotes,
                          int midiNoteForNormalPitch );

    ~ShurikenSamplerSound();

    void setAttack( qreal value );  // Value should be 0.00 - 1.00
    void setRelease( qreal value ); // Value should be 0.00 - 1.00

    qreal getAttack() const                     { return m_attackValue; }
    qreal getRelease() const                    { return m_releaseValue; }

    void setOneShot( bool set )                 { m_isOneShotSet = set; }
    bool isOneShotSet() const                   { return m_isOneShotSet; }

    // Set temporary sample range; only lasts for duration of one note
    void setTempSampleRange( SharedSampleRange sampleRange );

    SharedSampleBuffer getSampleBuffer() const  { return m_sampleBuffer; }

    bool appliesToNote( int midiNoteNumber ) override;
    bool appliesToChannel( int midiChannel ) override;


private:
    friend class ShurikenSamplerVoice;

    const SharedSampleBuffer m_sampleBuffer;
    const int m_originalStartFrame, m_originalEndFrame;
    const qreal m_sourceSampleRate;
    BigInteger m_midiNotes;
    int m_midiRootNote;

    volatile qreal m_attackValue, m_releaseValue;
    volatile int m_startFrame, m_endFrame;
    volatile int m_tempStartFrame, m_tempEndFrame;
    volatile bool m_isTempSampleRangeSet;
    volatile bool m_isOneShotSet;
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

    void renderNextBlock( AudioSampleBuffer&, int startFrame, int numFrames ) override;

private:
    qreal m_pitchRatio;
    qreal m_sourceSamplePosition;
    float m_leftGain, m_rightGain, m_attackReleaseLevel, m_attackDelta, m_releaseDelta;
    bool m_isInAttack, m_isInRelease;
};

#endif // SHURIKENSAMPLER_H
