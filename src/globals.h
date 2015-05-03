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

#ifndef GLOBALS_H
#define GLOBALS_H


#define APPLICATION_NAME            "Shuriken"
#define JUCE_ALSA_MIDI_INPUT_NAME   "Midi_In"
#define MAX_INPUT_CHANS             0
#define MAX_OUTPUT_CHANS            2
#define OUTPUT_CHAN_NAMES           "out_L out_R"   // Names must be separated by whitespace

#define AUDIO_CONFIG_FILE_PATH      "~/.shuriken/audioconfig.xml"
#define PATHS_CONFIG_FILE_PATH      "~/.shuriken/pathsconfig.xml"

#define FILE_EXTENSION              ".shuriken"


#include <QGraphicsItem>

namespace UserTypes
{
    const int WAVEFORM       = QGraphicsItem::UserType + 1;
    const int FRAME_MARKER   = QGraphicsItem::UserType + 2;
    const int SLICE_POINT    = QGraphicsItem::UserType + 3;
    const int LOOP_MARKER    = QGraphicsItem::UserType + 4;
}

namespace ZValues
{
    const int WAVEFORM               = 0;
    const int SELECTED_WAVEFORM      = 1;
    const int FRAME_MARKER           = 2;
    const int SELECTED_FRAME_MARKER  = 3;
    const int LOOP_MARKER            = 4;
    const int PLAYHEAD               = 5;
    const int BPM_RULER              = 6;
    const int BPM_RULER_TEXT         = 7;
}

namespace BpmRuler
{
    const qreal HEIGHT = 18.0;
}


namespace Midi
{
    const int MIDDLE_C       = 60;
    const int MAX_POLYPHONY  = 128;
}


namespace Jack
{
    /* This gets set in: JuceLibraryCode/modules/juce_audio_devices/native/juce_linux_JackAudio.cpp
                         void process (int numFrames)

       and is read in:   rubberbandaudiosource.cpp
                         void getNextAudioBlock( AudioSourceChannelInfo& bufferToFill )
    */
    extern double g_currentBPM;

    /* This gets set in: mainwindow.cpp
                         int nsmOpenCallback( ... )

       and is read in:   JuceLibraryCode/modules/juce_audio_devices/native/jack_device.h
                         void getDefaultJackClientConfig (JackClientConfig &conf)
    */
    extern QString g_clientId;
}


#endif // GLOBALS_H
