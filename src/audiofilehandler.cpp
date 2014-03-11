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


//==================================================================================================
// Public:

AudioFileHandler::AudioFileHandler( QObject* parent ) :
    QObject( parent )
{
    // Initialise JUCE AudioFormatManager so we can load OGG and FLAC files
    mFormatManager.registerBasicFormats();

    // Initialise sndlib so we can load many other audio file formats
    const int errorCode = initSndLib();

    if ( errorCode == MUS_ERROR )
    {
        sErrorTitle = tr("Error initialising sndlib!");
        sErrorInfo = tr("It will not be possible to open many audio file formats");
    }
}



SharedSampleBuffer AudioFileHandler::getSampleData( const QString filePath )
{
    Q_ASSERT_X( ! filePath.isEmpty(), "AudioFileHandler::getSampleData", "filePath is empty" );

    QByteArray charArray = filePath.toLocal8Bit();
    const char* file_path = charArray.data();
    SharedSampleBuffer sampleBuffer;

    // If file exists
    if ( mus_file_probe( file_path ) )
    {
        const int header_code = mus_sound_header_type( file_path );

        // If sndlib recognises the audio file type
        if ( mus_header_type_p( header_code ) )
        {
            if ( header_code == MUS_OGG || header_code == MUS_FLAC )
            {
                sampleBuffer = juceLoadFile( file_path );
            }
            else
            {
                sampleBuffer = sndlibLoadFile( file_path );
            }
        }
    }

    return sampleBuffer;
}



SharedSampleHeader AudioFileHandler::getSampleHeader( const QString filePath )
{
    Q_ASSERT_X( ! filePath.isEmpty(), "AudioFileHandler::getSampleHeader", "filePath is empty" );

    QByteArray charArray = filePath.toLocal8Bit();
    const char* file_path = charArray.data();
    SharedSampleHeader sampleHeader;

    // If file exists
    if ( mus_file_probe( file_path ) )
    {
        const int header_code = mus_sound_header_type( file_path );

        // If sndlib recognises the audio file type
        if ( mus_header_type_p( header_code ) )
        {
            sampleHeader = SharedSampleHeader( new SampleHeader );
            sampleHeader->fileName = filePath;
            sampleHeader->fileType = mus_header_type_name( mus_sound_header_type(file_path) );
            sampleHeader->sampleRate = mus_sound_srate( file_path );
        }
    }

    return sampleHeader;

//    QString headerTypeName = mus_header_type_name( mus_sound_header_type(file_path) );
//    QString dataFormatName = mus_data_format_name( mus_sound_data_format(file_path) );
}



//==================================================================================================
// Private:

SharedSampleBuffer AudioFileHandler::juceLoadFile( const char* filePath )
{
    const File audioFile( filePath );
    SharedSampleBuffer sampleBuffer;

    // Create a file reader that can read this type of audio file
    ScopedPointer<AudioFormatReader> reader( mFormatManager.createReaderFor( audioFile ) );

    if ( reader != NULL )
    {
        const int numChans = reader->numChannels;

        if ( numChans >= 1 && numChans <= 2 )
        {
            const int numFrames = reader->lengthInSamples / numChans;

            try
            {
                sampleBuffer = SharedSampleBuffer( new SampleBuffer( numChans, numFrames ) );
                reader->read( sampleBuffer.data(), 0, numFrames, 0, true, true );
            }
            catch ( std::bad_alloc& )
            {
                mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
            }
        }
        else
        {
            mus_error( MUS_UNSUPPORTED_DATA_FORMAT, "Only mono and stereo samples are supported" );
        }
    }
    else
    {
        mus_error( MUS_CANT_OPEN_FILE, "Perhaps audio file is corrupted?" );
    }

    return sampleBuffer;
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

    mus_error_set_handler( sndlibErrorHandler );

    return MUS_NO_ERROR;
}



void AudioFileHandler::sndlibErrorHandler( int errorCode, char* errorMessage )
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

    if ( ! (mus_data_format_p(mus_sound_data_format(filePath))) )
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
        for ( int i = 0; i < numChans; i++ )
            free( floatBuffers[i] );
        free( floatBuffers );
    }
    catch ( std::bad_alloc& )
    {
        mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
    }

    end:
    return sampleBuffer;
}
