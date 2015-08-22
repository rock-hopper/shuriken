/*
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

#include "sampleraudiosource.h"
#include "shurikensampler.h"
#include "globals.h"
//#include <QtDebug>


//==================================================================================================
// Public:

SamplerAudioSource::SamplerAudioSource() :
    QObject(),
    AudioSource(),
    m_fileSampleRate( 0.0 ),
    m_playbackSampleRate( 0.0 ),
    m_nextFreeNote( Midi::MIDDLE_C ),
    m_lowestAssignedNote( Midi::MIDDLE_C ),
    m_isPlaySeqEnabled( false ),
    m_isLoopingEnabled( false ),
    m_noteCounter( 0 ),
    m_frameCounter( 0 )
{
}



SamplerAudioSource::~SamplerAudioSource()
{
    m_sampler.clearVoices();
    m_sampler.clearSounds();
}



void SamplerAudioSource::setSamples( const QList<SharedSampleBuffer> sampleBufferList, const qreal sampleRate )
{
    clearSamples();
    m_sampleBufferList = sampleBufferList;
    m_fileSampleRate = sampleRate;

    if ( sampleBufferList.size() > Midi::MAX_POLYPHONY - Midi::MIDDLE_C )
    {
        m_nextFreeNote = qMax( Midi::MAX_POLYPHONY - sampleBufferList.size(), 0 );
    }

    m_lowestAssignedNote = m_nextFreeNote;

    int i = 0;
    while (  i < sampleBufferList.size() && i < Midi::MAX_POLYPHONY )
    {
        addNewSample( sampleBufferList.at( i ), sampleRate );
        i++;
    }
}



void SamplerAudioSource::playSample( const int sampleNum, const SharedSampleRange sampleRange )
{
    if ( m_isPlaySeqEnabled )
    {
        stop();
    }
    m_tempSampleRange = sampleRange;
    m_seqStartNote = m_lowestAssignedNote + sampleNum;
    m_noteCounter = 0;
    m_noteCounterEnd = 1;
    m_frameCounter = 0;
    m_isPlaySeqEnabled = true;
}



void SamplerAudioSource::playAll()
{
    if ( m_isPlaySeqEnabled )
    {
        stop();
    }
    m_seqStartNote = m_lowestAssignedNote;
    m_noteCounter = 0;
    m_noteCounterEnd = m_sampleBufferList.size();
    m_frameCounter = 0;
    m_isPlaySeqEnabled = true;
}



void SamplerAudioSource::stop()
{
    const int midiChannel = 1;
    const bool allowTailOff = false;

    m_isPlaySeqEnabled = false;
    m_sampler.allNotesOff( midiChannel, allowTailOff );
    m_tempSampleRange.clear();
}



void SamplerAudioSource::setLooping( const bool isLoopingDesired )
{
    m_isLoopingEnabled = isLoopingDesired;
}



qreal SamplerAudioSource::getAttack( const int sampleNum ) const
{
    qreal value = 0.0;

    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        value = samplerSound->getAttack();
    }

    return value;
}



void SamplerAudioSource::setAttack( const int sampleNum, const qreal value )
{
    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        samplerSound->setAttack( value );
    }
}



qreal SamplerAudioSource::getRelease( const int sampleNum ) const
{
    qreal value = 0.0;

    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        value = samplerSound->getRelease();
    }

    return value;
}



void SamplerAudioSource::setRelease( const int sampleNum, const qreal value )
{
    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        samplerSound->setRelease( value );
    }
}



bool SamplerAudioSource::isOneShotSet( const int sampleNum ) const
{
    bool isEnabled = false;

    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        isEnabled = samplerSound->isOneShotSet();
    }

    return isEnabled;
}



void SamplerAudioSource::setOneShot( const int sampleNum, const bool set )
{
    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        samplerSound->setOneShot( set );
    }
}



void SamplerAudioSource::getEnvelopeSettings( EnvelopeSettings& settings ) const
{
    for ( int i = 0; i < m_sampleBufferList.size(); i++ )
    {
        settings.attackValues << getAttack( i );
        settings.releaseValues << getRelease( i );
        settings.oneShotSettings << isOneShotSet( i );
    }
}



void SamplerAudioSource::setEnvelopeSettings( const EnvelopeSettings& settings )
{
    if ( settings.attackValues.size() == m_sampleBufferList.size() )
    {
        for ( int i = 0; i < m_sampleBufferList.size(); i++ )
        {
            setAttack( i, settings.attackValues.at( i ) );
            setRelease( i, settings.releaseValues.at( i ) );
            setOneShot( i, settings.oneShotSettings.at( i ) );
        }
    }
}



int SamplerAudioSource::getOutputPairNum( const int sampleNum ) const
{
    int outputPairNum = 0;

    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        outputPairNum = samplerSound->getOutputPairNum();
    }

    return outputPairNum;
}



void SamplerAudioSource::prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
{
    m_playbackSampleRate = sampleRate;
    m_midiCollector.reset( sampleRate );
    m_sampler.setCurrentPlaybackSampleRate( sampleRate );
}



void SamplerAudioSource::releaseResources()
{
    // Do not clear the voices, sample ranges or sample buffer here as 'releaseResources()' could be
    // called when the user is simply changing the playback sample rate in the options dialog
}



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& info )
{
    getNextAudioBlock( info, m_midiBuffer );
}



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& info, MidiBuffer& midiBuffer )
{
    // The sampler always adds its output to the audio buffer, so we have to clear it first
    info.clearActiveBufferRegion();


    // Fill the MIDI buffer with incoming messages from the MIDI input
    midiBuffer.clear();
    m_midiCollector.removeNextBlockOfMessages( midiBuffer, info.numSamples );


    // If requested, play all samples in sequence by adding appropriate MIDI messages to the buffer
    if ( m_isPlaySeqEnabled )
    {
        // End of note
        if ( m_frameCounter > 0 && m_frameCounter <= info.numSamples )
        {
            m_noteCounter++;

            if ( m_noteCounter == m_noteCounterEnd )
            {
                if ( m_isLoopingEnabled )
                {
                    m_noteCounter = 0;
                }
                else
                {
                    m_isPlaySeqEnabled = false;  // End of sequence reached
                    m_tempSampleRange.clear();
                }
            }
        }

        if ( m_isPlaySeqEnabled )
        {
            // Start of note
            if ( m_frameCounter < info.numSamples )
            {
                const MidiMessage message = MidiMessage::noteOn( 1,                                 // MIDI channel
                                                                 m_seqStartNote + m_noteCounter,    // MIDI note no.
                                                                 1.0f );                            // Velocity
                const int noteOnFrameNum = m_frameCounter;

                midiBuffer.addEvent( message, noteOnFrameNum );

                int numFrames = 0;

                if ( ! m_tempSampleRange.isNull() )
                {
                    SynthesiserSound* sound = m_sampler.getSound( m_seqStartNote - m_lowestAssignedNote + m_noteCounter );

                    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

                    if ( samplerSound != NULL )
                    {
                        samplerSound->setTempSampleRange( m_tempSampleRange );
                    }
                    numFrames = m_tempSampleRange->numFrames;
                }
                else
                {
                    numFrames = m_sampleBufferList.at( m_noteCounter )->getNumFrames();
                }
                m_frameCounter = static_cast<int>( numFrames * (m_playbackSampleRate / m_fileSampleRate) );
                m_frameCounter -= info.numSamples - noteOnFrameNum;
            }
            else // Middle of note
            {
                m_frameCounter -= info.numSamples;
            }
        }
    }


    // Tell the sampler to process the MIDI events and generate its output
    m_sampler.renderNextBlock( *info.buffer, midiBuffer, 0, info.numSamples );
}



//==================================================================================================
// Public Slots:

void SamplerAudioSource::setOutputPair( const int sampleNum, const int outputPairNum )
{
    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        samplerSound->setOutputPair( outputPairNum );
    }
}



//==================================================================================================
// Private:

bool SamplerAudioSource::addNewSample( const SharedSampleBuffer sampleBuffer, const qreal sampleRate )
{
    bool isSampleAssignedToKey = false;

    if ( m_nextFreeNote < Midi::MAX_POLYPHONY )
    {
        m_sampler.addVoice( new ShurikenSamplerVoice() );

        BigInteger noteNum;
        noteNum.clear();
        noteNum.setBit( m_nextFreeNote );

        m_sampler.addSound( new ShurikenSamplerSound( sampleBuffer,
                                                      sampleRate,
                                                      noteNum,           // MIDI note this sample should be assigned to
                                                      m_nextFreeNote )); // Root/pitch-centre MIDI note

        m_nextFreeNote++;
        isSampleAssignedToKey = true;
    }

    return isSampleAssignedToKey;
}



void SamplerAudioSource::clearSamples()
{
    m_isPlaySeqEnabled = false;
    m_sampler.clearVoices();
    m_sampler.clearSounds();
    m_sampleBufferList.clear();
    m_nextFreeNote = Midi::MIDDLE_C;
    m_lowestAssignedNote = Midi::MIDDLE_C;
}
