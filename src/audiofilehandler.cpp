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
#include <samplerate.h>
#include <QDir>
#include <QDebug>


//==================================================================================================
// Public:

AudioFileHandler::AudioFileHandler()
{
    // Initialise sndlib so we can read header info not available through aubio's API
    // and also open some audio file formats that may not be supported via aubio
    const int errorCode = sndlibInit();

    if ( errorCode == MUS_ERROR )
    {
        s_errorTitle = "Error initialising sndlib!";
        s_errorInfo = "It may not be possible to read some audio files";
    }
}


SharedSampleBuffer AudioFileHandler::getSampleData( const QString filePath )
{
    return getSampleData( filePath, 0, 0 );
}



SharedSampleBuffer AudioFileHandler::getSampleData( const QString filePath, const int startFrame, const int numFramesToRead )
{
    Q_ASSERT( ! filePath.isEmpty() );

    QByteArray charArray = filePath.toLocal8Bit();
    const char* path = charArray.data();

    SharedSampleBuffer sampleBuffer;

#ifdef ENABLE_AUBIO_FILE_IO
    // First try using aubio to load the file; if that fails, try using sndlib
    sampleBuffer = aubioLoadFile( path, startFrame, numFramesToRead );
#else
    // First try using sndlib to load the file; if that fails, try using sndlib
    sampleBuffer = sndfileLoadFile( path, startFrame, numFramesToRead );
#endif

    if ( sampleBuffer.isNull() )
    {
        sampleBuffer = sndlibLoadFile( path, startFrame, numFramesToRead );
    }

    return sampleBuffer;
}



SharedSampleHeader AudioFileHandler::getSampleHeader( const QString filePath )
{
    Q_ASSERT( ! filePath.isEmpty() );

    QByteArray charArray = filePath.toLocal8Bit();
    const char* path = charArray.data();

    SharedSampleHeader sampleHeader;

    // If `0` is passed as `samplerate` to new_aubio_source, the sample rate of the original file is used.
    aubio_source_t* aubioSource = new_aubio_source( const_cast<char*>(path), 0, 4096 );

    if ( aubioSource != NULL ) // First try using aubio to read the header
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
    else // If aubio can't read the header, try using sndlib
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

    // It's essential that the sample rate is known
    if ( ! sampleHeader.isNull() && sampleHeader->sampleRate < 1.0 )
    {
        sampleHeader.clear();
    }

    return sampleHeader;
}



QString AudioFileHandler::saveAudioFile( const QString dirPath,
                                         const QString fileBaseName,
                                         const SharedSampleBuffer sampleBuffer,
                                         const int currentSampleRate,
                                         const int outputSampleRate,
                                         const int sndFileFormat,
                                         const bool isOverwriteEnabled )
{
    Q_ASSERT( currentSampleRate != 0 );

    const int hopSize = 8192;
    const int numChans = sampleBuffer->getNumChannels();

    bool isSuccessful = false;

    QDir saveDir( dirPath );
    QString filePath;

    if ( saveDir.exists() )
    {
        filePath = saveDir.absoluteFilePath( fileBaseName );

        SF_INFO sfInfo;
        memset( &sfInfo, 0, sizeof( SF_INFO ) );

        sfInfo.samplerate = outputSampleRate;
        sfInfo.channels   = numChans;
        sfInfo.format     = sndFileFormat;

        switch ( sndFileFormat & SF_FORMAT_TYPEMASK )
        {
        case SF_FORMAT_WAV:
            filePath.append( ".wav" );
            break;
        case SF_FORMAT_AIFF:
            filePath.append( ".aiff" );
            break;
        case SF_FORMAT_AU:
            filePath.append( ".au" );
            break;
        case SF_FORMAT_FLAC:
            filePath.append( ".flac" );
            break;
        case SF_FORMAT_OGG:
            filePath.append( ".ogg" );
            break;
        default:
            qDebug() << "Unknown format: " << sndFileFormat;
            break;
        }

        Q_ASSERT( sf_format_check( &sfInfo ) );

        if ( isOverwriteEnabled || ! QFileInfo( filePath ).exists() )
        {
            SNDFILE* fileID = sf_open( filePath.toLocal8Bit().data(), SFM_WRITE, &sfInfo );

            if ( fileID != NULL )
            {
                if ( outputSampleRate == currentSampleRate )
                {
                    isSuccessful = sndfileSaveAudioFile( fileID, sampleBuffer, hopSize );
                }
                else
                {
                    const qreal sampleRateRatio = (qreal) outputSampleRate / (qreal) currentSampleRate;

                    Array<float> interleavedBuffer;

                    isSuccessful = convertSampleRate( sampleBuffer, sampleRateRatio, interleavedBuffer );

                    if ( isSuccessful )
                    {
                        isSuccessful = sndfileSaveAudioFile( fileID, interleavedBuffer, hopSize * numChans );
                    }
                }
                sf_write_sync( fileID );
                sf_close( fileID );
            }
            else // Could not open file for writing
            {
                s_errorTitle = "Couldn't open file for writing";
                s_errorInfo = sf_strerror( NULL );
                isSuccessful = false;
            }
        }
        else // File already exists and overwriting is not enabled
        {
            s_errorTitle = "Couldn't overwrite existing file";
            s_errorInfo = "The file " + filePath + " already exists and could not be overwritten";
            isSuccessful = false;
        }
    }

    if ( ! isSuccessful )
    {
        filePath.clear();
    }

    return filePath;
}



//==================================================================================================
// Private Static:

QString AudioFileHandler::s_errorTitle;
QString AudioFileHandler::s_errorInfo;


void AudioFileHandler::interleaveSamples( const SharedSampleBuffer inputBuffer,
                                          const int numChans,
                                          const int inputStartFrame,
                                          const int numFrames,
                                          Array<float>& outputBuffer )
{
    for ( int chanNum = 0; chanNum < numChans; ++chanNum )
    {
        const float* sampleData = inputBuffer->getReadPointer( chanNum, inputStartFrame );

        for ( int frameNum = 0; frameNum < numFrames; ++frameNum )
        {
            outputBuffer.set( numChans * frameNum + chanNum,    // Index
                              sampleData[ frameNum ] );         // Value
        }
    }
}



void AudioFileHandler::deinterleaveSamples( Array<float>& inputBuffer,
                                            const int numChans,
                                            const int outputStartFrame,
                                            const int numFrames,
                                            SharedSampleBuffer outputBuffer )
{
    const float* inputSampleData = inputBuffer.getRawDataPointer();

    for ( int chanNum = 0; chanNum < numChans; ++chanNum )
    {
        float* outputSampleData = outputBuffer->getWritePointer( chanNum, outputStartFrame );

        for ( int frameNum = 0; frameNum < numFrames; ++frameNum )
        {
            outputSampleData[ frameNum ] = inputSampleData[ numChans * frameNum + chanNum ];
        }
    }
}



bool AudioFileHandler::convertSampleRate( const SharedSampleBuffer inputBuffer,
                                          const qreal sampleRateRatio,
                                          Array<float>& outputBuffer )
{
    const int inputNumFrames = inputBuffer->getNumFrames();
    const int numChans = inputBuffer->getNumChannels();

    const long outputNumFrames = roundToInt( inputNumFrames * sampleRateRatio );

    if ( outputBuffer.size() != outputNumFrames * numChans )
    {
        outputBuffer.resize( outputNumFrames * numChans );
    }

    Array<float> tempBuffer;
    tempBuffer.resize( inputNumFrames * numChans );

    interleaveSamples( inputBuffer, numChans, 0, inputNumFrames, tempBuffer );

    SRC_DATA srcData;
    memset( &srcData, 0, sizeof( SRC_DATA ) );

    srcData.data_in = tempBuffer.getRawDataPointer();
    srcData.data_out = outputBuffer.getRawDataPointer();

    srcData.input_frames = inputNumFrames;
    srcData.output_frames = outputNumFrames;

    srcData.src_ratio = sampleRateRatio;

    bool isSuccessful = true;

    const int errorCode = src_simple( &srcData, SRC_SINC_BEST_QUALITY, numChans );

    if ( errorCode > 0 )
    {
        s_errorTitle = "Couldn't convert sample rate!";
        s_errorInfo = src_strerror( errorCode );
        isSuccessful = false;
    }

    return isSuccessful;
}



bool AudioFileHandler::sndfileSaveAudioFile( SNDFILE* fileID, const SharedSampleBuffer sampleBuffer, const int hopSize )
{
    const int totalNumFrames = sampleBuffer->getNumFrames();
    const int numChans = sampleBuffer->getNumChannels();

    int numFramesToWrite = 0;
    int startFrame = 0;
    int numSamplesWritten = 0;

    Array<float> tempBuffer;
    tempBuffer.resize( hopSize * numChans );

    bool isSuccessful = true;

    do
    {
        numFramesToWrite = totalNumFrames - startFrame >= hopSize ? hopSize : totalNumFrames - startFrame;

        interleaveSamples( sampleBuffer, numChans, startFrame, numFramesToWrite, tempBuffer );

        numSamplesWritten = sf_write_float( fileID, tempBuffer.getRawDataPointer(), numFramesToWrite * numChans );

        if ( numSamplesWritten != numFramesToWrite * numChans )
        {
            sndfileRecordWriteError( numFramesToWrite * numChans, numSamplesWritten );
            isSuccessful = false;
        }

        startFrame += hopSize;
    }
    while ( numFramesToWrite == hopSize && isSuccessful );

    return isSuccessful;
}



bool AudioFileHandler::sndfileSaveAudioFile( SNDFILE* fileID, const Array<float> interleavedBuffer, const int hopSize )
{
    const int totalNumSamples = interleavedBuffer.size();

    int numSamplesToWrite = 0;
    int startSample = 0;
    int numSamplesWritten = 0;

    Array<float> tempBuffer;
    tempBuffer.resize( hopSize );

    bool isSuccessful = true;

    do
    {
        numSamplesToWrite = totalNumSamples - startSample >= hopSize ? hopSize : totalNumSamples - startSample;

        for ( int i = 0; i < numSamplesToWrite; i++ )
        {
            tempBuffer.setUnchecked( i, interleavedBuffer.getUnchecked( startSample + i ) );
        }

        numSamplesWritten = sf_write_float( fileID, tempBuffer.getRawDataPointer(), numSamplesToWrite );

        if ( numSamplesWritten != numSamplesToWrite )
        {
            sndfileRecordWriteError( numSamplesToWrite, numSamplesWritten );
            isSuccessful = false;
        }

        startSample += hopSize;
    }
    while ( numSamplesToWrite == hopSize && isSuccessful );

    return isSuccessful;
}



void AudioFileHandler::sndfileRecordWriteError( const int numSamplesToWrite, const int numSamplesWritten )
{
    const QString samplesToWrite = QString::number( numSamplesToWrite );
    const QString samplesWritten = QString::number( numSamplesWritten );

    s_errorTitle = "Error while writing to audio file";
    s_errorInfo = "no. of samples to write: " + samplesToWrite + ", " +
                 "no. of samples written: " + samplesWritten;
}



SharedSampleBuffer AudioFileHandler::sndfileLoadFile( const char* filePath, sf_count_t startFrame, sf_count_t numFramesToRead )
{
    const sf_count_t hopSize = 4096;

    SharedSampleBuffer sampleBuffer;
    Array<float> tempBuffer;

    SF_INFO sfInfo;
    memset( &sfInfo, 0, sizeof( SF_INFO ) );

    SNDFILE* fileID = sf_open( filePath, SFM_READ, &sfInfo );

    if ( fileID == NULL )
    {
        s_errorTitle = "Couldn't open file for reading!";
        s_errorInfo = sf_strerror( NULL );
        goto end;
    }

    if ( sfInfo.channels < 1 )
    {
        mus_error( MUS_NO_CHANNEL, "File has no audio channels!" );
        goto end;
    }
    if ( sfInfo.channels > 2 )
    {
        mus_error( MUS_UNSUPPORTED_DATA_FORMAT, "Only mono and stereo samples are supported" );
        goto end;
    }

    tempBuffer.resize( hopSize * sfInfo.channels );

    // If caller has not set `numFramesToRead` assume whole file should be read

    if ( numFramesToRead < 1 ) // Read whole file
    {
        startFrame = 0;
        numFramesToRead = 0;
        sf_count_t numFramesRead = 0;

        // Find the no. of frames the long way
        do
        {
            numFramesRead = sf_readf_float( fileID, tempBuffer.getRawDataPointer(), hopSize );
            numFramesToRead += numFramesRead;
        }
        while ( numFramesRead > 0 );

        sf_seek( fileID, 0, SEEK_SET );
    }
    else // Read part of file
    {
        sf_seek( fileID, startFrame, SEEK_SET );
    }

    try
    {
        sampleBuffer = SharedSampleBuffer( new SampleBuffer( sfInfo.channels, numFramesToRead ) );
        sf_count_t numFramesRead = 0;
        sf_count_t totalNumFramesRead = 0;

        do
        {
            numFramesRead = sf_readf_float( fileID, tempBuffer.getRawDataPointer(), hopSize );

            deinterleaveSamples( tempBuffer, sfInfo.channels, totalNumFramesRead, numFramesRead, sampleBuffer );

            totalNumFramesRead += numFramesRead;
        }
        while ( numFramesRead > 0 && totalNumFramesRead < numFramesToRead );
    }
    catch ( std::bad_alloc& )
    {
        mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
    }

    sf_close( fileID );

    end:
    return sampleBuffer;
}



int AudioFileHandler::sndlibInit()
{
    if ( mus_sound_initialize() == MUS_ERROR )
    {
        return MUS_ERROR;
    }

    mus_error_set_handler( sndlibRecordError );

    return MUS_NO_ERROR;
}



void AudioFileHandler::sndlibRecordError( int errorCode, char* errorMessage )
{
    s_errorTitle = mus_error_type_to_string( errorCode );
    s_errorInfo = errorMessage;
}



SharedSampleBuffer AudioFileHandler::sndlibLoadFile( const char* filePath, mus_long_t startFrame, mus_long_t numFramesToRead )
{
    int fileID = 0;
    int numChans = 0;
    mus_long_t numFramesRead = 0;
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

    // If caller has not set `numFramesToRead` assume whole file should be read
    if ( numFramesToRead < 1 )
    {
        startFrame = 0;
        numFramesToRead = mus_sound_frames( filePath );

        if ( numFramesToRead == MUS_ERROR )
        {
            goto end;
        }
    }

    try
    {
        sampleBuffer = SharedSampleBuffer( new SampleBuffer( numChans, numFramesToRead ) );

        fileID = mus_sound_open_input( filePath );
        if ( fileID == MUS_ERROR )
        {
            goto end;
        }

        if ( mus_file_seek_frame( fileID, startFrame ) == MUS_ERROR )
        {
            mus_sound_close_input( fileID );
            goto end;
        }

        numFramesRead = mus_file_read( fileID,
                                       0,
                                       numFramesToRead - 1,
                                       numChans,
                                       sampleBuffer->getArrayOfWritePointers() );

        if ( numFramesRead == MUS_ERROR )
        {
            mus_sound_close_input( fileID );
            sampleBuffer.clear();
            goto end;
        }
        mus_sound_close_input( fileID );
    }
    catch ( std::bad_alloc& )
    {
        mus_error( MUS_MEMORY_ALLOCATION_FAILED, "Not enough memory to load audio file" );
    }

    end:
    return sampleBuffer;
}



SharedSampleBuffer AudioFileHandler::aubioLoadFile( const char* filePath, uint_t startFrame, uint_t numFramesToRead )
{
    const uint_t hopSize = 4096;

    uint_t sampleRate = 0; // If `0` is passed as `samplerate` to new_aubio_source, the sample rate of the original file is used.

    uint_t endFrame = 0; // Exclusive
    uint_t destStartFrame = 0; // Inclusive
    uint_t numFramesRead = 0;
    uint_t numFramesToCopy = 0;

    uint_t numChans = 0;

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
                // If caller has not set `numFramesToRead` assume whole file should be read

                if ( numFramesToRead < 1 ) // Read whole file
                {
                    startFrame = 0;
                    numFramesToRead = 0;

                    // Work out the no. of frames the long way
                    do
                    {
                        aubio_source_do_multi( aubioSource, sampleData, &numFramesRead );
                        numFramesToRead += numFramesRead;
                    }
                    while ( numFramesRead == hopSize );

                    aubio_source_seek( aubioSource, 0 );
                    numFramesRead = 0;
                }
                else // Read part of file
                {
                    aubio_source_seek( aubioSource, startFrame );
                }

                endFrame = startFrame + numFramesToRead;

                fmat_zeros( sampleData );

                try
                {
                    sampleBuffer = SharedSampleBuffer( new SampleBuffer( numChans, numFramesToRead ) );

                    // Read audio data from file
                    do
                    {
                        aubio_source_do_multi( aubioSource, sampleData, &numFramesRead );

                        numFramesToCopy = startFrame + numFramesRead <= endFrame ?
                                          numFramesRead : endFrame - startFrame;

                        for ( uint_t chanNum = 0; chanNum < numChans; chanNum++ )
                        {
                            sampleBuffer->copyFrom( chanNum, destStartFrame, sampleData->data[ chanNum ], numFramesToCopy );
                        }

                        startFrame += numFramesRead;
                        destStartFrame += numFramesRead;
                    }
                    while ( startFrame < endFrame );
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
