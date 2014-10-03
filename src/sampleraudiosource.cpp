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
    mFileSampleRate( 0.0 ),
    mPlaybackSampleRate( 0.0 ),
    mNextFreeNote( Midi::MIDDLE_C ),
    mFirstNote( Midi::MIDDLE_C ),
    mIsPlaySeqEnabled( false ),
    mIsLoopingEnabled( false ),
    mNoteCounter( 0 ),
    mFrameCounter( 0 )
{
}



SamplerAudioSource::~SamplerAudioSource()
{
    mSampler.clearVoices();
    mSampler.clearSounds();
}



void SamplerAudioSource::setSamples( const QList<SharedSampleBuffer> sampleBufferList, const qreal sampleRate )
{
    clearSamples();
    mSampleBufferList = sampleBufferList;
    mFileSampleRate = sampleRate;

    if ( sampleBufferList.size() > Midi::MAX_POLYPHONY - Midi::MIDDLE_C )
    {
        mNextFreeNote = qMax( Midi::MAX_POLYPHONY - sampleBufferList.size(), 0 );
    }

    mFirstNote = mNextFreeNote;

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
    const int midiNoteNum = mFirstNote + sampleNum;
    const float velocity = 1.0;

    SynthesiserSound* sound = mSampler.getSound( sampleNum );

    ShurikenSamplerSound* const samplerSound = dynamic_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        stop();
        samplerSound->setTempSampleRange( sampleRange );
        mSampler.noteOn( midiChannel, midiNoteNum, velocity );
    }
}



void SamplerAudioSource::playSamples( const int firstSampleNum, const QList<SharedSampleRange> sampleRangeList )
{
    mTempSampleRangeList = sampleRangeList;
    mSeqStartNote = mFirstNote + firstSampleNum;
    mNoteCounter = 0;
    mNoteCounterEnd = sampleRangeList.size();
    mFrameCounter = 0;
    mIsPlaySeqEnabled = true;
}



void SamplerAudioSource::playAll()
{
    mSeqStartNote = mFirstNote;
    mNoteCounter = 0;
    mNoteCounterEnd = mSampleBufferList.size();
    mFrameCounter = 0;
    mIsPlaySeqEnabled = true;
}



void SamplerAudioSource::stop()
{
    const int midiChannel = 1;
    const bool allowTailOff = false;

    mIsPlaySeqEnabled = false;
    mSampler.allNotesOff( midiChannel, allowTailOff );
    mTempSampleRangeList.clear();
}



void SamplerAudioSource::setLooping( const bool isLoopingDesired )
{
    mIsLoopingEnabled = isLoopingDesired;
}



void SamplerAudioSource::prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
{
    mPlaybackSampleRate = sampleRate;
    mMidiCollector.reset( sampleRate );
    mSampler.setCurrentPlaybackSampleRate( sampleRate );
}



void SamplerAudioSource::releaseResources()
{
    // Do not clear the voices, sample ranges or sample buffer here as 'releaseResources()' could be
    // called when the user is simply changing the playback sample rate in the options dialog
}



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
{
    // The synth always adds its output to the audio buffer, so we have to clear it first
    bufferToFill.clearActiveBufferRegion();


    // Fill a MIDI buffer with incoming messages from the MIDI input
    MidiBuffer incomingMidi;
    mMidiCollector.removeNextBlockOfMessages( incomingMidi, bufferToFill.numSamples );


    // If requested, play all samples in sequence by adding appropriate MIDI messages to the buffer
    if ( mIsPlaySeqEnabled )
    {
        // End of note
        if ( mFrameCounter - 1 >= 0 && mFrameCounter - 1 < bufferToFill.numSamples )
        {
            mNoteCounter++;

            if ( mNoteCounter == mNoteCounterEnd )
            {
                if ( mIsLoopingEnabled )
                {
                    mNoteCounter = 0;
                }
                else
                {
                    mIsPlaySeqEnabled = false;  // End of sequence reached
                    mTempSampleRangeList.clear();
                }
            }
        }

        if ( mIsPlaySeqEnabled )
        {
            // Start of note
            if ( mFrameCounter < bufferToFill.numSamples )
            {
                const MidiMessage message = MidiMessage::noteOn( 1,                             // MIDI channel
                                                                 mSeqStartNote + mNoteCounter,  // MIDI note no.
                                                                 1.0f );                        // Velocity
                const int noteOnFrameNum = mFrameCounter;

                incomingMidi.addEvent( message, noteOnFrameNum );

                int numFrames = 0;

                if ( ! mTempSampleRangeList.isEmpty() )
                {
                    SynthesiserSound* sound = mSampler.getSound( mSeqStartNote - mFirstNote + mNoteCounter );

                    ShurikenSamplerSound* const samplerSound = static_cast<ShurikenSamplerSound*>( sound );

                    if ( samplerSound != NULL )
                    {
                        samplerSound->setTempSampleRange( mTempSampleRangeList.at( mNoteCounter ) );
                    }
                    numFrames = mTempSampleRangeList.at( mNoteCounter )->numFrames;
                }
                else
                {
                    numFrames = mSampleBufferList.at( mNoteCounter )->getNumFrames();
                }
                mFrameCounter = roundToInt( numFrames * (mPlaybackSampleRate / mFileSampleRate) );
                mFrameCounter -= bufferToFill.numSamples - noteOnFrameNum;
            }
            else // Middle of note
            {
                mFrameCounter -= bufferToFill.numSamples;
            }
        }
    }


    // Tell the sampler to process the MIDI events and generate its output
    mSampler.renderNextBlock( *bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples );
}



//==================================================================================================
// Private:

bool SamplerAudioSource::addNewSample( const SharedSampleBuffer sampleBuffer, const qreal sampleRate )
{
    bool isSampleAssignedToKey = false;

    if ( mNextFreeNote < Midi::MAX_POLYPHONY )
    {
        mSampler.addVoice( new ShurikenSamplerVoice() );

        BigInteger noteNum;
        noteNum.clear();
        noteNum.setBit( mNextFreeNote );

        mSampler.addSound( new ShurikenSamplerSound( sampleBuffer,
                                                     sampleRate,
                                                     noteNum,           // MIDI note this sample should be assigned to
                                                     mNextFreeNote ));  // Root/pitch-centre MIDI note

        mNextFreeNote++;
        isSampleAssignedToKey = true;
    }

    return isSampleAssignedToKey;
}



void SamplerAudioSource::clearSamples()
{
    mIsPlaySeqEnabled = false;
    mSampler.clearVoices();
    mSampler.clearSounds();
    mSampleBufferList.clear();
    mNextFreeNote = Midi::MIDDLE_C;
    mFirstNote = Midi::MIDDLE_C;
}
