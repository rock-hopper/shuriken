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

#include "audioanalyser.h"


//==================================================================================================
// Public Static:

QList<int> AudioAnalyser::findOnsetFrameNums( const SharedSampleBuffer sampleBuffer, const DetectionSettings settings )
{
    char_t* detectionMethod = (char_t*) settings.detectionMethod.data();
    smpl_t threshold = settings.threshold;
    uint_t windowSize = settings.windowSize;
    uint_t hopSize = settings.hopSize;
    uint_t sampleRate = settings.sampleRate;
    fvec_t* detectionResultVector;
    fvec_t* inputBuffer;

    QList<int> slicePointFrameNumList;

    const int numFrames = sampleBuffer->getNumFrames();

    const int vectorSize = 1;
    const int onsetData = 0;
    int slicePointFrameNum = 0;

    // Create onset detector and detection result vector
    aubio_onset_t* onsetDetector = new_aubio_onset( detectionMethod, windowSize, hopSize, sampleRate );
    aubio_onset_set_threshold( onsetDetector, threshold );
    aubio_onset_set_minioi_s( onsetDetector, MIN_INTER_ONSET_SECS );
    detectionResultVector = new_fvec( vectorSize );

    inputBuffer = new_fvec( hopSize );

    // Do onset detection
    for ( int frameNum = 0; frameNum < numFrames; frameNum += hopSize )
    {
        fillAubioInputBuffer( inputBuffer, sampleBuffer, frameNum );

        aubio_onset_do( onsetDetector, inputBuffer, detectionResultVector );

        // If an onset is detected add a new slice point to the list
        if ( detectionResultVector->data[ onsetData ] )
        {
            slicePointFrameNum = aubio_onset_get_last( onsetDetector );
            slicePointFrameNumList.append( slicePointFrameNum );
        }
    }

    // Delete onset detector
    del_aubio_onset( onsetDetector );

    // Clean up memory
    del_fvec( detectionResultVector );
    del_fvec( inputBuffer );
    aubio_cleanup();

    return slicePointFrameNumList;
}



QList<int> AudioAnalyser::findBeatFrameNums( const SharedSampleBuffer sampleBuffer, const DetectionSettings settings )
{
    char_t* detectionMethod = (char_t*) settings.detectionMethod.data();
    smpl_t threshold = settings.threshold;
    uint_t windowSize = settings.windowSize;
    uint_t hopSize = settings.hopSize;
    uint_t sampleRate = settings.sampleRate;
    fvec_t* detectionResultVector;
    fvec_t* inputBuffer;

    QList<int> slicePointFrameNumList;

    const int numFrames = sampleBuffer->getNumFrames();

    const int vectorSize = 2;
    const int beatData = 0;
//    const int onsetData = 1;
    int slicePointFrameNum = 0;

    // Create beat detector and detection result vector
    aubio_tempo_t* beatDetector = new_aubio_tempo( detectionMethod, windowSize, hopSize, sampleRate );
    aubio_tempo_set_threshold( beatDetector, threshold );
    detectionResultVector = new_fvec( vectorSize );

    inputBuffer = new_fvec( hopSize );

    // Do beat detection
    for ( int frameNum = 0; frameNum < numFrames; frameNum += hopSize )
    {
        fillAubioInputBuffer( inputBuffer, sampleBuffer, frameNum );

        aubio_tempo_do( beatDetector, inputBuffer, detectionResultVector );

        // If a beat of the bar (tactus) is detected add a new slice point to the list
        if ( detectionResultVector->data[ beatData ] )
        {
            slicePointFrameNum = aubio_tempo_get_last( beatDetector );
            slicePointFrameNumList.append( slicePointFrameNum );
        }
    }

    // Delete beat detector
    del_aubio_tempo( beatDetector );

    // Clean up memory
    del_fvec( detectionResultVector );
    del_fvec( inputBuffer );
    aubio_cleanup();

    return slicePointFrameNumList;
}



qreal AudioAnalyser::calcBPM( const SharedSampleBuffer sampleBuffer, const DetectionSettings settings )
{
    char_t* detectionMethod = (char_t*) settings.detectionMethod.data();
    smpl_t threshold = settings.threshold;
    uint_t windowSize = settings.windowSize;
    uint_t hopSize = settings.hopSize;
    uint_t sampleRate = settings.sampleRate;

    fvec_t* detectionResultVector;
    fvec_t* inputBuffer;

    const int numFrames = sampleBuffer->getNumFrames();

    const int vectorSize = 2;
    const int beatData = 0;
//    const int onsetData = 1;

    const int confidenceThreshold = 0.2;

    int numDetections = 0;
    qreal currentBPM = 0.0;
    qreal summedBPMs = 0.0;
    qreal averageBPM = 0.0;
    qreal confidence = 0.0;

    // Create beat detector and detection result vector
    aubio_tempo_t* beatDetector = new_aubio_tempo( detectionMethod, windowSize, hopSize, sampleRate );
    aubio_tempo_set_threshold( beatDetector, threshold );
    detectionResultVector = new_fvec( vectorSize );

    inputBuffer = new_fvec( hopSize );

    // Do bpm detection
    for ( int frameNum = 0; frameNum < numFrames; frameNum += hopSize )
    {
        fillAubioInputBuffer( inputBuffer, sampleBuffer, frameNum );

        aubio_tempo_do( beatDetector, inputBuffer, detectionResultVector );

        // If a beat of the bar (tactus) is detected get the current BPM
        if ( detectionResultVector->data[ beatData ] )
        {
            currentBPM = aubio_tempo_get_bpm( beatDetector );
            confidence = aubio_tempo_get_confidence( beatDetector );

            if ( currentBPM > 0.0 && confidence > confidenceThreshold )
            {
                summedBPMs += currentBPM;
                numDetections++;
            }
        }
    }

    // Delete beat detector
    del_aubio_tempo( beatDetector );

    // Clean up memory
    del_fvec( detectionResultVector );
    del_fvec( inputBuffer );
    aubio_cleanup();

    if ( numDetections > 0 )
    {
         averageBPM = summedBPMs / numDetections;
    }

    return floor( averageBPM );
}



//==================================================================================================
// Private Static:

void AudioAnalyser::fillAubioInputBuffer( fvec_t* inputBuffer, const SharedSampleBuffer sampleBuffer, const int sampleOffset )
{
    const int numFrames = sampleBuffer->getNumFrames();
    const int numChans = sampleBuffer->getNumChannels();
    const float multiplier = 1.0 / numChans;
    const int hopSize = inputBuffer->length;

    FloatVectorOperations::clear( inputBuffer->data, hopSize );

    // Fill up the input buffer, converting stereo to mono if necessary
    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        const float* sampleData = sampleBuffer->getReadPointer( chanNum, sampleOffset );

        const int numFramesToAdd = ( sampleOffset + hopSize <= numFrames ? hopSize : numFrames - sampleOffset );

        FloatVectorOperations::addWithMultiply( inputBuffer->data, sampleData, multiplier, numFramesToAdd );
    }
}
