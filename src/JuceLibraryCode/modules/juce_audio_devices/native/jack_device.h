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

  This file contains code originally written by "jpo"
  <http://www.juce.com/comment/296820#comment-296820>
*/

#ifndef JACK_DEVICE_H
#define JACK_DEVICE_H

struct JackSessionCallbackArg
{
    String session_directory;
    String session_uuid;
    String command_line;
    bool quit;
};


struct JackClientConfig
{
    String clientName;

    // size of array = number of input channels. If the strings are empty, default names are chosen (in_1 , in_2 etc)
    StringArray inputChannels;
    StringArray outputChannels;

    bool isMidiEnabled;
    bool isAutoConnectEnabled;

    String session_uuid;

    typedef void (*SessionCallback)(JackSessionCallbackArg &arg);
    SessionCallback sessionCallback;
};


void getDefaultJackClientConfig (JackClientConfig &conf)
{
    conf.clientName = APPLICATION_NAME;
    conf.outputChannels.addTokens (OUTPUT_CHAN_NAMES, false);
    conf.isMidiEnabled = false;
    conf.isAutoConnectEnabled = false;
    conf.sessionCallback = nullptr;
}

#endif // JACK_DEVICE_H
