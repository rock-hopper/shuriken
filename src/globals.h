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
    static const int WAVEFORM       = QGraphicsItem::UserType + 1;
    static const int FRAME_MARKER   = QGraphicsItem::UserType + 2;
    static const int SLICE_POINT    = QGraphicsItem::UserType + 3;
    static const int LOOP_MARKER    = QGraphicsItem::UserType + 4;
}

namespace ZValues
{
    static const int WAVEFORM           = 0;
    static const int SELECTED_WAVEFORM  = 1;
    static const int FRAME_MARKER       = 2;
    static const int LOOP_MARKER        = 3;
    static const int PLAYHEAD           = 5;
}

namespace Ruler
{
    static const qreal HEIGHT = 18.0;
}


namespace Midi
{
    static const int MIDDLE_C       = 60;
    static const int MAX_POLYPHONY  = 128;
}


namespace Jack
{
    // This gets set in: JuceLibraryCode/modules/juce_audio_devices/native/juce_linux_JackAudio.cpp
    //                   void process (int numFrames)
    //
    // and is read in:   rubberbandaudiosource.cpp
    //                   void getNextAudioBlock( AudioSourceChannelInfo& bufferToFill )
    //
    extern volatile double g_currentBPM;
}


#endif // GLOBALS_H
