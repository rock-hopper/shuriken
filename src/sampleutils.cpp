/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#include "sampleutils.h"


SharedSampleBuffer SampleUtils::joinSampleBuffers( const QList<SharedSampleBuffer> sampleBufferList )
{
    Q_ASSERT( ! sampleBufferList.isEmpty() );

    const int numChans = sampleBufferList.first()->getNumChannels();

    int totalNumFrames = 0;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        totalNumFrames += sampleBuffer->getNumFrames();
    }

    SharedSampleBuffer newSampleBuffer( new SampleBuffer( numChans, totalNumFrames ) );

    int startFrame = 0;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        const int numFrames = sampleBuffer->getNumFrames();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            newSampleBuffer->copyFrom( chanNum, startFrame, *sampleBuffer.data(), chanNum, 0, numFrames );
        }

        startFrame += numFrames;
    }

    return newSampleBuffer;
}



QList<SharedSampleBuffer> SampleUtils::splitSampleBuffer( const SharedSampleBuffer sampleBuffer,
                                                          QList<int> slicePointFrameNums )
{
    Q_ASSERT( ! slicePointFrameNums.isEmpty() );

    QList<SharedSampleBuffer> sampleBufferList;

    qSort( slicePointFrameNums );

    const int totalNumFrames = sampleBuffer->getNumFrames();

    if ( slicePointFrameNums.last() != totalNumFrames )
    {
        slicePointFrameNums << totalNumFrames;
    }

    const int numChans = sampleBuffer->getNumChannels();

    int startFrame = 0;

    foreach ( int frameNum, slicePointFrameNums )
    {
        if( frameNum <= totalNumFrames )
        {
            const int numFrames = frameNum - startFrame;

            if ( numFrames > 0 )
            {
                SharedSampleBuffer newSampleBuffer( new SampleBuffer( numChans, numFrames ) );

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    newSampleBuffer->copyFrom( chanNum, 0, *sampleBuffer.data(), chanNum, startFrame, numFrames );
                }

                sampleBufferList << newSampleBuffer;

                startFrame += numFrames;
            }
        }
    }

    return sampleBufferList;
}



int SampleUtils::getTotalNumFrames( QList<SharedSampleBuffer> sampleBufferList )
{
    int numFrames = 0;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        numFrames += sampleBuffer->getNumFrames();
    }

    return numFrames;
}
