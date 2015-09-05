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
#include <QtDebug>


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



int SampleUtils::getTotalNumFrames( const QList<SharedSampleBuffer> sampleBufferList )
{
    int numFrames = 0;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        numFrames += sampleBuffer->getNumFrames();
    }

    return numFrames;
}



int SampleUtils::getPrevZeroCrossing( const SharedSampleBuffer sampleBuffer, const int startFrameNum )
{
    int zeroCrossingFrameNum = 0;

    const int numFrames = sampleBuffer->getNumFrames();

    if ( startFrameNum >= 0 && startFrameNum < numFrames )
    {
        const int numChans = sampleBuffer->getNumChannels();

        for ( int chanNum = 0; chanNum < numChans; ++chanNum )
        {
            int frameNum = startFrameNum;

            // If the value of the start sample is positive then find the previous negative sample
            if ( sampleBuffer->getSample( chanNum, startFrameNum ) > 0.0f )
            {
                while ( frameNum > 0 && sampleBuffer->getSample( chanNum, frameNum ) > 0.0f )
                    --frameNum;
            }
            // If the value of the start sample is negative then find the previous positive sample
            else if ( sampleBuffer->getSample( chanNum, startFrameNum ) < 0.0f )
            {
                while ( frameNum > 0 && sampleBuffer->getSample( chanNum, frameNum ) < 0.0f )
                    --frameNum;
            }

            // Determine the smallest of the two sample values at the zero-crossing
            if ( frameNum + 1 < numFrames )
            {
                const float leftAbsSampleValue = qAbs( sampleBuffer->getSample(chanNum, frameNum) );
                const float rightAbsSampleValue = qAbs( sampleBuffer->getSample(chanNum, frameNum + 1) );

                if ( rightAbsSampleValue < leftAbsSampleValue )
                    ++frameNum;
            }

            // Determine whether the zero-crossing on this channel is closest to the start frame
            if ( frameNum > zeroCrossingFrameNum )
            {
                zeroCrossingFrameNum = frameNum;
            }
        }
    }

    return zeroCrossingFrameNum;
}



int SampleUtils::getNextZeroCrossing( const SharedSampleBuffer sampleBuffer, const int startFrameNum )
{
    int zeroCrossingFrameNum = sampleBuffer->getNumFrames() - 1;

    const int numFrames = sampleBuffer->getNumFrames();

    if ( startFrameNum >= 0 && startFrameNum < numFrames )
    {
        const int numChans = sampleBuffer->getNumChannels();

        for ( int chanNum = 0; chanNum < numChans; ++chanNum )
        {
            int frameNum = startFrameNum;

            // If the value of the start sample is positive then find the next negative sample
            if ( sampleBuffer->getSample( chanNum, startFrameNum ) > 0.0f )
            {
                while ( frameNum < numFrames - 1 && sampleBuffer->getSample( chanNum, frameNum ) > 0.0f )
                    ++frameNum;
            }
            // If the value of the start sample is negative then find the next positive sample
            else if ( sampleBuffer->getSample( chanNum, startFrameNum ) < 0.0f )
            {
                while ( frameNum < numFrames - 1 && sampleBuffer->getSample( chanNum, frameNum ) < 0.0f )
                    ++frameNum;
            }

            // Determine the smallest of the two sample values at the zero-crossing
            if ( frameNum - 1 >= 0 )
            {
                const float leftAbsSampleValue = qAbs( sampleBuffer->getSample(chanNum, frameNum - 1) );
                const float rightAbsSampleValue = qAbs( sampleBuffer->getSample(chanNum, frameNum) );

                if ( leftAbsSampleValue < rightAbsSampleValue )
                    --frameNum;
            }

            // Determine whether the zero-crossing on this channel is closest to the start frame
            if ( frameNum < zeroCrossingFrameNum )
            {
                zeroCrossingFrameNum = frameNum;
            }
        }
    }

    return zeroCrossingFrameNum;
}



int SampleUtils::getClosestZeroCrossing( const SharedSampleBuffer sampleBuffer, const int startFrameNum )
{
    const int prevZeroCrossing = getPrevZeroCrossing( sampleBuffer, startFrameNum );
    const int nextZeroCrossing = getNextZeroCrossing( sampleBuffer, startFrameNum );

    if ( startFrameNum - prevZeroCrossing < nextZeroCrossing - startFrameNum )
    {
        return prevZeroCrossing;
    }
    else
    {
        return nextZeroCrossing;
    }
}
