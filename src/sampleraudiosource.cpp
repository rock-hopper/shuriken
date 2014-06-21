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

SamplerAudioSource::SamplerAudioSource() : AudioSource()
{
    mNextFreeKey = DEFAULT_KEY;
    mStartKey = DEFAULT_KEY;
    mIsPlaySampleSeqEnabled = false;
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
    mNoteOnFrameNumList.append( 0 );
}



bool SamplerAudioSource::setSampleRanges( const QList<SharedSampleRange> sampleRangeList )
{
    bool isSuccessful = false;
    int noteOnFrameNum = 0;

    if ( ! mSampleBuffer.isNull() )
    {
        clearSampleRanges();

        if ( sampleRangeList.size() > MAX_POLYPHONY - DEFAULT_KEY )
        {
            mNextFreeKey = qMax( MAX_POLYPHONY - sampleRangeList.size(), 0 );
        }

        mStartKey = mNextFreeKey;

        foreach ( SharedSampleRange sampleRange, sampleRangeList )
        {
            isSuccessful = addNewSample( mSampleBuffer, sampleRange, mFileSampleRate );

            if ( isSuccessful )
            {
                mNoteOnFrameNumList.append( noteOnFrameNum );
                noteOnFrameNum += sampleRange->numFrames;
            }
        }
    }

    return isSuccessful;
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
    mIsPlaySampleSeqEnabled = true;
    mNoteCounter = 0;
    mFrameCounter = 0;
}



void SamplerAudioSource::stop()
{
    const int midiChannel = 1;
    const bool allowTailOff = false;
    mIsPlaySampleSeqEnabled = false;

    mSampler.allNotesOff( midiChannel, allowTailOff );
}



void SamplerAudioSource::prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
{
    mMidiCollector.reset( sampleRate );
    mSampler.setCurrentPlaybackSampleRate( sampleRate );
}



void SamplerAudioSource::releaseResources()
{
    // Do not clear the voices, sample ranges or sample buffer here as 'releaseResources()' could be
    // called when the user is simply changing the playback sample rate in the settings dialog
}



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
{
    // The synth always adds its output to the audio buffer, so we have to clear it first
    bufferToFill.clearActiveBufferRegion();

    // Fill a MIDI buffer with incoming messages from the MIDI input
    MidiBuffer incomingMidi;
    mMidiCollector.removeNextBlockOfMessages( incomingMidi, bufferToFill.numSamples );

    if ( mIsPlaySampleSeqEnabled )
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
                mIsPlaySampleSeqEnabled = false;
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
    mIsPlaySampleSeqEnabled = false;
    mNoteOnFrameNumList.clear();
    mSampler.clearVoices();
    mSampler.clearSounds();
    mNextFreeKey = DEFAULT_KEY;
    mStartKey = DEFAULT_KEY;
}
