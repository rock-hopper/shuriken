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


class AudioFileHandler
{
public:
    AudioFileHandler();

    SharedSampleBuffer getSampleData( const QString filePath );
    SharedSampleBuffer getSampleData( const QString filePath, const int startFrame, const int numFramesToRead );
    SharedSampleHeader getSampleHeader( const QString filePath );

    QString saveAudioFile( const QString dirPath,
                        const QString fileBaseName,
                        const SharedSampleBuffer sampleBuffer,
                        const SharedSampleHeader sampleHeader,
                        const bool isTempFile = false );

    QString getLastErrorTitle() const   { return sErrorTitle; }
    QString getLastErrorInfo() const    { return sErrorInfo; }

private:
    static QString sErrorTitle;
    static QString sErrorInfo;

    static int initSndLib();
    static void recordSndLibError( int errorCode, char* errorMessage );

    static SharedSampleBuffer sndlibLoadFile( const char* filePath, mus_long_t startFrame, mus_long_t numFramesToRead );
    static SharedSampleBuffer aubioLoadFile( const char* filePath, uint_t startFrame, uint_t numFramesToRead );

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioFileHandler );
};

#endif // AUDIOFILEHANDLER_H
