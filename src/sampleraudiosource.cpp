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
#include <QDebug>


//==================================================================================================
// Public:

SamplerAudioSource::SamplerAudioSource() :
    AudioSource(),
    mFileSampleRate( 0.0 ),
    mPlaybackSampleRate( 0.0 ),
    mNextFreeKey( DEFAULT_KEY ),
    mStartKey( DEFAULT_KEY ),
    mIsPlaySeqEnabled( false ),
    mNoteCounter( 0 ),
    mFrameCounter( 0 )
{
}



SamplerAudioSource::~SamplerAudioSource()
{
    mSampler.clearVoices();
    mSampler.clearSounds();
}



void SamplerAudioSource::setSample( const SharedSampleBuffer sampleBuffer, const qreal sampleRate )
{
    clearSampleRanges();
    mSampleBuffer.clear();

    SharedSampleRange sampleRange( new SampleRange );
    sampleRange->startFrame = 0;
    sampleRange->numFrames = sampleBuffer->getNumFrames();

    addNewSample( sampleBuffer, sampleRange, sampleRate );
    mSampleBuffer = sampleBuffer;
    mFileSampleRate = sampleRate;

    mSampleRangeList << sampleRange;
}



void SamplerAudioSource::setSampleRanges( const QList<SharedSampleRange> sampleRangeList )
{
    if ( ! mSampleBuffer.isNull() )
    {
        clearSampleRanges();
        mSampleRangeList = sampleRangeList;

        if ( sampleRangeList.size() > MAX_POLYPHONY - DEFAULT_KEY )
        {
            mNextFreeKey = qMax( MAX_POLYPHONY - sampleRangeList.size(), 0 );
        }

        mStartKey = mNextFreeKey;

        int i = 0;
        while (  i < sampleRangeList.size() && i < MAX_POLYPHONY )
        {
            addNewSample( mSampleBuffer, sampleRangeList.at( i ), mFileSampleRate );
            i++;
        }

        updateNoteOnFrameNumList();
    }
}



void SamplerAudioSource::playRange( const SharedSampleRange sampleRange )
{
    const int midiChannel = 1;
    const int midiNoteNum = mStartKey;
    const float velocity = 1.0;

    SynthesiserSound* sound = mSampler.getSound( 0 );

    ShurikenSamplerSound* const samplerSound = dynamic_cast<ShurikenSamplerSound*>( sound );

    if ( samplerSound != NULL )
    {
        stop();
        samplerSound->setTempSampleRange( sampleRange );
        mSampler.noteOn( midiChannel, midiNoteNum, velocity );
    }
}



void SamplerAudioSource::playAll()
{
    mIsPlaySeqEnabled = true;
    mNoteCounter = 0;
    mFrameCounter = 0;
}



void SamplerAudioSource::stop()
{
    const int midiChannel = 1;
    const bool allowTailOff = false;
    mIsPlaySeqEnabled = false;

    mSampler.allNotesOff( midiChannel, allowTailOff );
}



void SamplerAudioSource::prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
{
    mPlaybackSampleRate = sampleRate;
    mMidiCollector.reset( sampleRate );
    mSampler.setCurrentPlaybackSampleRate( sampleRate );
    updateNoteOnFrameNumList();
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

    // If requested, play back all sample ranges in sequence by adding appropriate MIDI messages to the buffer
    if ( mIsPlaySeqEnabled )
    {
        int noteOnFrameNum = mNoteOnFrameNumList.at( mNoteCounter );

        while ( noteOnFrameNum >= mFrameCounter &&
                noteOnFrameNum < mFrameCounter + bufferToFill.numSamples )
        {
            MidiMessage message = MidiMessage( MidiMessage::noteOn( 1,                          // MIDI channel
                                                                    mStartKey + mNoteCounter,   // MIDI note no.
                                                                    1.0f ));                    // Velocity

            incomingMidi.addEvent( message, noteOnFrameNum % bufferToFill.numSamples );

            mNoteCounter++;

            if ( mNoteCounter < mNoteOnFrameNumList.size() )
            {
                noteOnFrameNum = mNoteOnFrameNumList.at( mNoteCounter );
            }
            else
            {
                mIsPlaySeqEnabled = false;
                break;
            }
        }
        mFrameCounter += bufferToFill.numSamples;
    }

    // Tell the sampler to process the MIDI events and generate its output
    mSampler.renderNextBlock( *bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples );
}



//==================================================================================================
// Private:

bool SamplerAudioSource::addNewSample( const SharedSampleBuffer sampleBuffer,
                                       const SharedSampleRange sampleRange,
                                       const qreal sampleRate )
{
    bool isSampleAssignedToKey = false;

    if ( mNextFreeKey < MAX_POLYPHONY )
    {
        mSampler.addVoice( new ShurikenSamplerVoice() );

        BigInteger keyNum;
        keyNum.clear();
        keyNum.setBit( mNextFreeKey );

        mSampler.addSound( new ShurikenSamplerSound( sampleBuffer,
                                                     sampleRange,
                                                     sampleRate,
                                                     keyNum,              // MIDI key this sample should be mapped to
                                                     mNextFreeKey ));     // Root/pitch-centre MIDI key

        mNextFreeKey++;
        isSampleAssignedToKey = true;
    }

    return isSampleAssignedToKey;
}



void SamplerAudioSource::clearSampleRanges()
{
    mIsPlaySeqEnabled = false;
    mSampler.clearVoices();
    mSampler.clearSounds();
    mNoteOnFrameNumList.clear();
    mSampleRangeList.clear();
    mNextFreeKey = DEFAULT_KEY;
    mStartKey = DEFAULT_KEY;
}



void SamplerAudioSource::updateNoteOnFrameNumList()
{
    if ( mFileSampleRate > 0.0 )
    {
        mNoteOnFrameNumList.clear();
        int noteOnFrameNum = 0;

        int i = 0;
        while (  i < mSampleRangeList.size() && i < MAX_POLYPHONY )
        {
            mNoteOnFrameNumList.append( noteOnFrameNum );
            noteOnFrameNum += mSampleRangeList.at( i )->numFrames * ( mPlaybackSampleRate / mFileSampleRate );
            i++;
        }
    }
}
