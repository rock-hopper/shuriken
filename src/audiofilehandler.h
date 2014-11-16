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

#ifndef AUDIOFILEHANDLER_H
#define AUDIOFILEHANDLER_H

#include "JuceHeader.h"
#include "samplebuffer.h"
#include "SndLibShuriken/_sndlib.h"
#include <aubio/aubio.h>
#include <sndfile.h>


class AudioFileHandler
{
public:
    AudioFileHandler();

    SharedSampleBuffer getSampleData( QString filePath );
    SharedSampleBuffer getSampleData( QString filePath, int startFrame, int numFramesToRead );
    SharedSampleHeader getSampleHeader( QString filePath );

    // Returns absolute file path of saved audio file on success, otherwise returns an empty string
    QString saveAudioFile( QString dirPath,
                           QString fileBaseName,
                           SharedSampleBuffer sampleBuffer,
                           int currentSampleRateRate,
                           int outputSampleRate,
                           int sndFileFormat,
                           bool isOverwriteEnabled = true );

    QString getLastErrorTitle() const   { return s_errorTitle; }
    QString getLastErrorInfo() const    { return s_errorInfo; }

public:
    static const int SAVE_FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    static const int TEMP_FORMAT = SF_ENDIAN_CPU | SF_FORMAT_AU | SF_FORMAT_FLOAT;

private:
    static void interleaveSamples( SharedSampleBuffer inputBuffer,
                                   int numChans,
                                   int inputStartFrame,
                                   int numFrames,
                                   Array<float>& outputBuffer );

    static void deinterleaveSamples( Array<float>& inputBuffer,
                                     int numChans,
                                     int outputStartFrame,
                                     int numFrames,
                                     SharedSampleBuffer outputBuffer );

    // Takes a buffer of non-interleaved samples, converts sample rate,
    // and outputs a buffer of interleaved samples
    static bool convertSampleRate( SharedSampleBuffer inputBuffer,
                                   qreal sampleRateRatio,
                                   Array<float>& outputBuffer );

    static bool sndfileSaveAudioFile( SNDFILE* fileID, SharedSampleBuffer sampleBuffer, int hopSize );
    static bool sndfileSaveAudioFile( SNDFILE* fileID, Array<float> interleavedBuffer, int hopSize );
    static void sndfileRecordWriteError( int numSamplesToWrite, int numSamplesWritten );
    static SharedSampleBuffer sndfileLoadFile( const char* filePath, sf_count_t startFrame, sf_count_t numFramesToRead );

    static int sndlibInit();
    static void sndlibRecordError( int errorCode, char* errorMessage );
    static SharedSampleBuffer sndlibLoadFile( const char* filePath, mus_long_t startFrame, mus_long_t numFramesToRead );

    static SharedSampleBuffer aubioLoadFile( const char* filePath, uint_t startFrame, uint_t numFramesToRead );

    static QString s_errorTitle;
    static QString s_errorInfo;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioFileHandler );
};

#endif // AUDIOFILEHANDLER_H
