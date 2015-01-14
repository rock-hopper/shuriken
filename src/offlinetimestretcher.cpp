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

#include "offlinetimestretcher.h"
#include <unistd.h>


//==================================================================================================
// Public Static:

int OfflineTimeStretcher::stretch( const SharedSampleBuffer sampleBuffer,
                                   const int sampleRate,
                                   const int numChans,
                                   RubberBandStretcher::Options options,
                                   const qreal timeRatio,
                                   const qreal pitchScale )
{
    RubberBandStretcher stretcher( sampleRate, numChans, options, timeRatio, pitchScale );

    // Copy sample buffer to a temporary buffer
    SharedSampleBuffer tempBuffer( new SampleBuffer( *sampleBuffer.data() ) );

    const int origNumFrames = tempBuffer->getNumFrames();
    const int newBufferSize = roundToInt( origNumFrames * timeRatio );

    const float** inFloatBuffer = new const float*[ numChans ];
    float** outFloatBuffer = new float*[ numChans ];

    int inFrameNum = 0;
    int totalNumFramesRetrieved = 0;

    stretcher.setExpectedInputDuration( origNumFrames );

    sampleBuffer->setSize( numChans, newBufferSize );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        inFloatBuffer[ chanNum ] = tempBuffer->getReadPointer( chanNum );
        outFloatBuffer[ chanNum ] = sampleBuffer->getWritePointer( chanNum );
    }

    stretcher.study( inFloatBuffer, origNumFrames, true );

    while ( inFrameNum < origNumFrames )
    {
        const int numRequired = stretcher.getSamplesRequired();

        const int numFramesToProcess = inFrameNum + numRequired <= origNumFrames ?
                                       numRequired : origNumFrames - inFrameNum;
        const bool isFinal = (inFrameNum + numRequired >= origNumFrames);

        stretcher.process( inFloatBuffer, numFramesToProcess, isFinal );

        const int numAvailable = stretcher.available();

        if ( numAvailable > 0 )
        {
            // Ensure enough space to store output
            if ( sampleBuffer->getNumFrames() < totalNumFramesRetrieved + numAvailable )
            {
                sampleBuffer->setSize( numChans,                                  // No. of channels
                                       totalNumFramesRetrieved + numAvailable,    // New no. of frames
                                       true );                                    // Keep existing content

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    outFloatBuffer[ chanNum ] = sampleBuffer->getWritePointer( chanNum );
                    outFloatBuffer[ chanNum ] += totalNumFramesRetrieved;
                }
            }

            const int numRetrieved = stretcher.retrieve( outFloatBuffer, numAvailable );

            for ( int chanNum = 0; chanNum < numChans; chanNum++ )
            {
                outFloatBuffer[ chanNum ] += numRetrieved;
            }
            totalNumFramesRetrieved += numRetrieved;
        }

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            inFloatBuffer[ chanNum ] += numFramesToProcess;
        }
        inFrameNum += numFramesToProcess;
    }

    int numAvailable;

    while ( (numAvailable = stretcher.available()) >= 0 )
    {
        if ( numAvailable > 0 )
        {
            // Ensure enough space to store output
            if ( sampleBuffer->getNumFrames() < totalNumFramesRetrieved + numAvailable )
            {
                sampleBuffer->setSize( numChans,                                  // No. of channels
                                       totalNumFramesRetrieved + numAvailable,    // New no. of frames
                                       true );                                    // Keep existing content

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    outFloatBuffer[ chanNum ] = sampleBuffer->getWritePointer( chanNum );
                    outFloatBuffer[ chanNum ] += totalNumFramesRetrieved;
                }
            }

            const int numRetrieved = stretcher.retrieve( outFloatBuffer, numAvailable );

            for ( int chanNum = 0; chanNum < numChans; chanNum++ )
            {
                outFloatBuffer[ chanNum ] += numRetrieved;
            }
            totalNumFramesRetrieved += numRetrieved;
        }
        else
        {
            usleep( 10000 );
        }
    }

    if ( sampleBuffer->getNumFrames() != totalNumFramesRetrieved )
    {
        sampleBuffer->setSize( numChans, totalNumFramesRetrieved, true );
    }

    delete[] inFloatBuffer;
    delete[] outFloatBuffer;

    return totalNumFramesRetrieved;
}
