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

#include "audiofilehandler.h"
#include "SndLibShuriken/_sndlib.h"
#include <aubio/aubio.h>
#include <sndfile.h>
#include <QDir>
//#include <QDebug>


//==================================================================================================
// Public:

AudioFileHandler::AudioFileHandler( QObject* parent ) :
    QObject( parent )
{
    // Initialise sndlib so we can read header info not available through aubio's API
    // and also open some audio file formats that may not be supported via aubio
    const int errorCode = initSndLib();

    if ( errorCode == MUS_ERROR )
    {
        sErrorTitle = tr("Error initialising sndlib!");
        sErrorInfo = tr("It may not be possible to read some audio files");
    }
}



SharedSampleBuffer AudioFileHandler::getSampleData( const QString filePath )
{
    Q_ASSERT_X( ! filePath.isEmpty(), "AudioFileHandler::getSampleData", "filePath is empty" );

    QByteArray charArray = filePath.toLocal8Bit();
    const char* path = charArray.data();

    SharedSampleBuffer sampleBuffer;

    // If file exists
    if ( mus_file_probe( path ) )
    {
        // First try using aubio to load the file; if that fails, try using sndlib
        sampleBuffer = aubioLoadFile( path );

        if ( sampleBuffer.isNull() )
        {
            sampleBuffer = sndlibLoadFile( path );
        }
    }

    return sampleBuffer;
}



SharedSampleHeader AudioFileHandler::getSampleHeader( const QString filePath )
{
    Q_ASSERT_X( ! filePath.isEmpty(), "AudioFileHandler::getSampleHeader", "filePath is empty" );

    QByteArray charArray = filePath.toLocal8Bit();
    const char* path = charArray.data();

    SharedSampleHeader sampleHeader;

    // If `0` is passed as `samplerate` to new_aubio_source, the sample rate of the original file is used.
    aubio_source_t* aubioSource = new_aubio_source( const_cast<char*>(path), 0, 4096 );

    if ( aubioSource != NULL )
    {
        sampleHeader = SharedSampleHeader( new SampleHeader );

        sampleHeader->sampleRate = aubio_source_get_samplerate( aubioSource );
        sampleHeader->numChans = aubio_source_get_channels( aubioSource );

        del_aubio_source( aubioSource );

        const int headerCode = mus_sound_header_type( path );

        // If sndlib recognises the audio file type
        if ( mus_header_type_p( headerCode ) )
        {
            sampleHeader->format = mus_header_type_name( headerCode );
            sampleHeader->bitsPerSample = mus_sound_bits_per_sample( path );
        }
        else
        {
            sampleHeader->bitsPerSample = 0;
        }
    }
    else
    {
        const int headerCode = mus_sound_header_type( path );

        // If sndlib recognises the audio file type
        if ( mus_header_type_p( headerCode ) )
        {
            sampleHeader = SharedSampleHeader( new SampleHeader );

            sampleHeader->format = mus_header_type_name( headerCode );
            sampleHeader->numChans = mus_sound_chans( path );
            sampleHeader->sampleRate = mus_sound_srate( path );
            sampleHeader->bitsPerSample = mus_sound_bits_per_sample( path );
        }
    }

    return sampleHeader;

//    QString dataFormatName = mus_data_format_name( mus_sound_data_format(path) );
}



bool AudioFileHandler::saveAudioFiles( const QString dirPath,
                                       const QList<SharedSampleBuffer> sampleBufferList,
                                       const SharedSampleHeader sampleHeader )
{
    const int hopSize = 8192;
    const int numChans = sampleHeader->numChans;

    const QDir dir( dirPath );
    int fileNum = 0;

    bool isSuccessful = true;

    SF_INFO sfInfo;
    memset( &sfInfo, 0, sizeof( sfInfo ) );

    sfInfo.samplerate = sampleHeader->sampleRate;
    sfInfo.channels   = numChans;
    sfInfo.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    Q_ASSERT_X( sf_format_check( &sfInfo ), "AudioFileHandler::saveAudioFiles", "sfInfo.format is not valid" );

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        QString fileNumStr;
        fileNumStr.setNum( fileNum );

        const QString filePath = dir.filePath( fileNumStr + ".wav" );

        SNDFILE* handle = sf_open( filePath.toUtf8().data(), SFM_WRITE, &sfInfo );

        if ( handle != NULL )
        {
            const int numFrames = sampleBuffer->getNumFrames();
            int numFramesToWrite = 0;
            int startFrame = 0;
            int numSamplesWritten = 0;

            Array<float> tempBuffer;
            tempBuffer.resize( hopSize * numChans );

            float* sampleData = NULL;

            do
            {
                numFramesToWrite = numFrames - startFrame >= hopSize ? hopSize : numFrames - startFrame;

                // Interleave sample data
                for ( int chanNum = 0; chanNum < numChans; ++chanNum )
                {
                    sampleData = sampleBuffer->getSampleData( chanNum, startFrame );

                    for ( int frameNum = 0; frameNum < numFramesToWrite; ++frameNum )
                    {
                        tempBuffer.set( numChans * frameNum + chanNum,  // Index
                                        sampleData[ frameNum ] );       // Value
                    }
                }

                // Write sample data to file
                numSamplesWritten = sf_write_float( handle, tempBuffer.getRawDataPointer(), numFramesToWrite * numChans );

                startFrame += hopSize;

                // If there was a write error
                if ( numSamplesWritten != numFramesToWrite * numChans)
                {
                    QString samplesToWriteStr;
                    samplesToWriteStr.setNum( numFramesToWrite * numChans );

                    QString samplesWrittenStr;
                    samplesWrittenStr.setNum( numSamplesWritten );

                    sErrorTitle = tr("Error while writing file") + fileNumStr + ".wav";
                    sErrorInfo = tr("no. of samples to write: ") + samplesToWriteStr +
                                 tr("no. of samples written: ") + samplesWrittenStr;

                    isSuccessful = false;
                }
            }
            while ( numFramesToWrite == hopSize && isSuccessful );

            sf_write_sync( handle );
            sf_close( handle );
        }
        else
        {
            sErrorTitle = tr("Couldn't open file for writing: ") + fileNumStr + ".wav";
            sErrorInfo = sf_strerror( NULL );
            isSuccessful = false;
        }

        fileNum++;

        // If a write error has occurred break out of the loop
        if ( ! isSuccessful )
            break;
    }

    return isSuccessful;
}



//==================================================================================================
// Private Static:

QString AudioFileHandler::sErrorTitle;
QString AudioFileHandler::sErrorInfo;


int AudioFileHandler::initSndLib()
{
    if ( mus_sound_initialize() == MUS_ERROR )
    {
        return MUS_ERROR;
    }

    mus_error_set_handler( recordSndLibError );

    return MUS_NO_ERROR;
}



void AudioFileHandler::recordSndLibError( int errorCode, char* errorMessage )
{
    sErrorTitle = mus_error_type_to_string( errorCode );
    sErrorInfo = errorMessage;
}



SharedSampleBuffer AudioFileHandler::sndlibLoadFile( const char* filePath )
{
    const int leftChan = 0;
    const int rightChan = 1;

    int fileID = 0;
    int numChans = 0;
    mus_long_t numFrames = 0;
    mus_float_t** floatBuffers = NULL;
    SharedSampleBuffer sampleBuffer;


    if ( ! mus_header_type_p( mus_sound_header_type(filePath) ) )
    {
        goto end;
    }

    if ( ! mus_data_format_p( mus_sound_data_format(filePath) ) )
    {
        goto end;
    }

    numChans = mus_sound_chans( filePath );
    if ( numChans == MUS_ERROR )
    {
        goto end;
    }
    if ( numChans < 1 )
    {
        mus_error( MUS_NO_CHANNEL, "File has no audio channels!" );
        goto end;
    }
    if ( numChans > 2 )
    {
        mus_error( MUS_UNSUPPORTED_DATA_FORMAT, "Only mono and stereo samples are supported" );
        goto end;
    }

    if ( mus_sound_srate(filePath) == MUS_ERROR )
    {
        goto end;
    }

    numFrames = mus_sound_frames( filePath );
    if ( numFrames == MUS_ERROR )
    {
        goto end;
    }

    floatBuffers = (mus_float_t**) calloc( numChans, sizeof(mus_float_t*) );
    if ( floatBuffers == NULL )
    {
        mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
        goto end;
    }
    for ( int i = 0; i < numChans; i++)
    {
        floatBuffers[i] = (mus_float_t*) calloc( numFrames, sizeof(mus_float_t) );
        if ( floatBuffers[i] == NULL )
        {
            // clean up and bail out!
            for ( int n = 0; n < i; n++ )
                free( floatBuffers[n] );
            free( floatBuffers );
            mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
            goto end;
        }
    }

    try
    {
        sampleBuffer = SharedSampleBuffer( new SampleBuffer( numChans, numFrames ) );

        fileID = mus_sound_open_input( filePath );
        if ( fileID == MUS_ERROR )
        {
            // clean up and bail out!
            for ( int i = 0; i < numChans; i++ )
                free( floatBuffers[i] );
            free( floatBuffers );
            goto end;
        }

        if ( mus_file_read( fileID, 0, numFrames - 1, numChans, floatBuffers ) == MUS_ERROR )
        {
            // clean up and bail out!
            mus_sound_close_input( fileID );
            for ( int i = 0; i < numChans; i++ )
                free( floatBuffers[i] );
            free( floatBuffers );
            goto end;
        }
        if ( numChans == 1 )
        {
            sampleBuffer->copyFrom( leftChan, 0, floatBuffers[leftChan], numFrames );
        }
        else
        {
            sampleBuffer->copyFrom( leftChan, 0, floatBuffers[leftChan], numFrames );
            sampleBuffer->copyFrom( rightChan, 0, floatBuffers[rightChan], numFrames );
        }

        mus_sound_close_input( fileID );
    }
    catch ( std::bad_alloc& )
    {
        mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
    }

    // Clean up
    for ( int i = 0; i < numChans; i++ )
        free( floatBuffers[i] );
    free( floatBuffers );

    end:
    return sampleBuffer;
}



SharedSampleBuffer AudioFileHandler::aubioLoadFile( const char* filePath )
{
    uint_t sampleRate = 0; // If `0` is passed as `samplerate` to new_aubio_source, the sample rate of the original file is used.
    uint_t hopSize = 4096;
    uint_t numFrames = 0;
    uint_t numFramesRead = 0;
    uint_t numChans = 0;

    int startFrame = 0;
    int numFramesToCopy = hopSize;

    aubio_source_t* aubioSource = new_aubio_source( const_cast<char*>(filePath), sampleRate, hopSize );
    fmat_t* sampleData = NULL;

    SharedSampleBuffer sampleBuffer;

    if ( aubioSource != NULL )
    {
        sampleRate = aubio_source_get_samplerate( aubioSource );
        numChans = aubio_source_get_channels( aubioSource );

        if ( numChans > 2 )
        {
            mus_error( MUS_UNSUPPORTED_DATA_FORMAT, "Only mono and stereo samples are supported" );
        }
        else
        {
            sampleData = new_fmat( numChans, hopSize );

            if ( sampleData != NULL )
            {
                numFrames = mus_sound_frames( filePath );
                if ( numFrames < 1 )
                {
                    numFrames = 0;
                    do
                    {
                        aubio_source_do_multi( aubioSource, sampleData, &numFramesRead );
                        numFrames += numFramesRead;
                    }
                    while ( numFramesRead == hopSize );

                    aubio_source_seek( aubioSource, 0 );
                    numFramesRead = 0;
                    fmat_zeros( sampleData );
                }

                try
                {
                    sampleBuffer = SharedSampleBuffer( new SampleBuffer( numChans, numFrames ) );

                    do
                    {
                        aubio_source_do_multi( aubioSource, sampleData, &numFramesRead );

                        numFramesToCopy = numFrames - startFrame >= hopSize ? hopSize : numFrames - startFrame;

                        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                        {
                            sampleBuffer->copyFrom( chanNum, startFrame, sampleData->data[ chanNum ], numFramesToCopy );
                        }
                        startFrame += numFramesRead;
                    }
                    while ( numFramesRead == hopSize );
                }
                catch ( std::bad_alloc& )
                {
                    mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
                }

                del_fmat( sampleData );
            }
        }
        del_aubio_source( aubioSource );
    }

    return sampleBuffer;
}
