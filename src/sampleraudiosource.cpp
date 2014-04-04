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


//==================================================================================================
// Public:

SamplerAudioSource::SamplerAudioSource() : PositionableAudioSource()
{
    mNextFreeKey = DEFAULT_KEY;
    mTotalNumFrames = 0;
    mNextPlayPos = 0;
}



bool SamplerAudioSource::addNewSample( const SharedSampleBuffer sampleBuffer, const qreal sampleRate )
{
    bool isSampleAssignedToKey = false;

    if ( mNextFreeKey < MAX_POLYPHONY )
    {
        mSynth.addVoice( new ShurikenSamplerVoice() );

        BigInteger keyNum;
        keyNum.clear();
        keyNum.setBit( mNextFreeKey );

        mSynth.addSound( new ShurikenSamplerSound( "key" + mNextFreeKey,    // Sample name
                                                   sampleBuffer,
                                                   sampleRate,
                                                   keyNum,                  // MIDI key this sample should be mapped to
                                                   mNextFreeKey             // Root/pitch-centre MIDI key
                                                   ));

        mNextFreeKey++;
        mTotalNumFrames += sampleBuffer->getNumFrames();
        isSampleAssignedToKey = true;
    }

    return isSampleAssignedToKey;
}



bool SamplerAudioSource::setSamples( const QList<SharedSampleBuffer> sampleBufferList, const qreal sampleRate )
{
    clearAllSamples();

    if ( sampleBufferList.size() >= MAX_POLYPHONY - DEFAULT_KEY )
    {
        mNextFreeKey = qMax( MAX_POLYPHONY - sampleBufferList.size(), 0 );
    }

    bool isEverySampleAssignedToKey = false;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        isEverySampleAssignedToKey = addNewSample( sampleBuffer, sampleRate );
    }

    return isEverySampleAssignedToKey;
}



void SamplerAudioSource::clearAllSamples()
{
    mSynth.clearVoices();
    mSynth.clearSounds();
    mNextFreeKey = DEFAULT_KEY;
    mTotalNumFrames = 0;
    mNextPlayPos = 0;
}



void SamplerAudioSource::play()
{
    const int midiChannel = 1;
    const int midiNoteNum = DEFAULT_KEY; // C4
    const float velocity = 1.0;

    mSynth.noteOn( midiChannel, midiNoteNum, velocity );
}



void SamplerAudioSource::stop()
{
    const int midiChannel = 1;
    const int midiNoteNum = DEFAULT_KEY; // C4
    const bool allowTailOff = false;

    mSynth.noteOff( midiChannel, midiNoteNum, allowTailOff );
}



void SamplerAudioSource::prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
{
    mMidiCollector.reset( sampleRate );

    mSynth.setCurrentPlaybackSampleRate( sampleRate );
}



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
{
//    const ScopedLock sl( mStartPosLock );

    // The synth always adds its output to the audio buffer, so we have to clear it first
    bufferToFill.clearActiveBufferRegion();

    // Fill a MIDI buffer with incoming messages from the MIDI input
    MidiBuffer incomingMidi;
    mMidiCollector.removeNextBlockOfMessages( incomingMidi, bufferToFill.numSamples );

    // Tell the synth to process the MIDI events and generate its output
    mSynth.renderNextBlock( *bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples );
}



void SamplerAudioSource::setNextReadPosition( int64 newPosition )
{
//    const ScopedLock sl( mStartPosLock );
//
//    mNextPlayPos = newPosition;
}



int64 SamplerAudioSource::getTotalLength() const
{
    return mTotalNumFrames;
}
