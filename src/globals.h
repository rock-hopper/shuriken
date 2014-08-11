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

#ifndef GLOBALS_H
#define GLOBALS_H


#define APPLICATION_NAME            "Shuriken"
#define JUCE_ALSA_MIDI_INPUT_NAME   "Midi_In"
#define NUM_INPUT_CHANS             0
#define NUM_OUTPUT_CHANS            2
#define OUTPUT_CHAN_NAMES           "out_L out_R"   // Names must be separated by whitespace

#define AUDIO_CONFIG_FILE_PATH      "~/.shuriken/audioconfig.xml"
#define PATHS_CONFIG_FILE_PATH      "~/.shuriken/pathsconfig.xml"


#include <QGraphicsItem>

namespace UserTypes
{
    static const int waveformItem       = QGraphicsItem::UserType + 1;
    static const int frameMarkerItem    = QGraphicsItem::UserType + 2;
    static const int slicePointItem     = QGraphicsItem::UserType + 3;
    static const int playheadItem       = QGraphicsItem::UserType + 4;
}


extern volatile double gCurrentJackBPM;


#endif // GLOBALS_H
