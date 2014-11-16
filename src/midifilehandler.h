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

#ifndef MIDIFILEHANDLER_H
#define MIDIFILEHANDLER_H

#include "JuceHeader.h"
#include "samplebuffer.h"


class MidiFileHandler
{
public:
    enum MidiFileType { MIDI_FILE_TYPE_0 = 0, MIDI_FILE_TYPE_1 = 1 };

    static bool SaveMidiFile( QString fileBaseName,
                              QString outputDirPath,
                              QList<SharedSampleBuffer> sampleBufferList,
                              int numSampleBuffers,
                              qreal sampleRate,
                              qreal bpm,
                              int timeSigNumerator,
                              int timeSigDenominator,
                              MidiFileType midiFileType,
                              bool isOverwriteEnabled = true );

    static QString getFileExtension()           { return QString( ".mid" ); }

private:
    static const int SECONDS_PER_MINUTE   = 60;
    static const int MICROSECS_PER_MINUTE = 60000000;

    static const int TICKS_PER_QUART_NOTE = 960;
};

#endif // MIDIFILEHANDLER_H
