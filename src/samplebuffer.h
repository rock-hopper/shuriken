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

#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include <QSharedPointer>
#include "JuceHeader.h"


class SampleBuffer : public AudioSampleBuffer
{
public:
    SampleBuffer() :
            AudioSampleBuffer()
    {
    }

    SampleBuffer( int numChannels, int numFrames ) :
            AudioSampleBuffer( numChannels, numFrames )
    {
    }

    SampleBuffer( float* const* dataToReferTo, int numChannels, int numFrames ) :
            AudioSampleBuffer( dataToReferTo, numChannels, numFrames )
    {
    }

    SampleBuffer( float* const* dataToReferTo, int numChannels, int startFrame, int numFrames ) :
            AudioSampleBuffer( dataToReferTo, numChannels, startFrame, numFrames )
    {
    }

    SampleBuffer( const SampleBuffer& other ) :
            AudioSampleBuffer( other )
    {
    }

    int getNumFrames() const
    {
        return getNumSamples();
    }
};

typedef QSharedPointer<SampleBuffer> SharedSampleBuffer;
typedef ScopedPointer<SampleBuffer> ScopedSampleBuffer;



struct SampleHeader
{
    QString format;
    int numChans;
    int bitsPerSample;
    qreal sampleRate;

    JUCE_LEAK_DETECTOR( SampleHeader )
};

typedef QSharedPointer<SampleHeader> SharedSampleHeader;
typedef ScopedPointer<SampleHeader> ScopedSampleHeader;



struct SampleRange
{
    SampleRange() :
        startFrame( 0 ),
        numFrames( 0 )
    {
    }

    int startFrame;
    int numFrames;

    JUCE_LEAK_DETECTOR( SampleRange )
};

typedef QSharedPointer<SampleRange> SharedSampleRange;
typedef ScopedPointer<SampleRange> ScopedSampleRange;



class SampleUtils
{
public:
    static SharedSampleBuffer joinSampleBuffers( QList<SharedSampleBuffer> sampleBufferList );

    // Slice points with a value greater than or equal to the sample buffer's no. of frames are ignored
    static QList<SharedSampleBuffer> splitSampleBuffer( SharedSampleBuffer sampleBuffer,
                                                        QList<int> slicePointFrameNums );

    static int getTotalNumFrames( QList<SharedSampleBuffer> sampleBufferList );
};


#endif // SAMPLEBUFFER_H
