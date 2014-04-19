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

#include <QObject>
#include "JuceHeader.h"
#include "samplebuffer.h"


class AudioFileHandler : public QObject
{
    Q_OBJECT

public:
    explicit AudioFileHandler( QObject* parent = NULL );

    SharedSampleBuffer getSampleData( const QString filePath );
    SharedSampleHeader getSampleHeader( const QString filePath );

    bool saveAudioFiles( const QString dirPath,
                         const QList<SharedSampleBuffer> sampleBufferList,
                         const SharedSampleHeader sampleHeader );

    QString getLastErrorTitle() const   { return sErrorTitle; }
    QString getLastErrorInfo() const    { return sErrorInfo; }

private:
    static QString sErrorTitle;
    static QString sErrorInfo;

    static int initSndLib();
    static void recordSndLibError( int errorCode, char* errorMessage );

    static SharedSampleBuffer sndlibLoadFile( const char* filePath );
    static SharedSampleBuffer aubioLoadFile( const char* filePath );
};

#endif // AUDIOFILEHANDLER_H
