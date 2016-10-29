/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#include "akaifilehandler.h"
#include "globals.h"
#include <QFile>
#include <QDir>
#include <QtDebug>


//==================================================================================================
// Public Static:

int AkaiFileHandler::getNumPads( const int modelID )
{
    int numPads = 0;

    switch ( modelID )
    {
    case AkaiModelID::MPC1000_ID:
        numPads = MPC1000_Profile::NUM_PADS;
        break;
    case AkaiModelID::MPC500_ID:
        numPads = MPC500_Profile::NUM_PADS;
        break;
    default:
        break;
    }

    return numPads;
}



bool AkaiFileHandler::writePgmFileMPC1000( QStringList sampleNames,
                                           const QString fileBaseName,
                                           const QString outputDirPath,
                                           const QString tempDirPath,
                                           const bool isVoiceOverlapMono,
                                           const int muteGroup,
                                           const SamplerAudioSource::EnvelopeSettings& envelopes,
                                           const bool isOverwriteEnabled )
{
    const QString fileName = fileBaseName + getFileExtension();

    if ( QDir( outputDirPath ).exists( fileName ) && ! isOverwriteEnabled )
    {
        return false;
    }

    while ( sampleNames.size() > MPC1000_Profile::NUM_PADS )
    {
        sampleNames.removeLast();
    }

    QByteArray pgmData( MPC1000_PGM::FILE_SIZE, PADDING );

    bool isSuccessful = getTemplateDataMPC1000( pgmData );

    if ( isSuccessful )
    {
        quint8 noteNum = Midi::MIDDLE_C;

        for ( quint8 padNum = 0; padNum < sampleNames.size(); padNum++ )
        {
            // Add sample name to PGM data
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE );

                QByteArray bytes = sampleNames.at( padNum ).toLatin1().leftJustified( MPC1000_PGM::SAMPLE_NAME_SIZE, PADDING, true );

                pgmData.replace( pos, MPC1000_PGM::SAMPLE_NAME_SIZE, bytes );
            }

            // Add sample volume level
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::LEVEL_OFFSET;

                QByteArray bytes;
                bytes += quint8( 100 );

                pgmData.replace( pos, 1, bytes );
            }

            // Add play mode
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::PLAY_MODE_OFFSET;

                QByteArray bytes;
                bytes += envelopes.oneShotSettings.at( padNum ) ? (char) 0x0     // 0 - One shot is set
                                                                : quint8( 1 );   // 1 - One shot is not set

                pgmData.replace( pos, 1, bytes );
            }

            // Add voice overlap
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::VOICE_OVERLAP_OFFSET;

                QByteArray bytes;
                bytes += isVoiceOverlapMono ? quint8( 1 )    // 1 - Mono
                                            : (char) 0x0;    // 0 - Poly

                pgmData.replace( pos, 1, bytes );
            }

            // Add mute group
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::MUTE_GROUP_OFFSET;

                QByteArray bytes;
                bytes += muteGroup;

                pgmData.replace( pos, 1, bytes );
            }

            // Add attack
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::ATTACK_OFFSET;

                QByteArray bytes;
                bytes += quint8( envelopes.attackValues.at( padNum ) * 100 );

                pgmData.replace( pos, 1, bytes );
            }

            // Add decay
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::DECAY_OFFSET;

                QByteArray bytes;
                bytes += quint8( envelopes.releaseValues.at( padNum ) * 100 );

                pgmData.replace( pos, 1, bytes );
            }

            // Add decay mode
            {
                const int pos = MPC1000_PGM::HEADER_SIZE + ( padNum * MPC1000_PGM::PAD_DATA_SIZE ) + MPC1000_PGM::DECAY_MODE_OFFSET;

                QByteArray bytes;
                bytes += (char) 0x0;

                pgmData.replace( pos, 1, bytes );
            }

            // Add "pad" -> "MIDI note" mapping
            {
                const int pos = MPC1000_PGM::PAD_MIDI_DATA_START + padNum;

                QByteArray bytes;
                bytes += noteNum;

                pgmData.replace( pos, 1, bytes );
            }

            // Add "MIDI note" -> "pad" mapping
            {
                const int pos = MPC1000_PGM::MIDI_NOTE_DATA_START + noteNum;

                QByteArray bytes;
                bytes += padNum;

                pgmData.replace( pos, 1, bytes );
            }

            noteNum++;
        }

        // Write PGM data to file
        const QString tempFilePath = QDir( tempDirPath ).absoluteFilePath( fileName );
        QFile tempFile( tempFilePath );

        isSuccessful = tempFile.open( QIODevice::WriteOnly );

        if ( isSuccessful )
        {
            QDataStream outStream( &tempFile );

            const int numBytesWritten = outStream.writeRawData( pgmData.data(), MPC1000_PGM::FILE_SIZE );

            if ( numBytesWritten != MPC1000_PGM::FILE_SIZE )
            {
                isSuccessful = false;
            }
            else
            {
                const QString outputFilePath = QDir( outputDirPath ).absoluteFilePath( fileName );

                QFile::remove( outputFilePath );
                tempFile.copy( outputFilePath );
            }

            tempFile.remove();
        }
    }

    return isSuccessful;
}



bool AkaiFileHandler::writePgmFileMPC500( QStringList sampleNames,
                                          const QString fileBaseName,
                                          const QString outputDirPath,
                                          const QString tempDirPath,
                                          const bool isVoiceOverlapMono,
                                          const int muteGroup,
                                          const SamplerAudioSource::EnvelopeSettings& envelopes,
                                          const bool isOverwriteEnabled )
{
    while ( sampleNames.size() > MPC500_Profile::NUM_PADS )
    {
        sampleNames.removeLast();
    }

    return writePgmFileMPC1000( sampleNames, fileBaseName, outputDirPath, tempDirPath, isVoiceOverlapMono, muteGroup, envelopes, isOverwriteEnabled );
}



//==================================================================================================
// Private Static:

bool AkaiFileHandler::getTemplateDataMPC1000( QByteArray& pgmData )
{
    QFile templateFile( ":/resources/akai/Template.pgm" );

    bool isSuccessful = templateFile.open( QIODevice::ReadOnly );

    if ( isSuccessful )
    {
        QDataStream inStream( &templateFile );

        const int numBytesRead = inStream.readRawData( pgmData.data(), MPC1000_PGM::FILE_SIZE );

        if ( numBytesRead != MPC1000_PGM::FILE_SIZE )
        {
            isSuccessful = false;
        }
    }

    return isSuccessful;
}
