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

#include "shurikensampler.h"
//#include <QDebug>


ShurikenSamplerSound::ShurikenSamplerSound( const SharedSampleBuffer sampleBuffer,
                                            const qreal sampleRate,
                                            const BigInteger& notes,
                                            const int midiNoteForNormalPitch ) :
    m_data( sampleBuffer ),
    m_originalStartFrame( 0 ),
    m_originalEndFrame( sampleBuffer->getNumFrames() - 1 ),
    m_sourceSampleRate( sampleRate ),
    m_midiNotes( notes ),
    m_midiRootNote( midiNoteForNormalPitch )
{
    m_attackSamples = 0;
    m_releaseSamples = 0;

    m_startFrame = m_originalStartFrame;
    m_endFrame = m_originalEndFrame;

    m_tempStartFrame = m_originalStartFrame;
    m_tempEndFrame = m_originalEndFrame;

    m_isTempSampleRangeSet = false;
}



ShurikenSamplerSound::~ShurikenSamplerSound()
{
}



void ShurikenSamplerSound::setTempSampleRange( const SharedSampleRange sampleRange )
{
    m_tempStartFrame = sampleRange->startFrame;
    m_tempEndFrame = sampleRange->startFrame + sampleRange->numFrames - 1;

    m_isTempSampleRangeSet = true;
}



bool ShurikenSamplerSound::appliesToNote( const int midiNoteNumber )
{
    return m_midiNotes[ midiNoteNumber ];
}



bool ShurikenSamplerSound::appliesToChannel( const int /*midiChannel*/ )
{
    return true;
}



//==============================================================================
ShurikenSamplerVoice::ShurikenSamplerVoice()
    : m_pitchRatio( 0.0 ),
      m_sourceSamplePosition( 0.0 ),
      m_leftGain( 0.0f ), m_rightGain( 0.0f ),
      m_attackReleaseLevel( 0 ), m_attackDelta( 0 ), m_releaseDelta( 0 ),
      m_isInAttack( false ), m_isInRelease( false )
{
}



ShurikenSamplerVoice::~ShurikenSamplerVoice()
{
}



bool ShurikenSamplerVoice::canPlaySound( SynthesiserSound* sound )
{
    return dynamic_cast<const ShurikenSamplerSound*>( sound ) != nullptr;
}



void ShurikenSamplerVoice::startNote( const int midiNoteNumber,
                                      const float velocity,
                                      SynthesiserSound* s,
                                      const int /*currentPitchWheelPosition*/ )
{
    if ( ShurikenSamplerSound* const sound = dynamic_cast<ShurikenSamplerSound*>( s ) )
    {
        m_pitchRatio = pow( 2.0, (midiNoteNumber - sound->m_midiRootNote) / 12.0 )
                        * sound->m_sourceSampleRate / getSampleRate();

        if ( sound->m_isTempSampleRangeSet )
        {
            sound->m_startFrame = sound->m_tempStartFrame;
            sound->m_endFrame = sound->m_tempEndFrame;
            sound->m_isTempSampleRangeSet = false;
        }

        m_sourceSamplePosition = sound->m_startFrame;
        m_leftGain = velocity;
        m_rightGain = velocity;

        m_isInAttack =( sound->m_attackSamples > 0 );
        m_isInRelease = false;

        if ( m_isInAttack )
        {
            m_attackReleaseLevel = 0.0f;
            m_attackDelta = (float) (m_pitchRatio / sound->m_attackSamples);
        }
        else
        {
            m_attackReleaseLevel = 1.0f;
            m_attackDelta = 0.0f;
        }

        if ( sound->m_releaseSamples > 0 )
            m_releaseDelta = (float) ( -m_pitchRatio / sound->m_releaseSamples );
        else
            m_releaseDelta = 0.0f;
    }
    else
    {
        jassertfalse; // this object can only play ShurikenSamplerSounds!
    }
}



void ShurikenSamplerVoice::stopNote( const float /*velocity*/, const bool allowTailOff )
{
    ShurikenSamplerSound* const playingSound =
             dynamic_cast<ShurikenSamplerSound*>( getCurrentlyPlayingSound().get() );

    if ( allowTailOff )
    {
        m_isInAttack = false;
        m_isInRelease = true;
    }
    else
    {
        clearCurrentNote();
    }

    if ( playingSound != NULL )
    {
        playingSound->m_startFrame = playingSound->m_originalStartFrame;
        playingSound->m_endFrame = playingSound->m_originalEndFrame;
    }
}



void ShurikenSamplerVoice::pitchWheelMoved( const int /*newValue*/ )
{
}



void ShurikenSamplerVoice::controllerMoved( const int /*controllerNumber*/,
                                            const int /*newValue*/ )
{
}



//==============================================================================
void ShurikenSamplerVoice::renderNextBlock( AudioSampleBuffer& outputBuffer, int startFrame, int numFrames )
{
    if ( const ShurikenSamplerSound* const playingSound =
         static_cast<ShurikenSamplerSound*>( getCurrentlyPlayingSound().get() ) )
    {
        const float* const inL = playingSound->m_data->getReadPointer( 0 );
        const float* const inR = playingSound->m_data->getNumChannels() > 1
                                    ? playingSound->m_data->getReadPointer( 1 ) : nullptr;

        float* outL = outputBuffer.getWritePointer( 0, startFrame );
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer( 1, startFrame ) : nullptr;

        const int totalnumFrames = playingSound->m_data->getNumFrames();

        while ( --numFrames >= 0 )
        {
            const int pos = (int) m_sourceSamplePosition;
            const float alpha = (float) ( m_sourceSamplePosition - pos );
            const float invAlpha = 1.0f - alpha;

            float l = 0;
            float r = 0;

            // just using a very simple linear interpolation here..
            if ( pos + 1 < totalnumFrames )
            {
                l = ( inL [pos] * invAlpha + inL [pos + 1] * alpha );
                r = ( inR != nullptr ) ? ( inR [pos] * invAlpha + inR [pos + 1] * alpha ) : l;
            }
            else
            {
                l = ( inL [pos] * invAlpha );
                r = ( inR != nullptr ) ? ( inR [pos] * invAlpha ) : l;
            }

            l *= m_leftGain;
            r *= m_rightGain;

            if ( m_isInAttack )
            {
                l *= m_attackReleaseLevel;
                r *= m_attackReleaseLevel;

                m_attackReleaseLevel += m_attackDelta;

                if ( m_attackReleaseLevel >= 1.0f )
                {
                    m_attackReleaseLevel = 1.0f;
                    m_isInAttack = false;
                }
            }
            else if ( m_isInRelease )
            {
                l *= m_attackReleaseLevel;
                r *= m_attackReleaseLevel;

                m_attackReleaseLevel += m_releaseDelta;

                if ( m_attackReleaseLevel <= 0.0f )
                {
                    stopNote( 0.0f, false );
                    break;
                }
            }

            if ( outR != nullptr )
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            m_sourceSamplePosition += m_pitchRatio;

            if ( m_sourceSamplePosition > playingSound->m_endFrame )
            {
                stopNote( 0.0f, false );
                break;
            }
        }
    }
}
