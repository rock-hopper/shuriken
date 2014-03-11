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

SamplerAudioSource::SamplerAudioSource() : AudioSource()
{
    mNextFreeKey = DEFAULT_KEY;
}



bool SamplerAudioSource::addNewSample( const SharedSampleBuffer sampleBuffer, const qreal sampleRate )
{
    bool isSampleAssignedToKey = false;

    if ( mNextFreeKey < MAX_POLYPHONY )
    {
        mSynth.addVoice( new ShurikenSamplerVoice() );

//        AudioSampleBufferAudioFormat sampleBufferFormat;
//
//        const int numFrames = sampleBuffer->getNumFrames();
//        const int numChans = sampleBuffer->getNumChannels();
//
//        // calculate size of array of channels taking null terminator into account
//        int sizeOfArrayOfChannels = ( ( numChans + 1 ) * sizeof( float* ) )
//                                    + ( numChans * numFrames * sizeof( float ) );
//
//        ScopedPointer<AudioFormatReader> sampleReader( sampleBufferFormat.createReaderFor
//                                                       ( new MemoryInputStream
//                                                         ( sampleBuffer->getArrayOfChannels(),
//                                                           sizeOfArrayOfChannels,
//                                                           false ),
//                                                         true ));

        BigInteger keyNum;
        keyNum.clear();
        keyNum.setBit( mNextFreeKey );

//        mSynth.addSound( new SamplerSound( "key" + mNextFreeKey,
//                                          *sampleReader,
//                                          keyNum,
//                                          mNextFreeKey,   // root midi key
//                                          0.0,  // attack time
//                                          0.0,  // release time
//                                          3600.0  // maximum sample length in seconds
//                                          ));

        mSynth.addSound( new ShurikenSamplerSound( "key" + mNextFreeKey,
                                                   sampleBuffer,
                                                   sampleRate,
                                                   keyNum,
                                                   mNextFreeKey   // root midi key
                                                   ));

        mNextFreeKey++;
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
}



void SamplerAudioSource::prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate )
{
    mMidiCollector.reset( sampleRate );

    mSynth.setCurrentPlaybackSampleRate( sampleRate );
}



void SamplerAudioSource::releaseResources()
{
}



void SamplerAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
{
    // The synth always adds its output to the audio buffer, so we have to clear it first
    bufferToFill.clearActiveBufferRegion();

    // Fill a MIDI buffer with incoming messages from the MIDI input
    MidiBuffer incomingMidi;
    mMidiCollector.removeNextBlockOfMessages( incomingMidi, bufferToFill.numSamples );

    // Tell the synth to process the MIDI events and generate its output
    mSynth.renderNextBlock( *bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples );
}



