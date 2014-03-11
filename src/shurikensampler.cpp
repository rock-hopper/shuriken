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

#include "shurikensampler.h"


ShurikenSamplerSound::ShurikenSamplerSound( const String& soundName,
                                            const SharedSampleBuffer sampleBuffer,
                                            const double sampleRate,
                                            const BigInteger& notes,
                                            const int midiNoteForNormalPitch ) :
    name( soundName ),
    sourceSampleRate( sampleRate ),
    midiNotes( notes ),
    midiRootNote( midiNoteForNormalPitch )
{
    if ( sourceSampleRate <= 0 || sampleBuffer->getNumFrames() <= 0 )
    {
        length = 0;
        attackSamples = 0;
        releaseSamples = 0;
    }
    else
    {
        length = sampleBuffer->getNumFrames();

        data = sampleBuffer;

        attackSamples = 0;
        releaseSamples = 0;
    }
}



ShurikenSamplerSound::~ShurikenSamplerSound()
{
}



bool ShurikenSamplerSound::appliesToNote( const int midiNoteNumber )
{
    return midiNotes [midiNoteNumber];
}



bool ShurikenSamplerSound::appliesToChannel( const int /*midiChannel*/ )
{
    return true;
}



//==============================================================================
ShurikenSamplerVoice::ShurikenSamplerVoice()
    : pitchRatio( 0.0 ),
      sourceSamplePosition( 0.0 ),
      lgain( 0.0f ), rgain( 0.0f ),
      attackReleaseLevel( 0 ), attackDelta( 0 ), releaseDelta( 0 ),
      isInAttack( false ), isInRelease( false )
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
    if ( const ShurikenSamplerSound* const sound = dynamic_cast<const ShurikenSamplerSound*>( s ) )
    {
        pitchRatio = pow( 2.0, (midiNoteNumber - sound->midiRootNote) / 12.0 )
                        * sound->sourceSampleRate / getSampleRate();

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        isInAttack =( sound->attackSamples > 0 );
        isInRelease = false;

        if  (isInAttack )
        {
            attackReleaseLevel = 0.0f;
            attackDelta = (float) (pitchRatio / sound->attackSamples);
        }
        else
        {
            attackReleaseLevel = 1.0f;
            attackDelta = 0.0f;
        }

        if ( sound->releaseSamples > 0 )
            releaseDelta = (float) ( -pitchRatio / sound->releaseSamples );
        else
            releaseDelta = 0.0f;
    }
    else
    {
        jassertfalse; // this object can only play ShurikenSamplerSounds!
    }
}



void ShurikenSamplerVoice::stopNote( const bool allowTailOff )
{
    if ( allowTailOff )
    {
        isInAttack = false;
        isInRelease = true;
    }
    else
    {
        clearCurrentNote();
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
void ShurikenSamplerVoice::renderNextBlock( AudioSampleBuffer& outputBuffer, int startSample, int numSamples )
{
    if ( const ShurikenSamplerSound* const playingSound =
         static_cast<ShurikenSamplerSound*>( getCurrentlyPlayingSound().get() ) )
    {
        const float* const inL = playingSound->data->getSampleData( 0, 0 );
        const float* const inR = playingSound->data->getNumChannels() > 1
                                    ? playingSound->data->getSampleData( 1, 0 ) : nullptr;

        float* outL = outputBuffer.getSampleData( 0, startSample );
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getSampleData( 1, startSample ) : nullptr;

        while ( --numSamples >= 0 )
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) ( sourceSamplePosition - pos );
            const float invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = ( inL [pos] * invAlpha + inL [pos + 1] * alpha );
            float r = ( inR != nullptr ) ? ( inR [pos] * invAlpha + inR [pos + 1] * alpha ) : l;

            l *= lgain;
            r *= rgain;

            if ( isInAttack )
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += attackDelta;

                if ( attackReleaseLevel >= 1.0f )
                {
                    attackReleaseLevel = 1.0f;
                    isInAttack = false;
                }
            }
            else if ( isInRelease )
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += releaseDelta;

                if ( attackReleaseLevel <= 0.0f )
                {
                    stopNote( false );
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

            sourceSamplePosition += pitchRatio;

            if ( sourceSamplePosition > playingSound->length )
            {
                stopNote( false );
                break;
            }
        }
    }
}
