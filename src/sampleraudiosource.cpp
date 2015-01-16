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

*/

#include "sampleraudiosource.h"
#include "shurikensampler.h"
#include "globals.h"
#include <QDebug>


//==================================================================================================
// Public:

SamplerAudioSource::SamplerAudioSource() :
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
    const int midiChannel = 1;
    const int midiNoteNum = m_lowestAssignedNote + sampleNum;
    const float velocity = 1.0;

    SynthesiserSound* sound = m_sampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = dynamic_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        stop();
        samplerSound->setTempSampleRange( sampleRange );
        m_sampler.noteOn( midiChannel, midiNoteNum, velocity );
    }
}



void SamplerAudioSource::playSamples( const int firstSampleNum, const QList<SharedSampleRange> sampleRangeList )
{
    m_tempSampleRangeList = sampleRangeList;
    m_seqStartNote = m_lowestAssignedNote + firstSampleNum;
    m_noteCounter = -1;
    m_noteCounterEnd = sampleRangeList.size();
    m_frameCounter = 0;
    m_isPlaySeqEnabled = true;
}



void SamplerAudioSource::playAll()
{
    m_seqStartNote = m_lowestAssignedNote;
    m_noteCounter = -1;
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
    m_tempSampleRangeList.clear();
}



void SamplerAudioSource::setLooping( const bool isLoopingDesired )
{
    m_isLoopingEnabled = isLoopingDesired;
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



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
{
    // The sampler always adds its output to the audio buffer, so we have to clear it first
    bufferToFill.clearActiveBufferRegion();


    // Fill a MIDI buffer with incoming messages from the MIDI input
    m_incomingMidi.clear();
    m_midiCollector.removeNextBlockOfMessages( m_incomingMidi, bufferToFill.numSamples );


    // If requested, play all samples in sequence by adding appropriate MIDI messages to the buffer
    if ( m_isPlaySeqEnabled )
    {
        // End of note
        if ( m_frameCounter + 1 < bufferToFill.numSamples )
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
                    m_tempSampleRangeList.clear();
                }
            }
        }

        if ( m_isPlaySeqEnabled )
        {
            // Start of note
            if ( m_frameCounter < bufferToFill.numSamples )
            {
                const MidiMessage message = MidiMessage::noteOn( 1,                                 // MIDI channel
                                                                 m_seqStartNote + m_noteCounter,    // MIDI note no.
                                                                 1.0f );                            // Velocity
                const int noteOnFrameNum = m_frameCounter;

                m_incomingMidi.addEvent( message, noteOnFrameNum );

                int numFrames = 0;

                if ( ! m_tempSampleRangeList.isEmpty() )
                {
                    SynthesiserSound* sound = m_sampler.getSound( m_seqStartNote - m_lowestAssignedNote + m_noteCounter );

                    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

                    if ( samplerSound != NULL )
                    {
                        samplerSound->setTempSampleRange( m_tempSampleRangeList.at( m_noteCounter ) );
                    }
                    numFrames = m_tempSampleRangeList.at( m_noteCounter )->numFrames;
                }
                else
                {
                    numFrames = m_sampleBufferList.at( m_noteCounter )->getNumFrames();
                }
                m_frameCounter = roundToInt( numFrames * (m_playbackSampleRate / m_fileSampleRate) );
                m_frameCounter -= bufferToFill.numSamples - noteOnFrameNum;
            }
            else // Middle of note
            {
                m_frameCounter -= bufferToFill.numSamples;
            }
        }
    }


    // Tell the sampler to process the MIDI events and generate its output
    m_sampler.renderNextBlock( *bufferToFill.buffer, m_incomingMidi, 0, bufferToFill.numSamples );
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
