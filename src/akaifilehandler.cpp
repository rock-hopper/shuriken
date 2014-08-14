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

#include "akaifilehandler.h"
#include <QFile>
#include <QDir>
#include <QDebug>


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



bool AkaiFileHandler::writePgmFileMPC1000( const QStringList sampleNames,
                                           const QString fileBaseName,
                                           const QString outputDirPath,
                                           const QString tempDirPath,
                                           const bool isOverwriteEnabled )
{
    const QString fileName = fileBaseName + ".pgm";

    if ( QDir( outputDirPath ).exists( fileName ) && ! isOverwriteEnabled )
    {
        return false;
    }

    QByteArray pgmData( MPC1000_PGM::FILE_SIZE, PAD_BYTE );

    bool isSuccessful = getTemplateDataMPC1000( pgmData );

    if ( isSuccessful )
    {
        quint8 noteNum = 60; // MIDI note C4

        for ( quint8 padNum = 0; padNum < MPC1000_Profile::NUM_PADS && padNum < sampleNames.size(); padNum++ )
        {
            // Add sample names to PGM data
            {
                QByteArray sampleName = sampleNames.at( padNum ).toAscii().leftJustified( MPC1000_PGM::SAMPLE_NAME_SIZE, PAD_BYTE, true );

                const int pos = MPC1000_PGM::PAD_DATA_START + ( padNum * MPC1000_PGM::PAD_DATA_SIZE );

                pgmData.replace( pos, MPC1000_PGM::SAMPLE_NAME_SIZE, sampleName );
            }

            // Add pad MIDI note mapping to PGM data
            {
                const int pos = MPC1000_PGM::PAD_MIDI_DATA_START + padNum;

                pgmData.replace( pos, 1, (char*) &noteNum );
            }

            // Add MIDI note pad mapping to PGM data
            {
                const int pos = MPC1000_PGM::MIDI_NOTE_DATA_START + noteNum;

                pgmData.replace( pos, 1, (char*) &padNum );
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
                                          const bool isOverwriteEnabled )
{
    while ( sampleNames.size() > MPC500_Profile::NUM_PADS )
    {
        sampleNames.removeLast();
    }

    return writePgmFileMPC1000( sampleNames, fileBaseName, outputDirPath, tempDirPath, isOverwriteEnabled );
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
