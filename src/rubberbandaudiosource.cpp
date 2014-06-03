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

#include "rubberbandaudiosource.h"
//#include <QDebug>


//==================================================================================================
// Public:

RubberbandAudioSource::RubberbandAudioSource( AudioSource* const source, const int numChans ) :
    AudioSource(),
    mSource( source ),
    mNumChans( numChans ),
    mOptions( RubberBandStretcher::OptionProcessRealTime | RubberBandStretcher::OptionPitchHighConsistency ),
    mStretcher( NULL ),
    mInputBuffer( numChans, 8192 ),
    mTimeRatio( 1.0 ),
    mPrevTimeRatio( 1.0 )
{
}



RubberbandAudioSource::~RubberbandAudioSource()
{
    releaseResources();
}



void RubberbandAudioSource::prepareToPlay( int samplesPerBlockExpected, double sampleRate )
{
    if ( mStretcher == NULL )
    {
        mStretcher = new RubberBandStretcher( sampleRate, mNumChans, mOptions );
    }

    mStretcher->reset();

    mSource->prepareToPlay( samplesPerBlockExpected, sampleRate );
}



void RubberbandAudioSource::releaseResources()
{
    if ( mStretcher != NULL )
    {
        delete mStretcher;
        mStretcher = NULL;
    }

    mSource->releaseResources();
}



void RubberbandAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill )
{
    if ( mTimeRatio != mPrevTimeRatio)
    {
        mStretcher->setTimeRatio( mTimeRatio );
        mPrevTimeRatio = mTimeRatio;
    }

//    const int latency = mStretcher->getLatency() + mReserveSize;
//    std::cerr << "latency = " << latency << std::endl;

    while ( mStretcher->available() < bufferToFill.numSamples )
    {
        readNextBufferChunk();
    }

    mStretcher->retrieve( bufferToFill.buffer->getArrayOfChannels(), bufferToFill.numSamples );
}



void RubberbandAudioSource::readNextBufferChunk()
{
    const int numRequired = mStretcher->getSamplesRequired();

    if ( numRequired > 0 )
    {
        AudioSourceChannelInfo info;
        info.buffer = &mInputBuffer;
        info.startSample = 0;
        info.numSamples = qMin( mInputBuffer.getNumFrames(), numRequired );

        mSource->getNextAudioBlock( info );

        mStretcher->process( mInputBuffer.getArrayOfChannels(), info.numSamples, false );
    }
    else
    {
        mStretcher->process( mInputBuffer.getArrayOfChannels(), 0, false );
    }
}
