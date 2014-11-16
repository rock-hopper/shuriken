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

#ifndef AKAIFILEHANDLER_H
#define AKAIFILEHANDLER_H

#include <QStringList>


namespace AkaiModelID
{
    static const int MPC1000_ID = 0;
    static const int MPC500_ID  = 1;
}


class AkaiFileHandler
{
public:
    static int getNumPads( const int modelID );

    static QString getFileNameRegExpMPC1000()       { return QString( "\\w{1,16}" ); }

    static bool writePgmFileMPC1000( QStringList sampleNames,
                                     QString fileBaseName,
                                     QString outputDirPath,
                                     QString tempDirPath,
                                     bool isOverwriteEnabled = true );

    static bool writePgmFileMPC500( QStringList sampleNames,
                                    QString fileBaseName,
                                    QString outputDirPath,
                                    QString tempDirPath,
                                    bool isOverwriteEnabled = true );

    static QString getFileExtension()               { return QString( ".pgm" ); }

private:
    static bool getTemplateDataMPC1000( QByteArray& pgmData );

    struct MPC1000_Profile
    {
        static const int NUM_PHYSICAL_PADS      = 16;
        static const int NUM_PAD_BANKS          = 4;
        static const int NUM_PADS               = NUM_PHYSICAL_PADS * NUM_PAD_BANKS;
    };

    struct MPC500_Profile
    {
        static const int NUM_PHYSICAL_PADS      = 12;
        static const int NUM_PAD_BANKS          = 4;
        static const int NUM_PADS               = NUM_PHYSICAL_PADS * NUM_PAD_BANKS;
    };

    struct MPC1000_PGM
    {
        static const int FILE_SIZE              = 0x2A04;
        static const int PAD_DATA_START         = 0x18;
        static const int PAD_DATA_SIZE          = 0xA4;
        static const int PAD_DATA_LEVEL_OFFSET  = 0x11;
        static const int SAMPLE_NAME_SIZE       = 16;
        static const int PAD_MIDI_DATA_START    = 0x2918;
        static const int MIDI_NOTE_DATA_START   = 0x2958;
    };

    static const char PAD_BYTE = 0x00;
};


#endif // AKAIFILEHANDLER_H
