/*
  This file contains code originally written by "jpo"
  <http://www.juce.com/comment/296820#comment-296820>

  All modifications to jpo's code by Andrew M Taylor <a.m.taylor303@gmail.com>, 2014

  This is free and unencumbered software released into the public domain.
  Please read UNLICENSE for more details, or refer to <http://unlicense.org/>

*/

#ifndef JACK_DEVICE_H
#define JACK_DEVICE_H

#include "globals.h"


struct JackClientConfig
{
    String clientName;

    // size of array = number of input channels. If the strings are empty, default names are chosen (in_1 , in_2 etc)
    StringArray inputChannels;
    StringArray outputChannels;

    bool isMidiEnabled;
    bool isAutoConnectEnabled;
};



void getDefaultJackClientConfig (JackClientConfig &conf)
{
    if ( ! Jack::g_clientId.isEmpty() )
    {
        conf.clientName = Jack::g_clientId.toLocal8Bit().data();
    }
    else
    {
        conf.clientName = APPLICATION_NAME;
    }
    conf.outputChannels.addTokens (OUTPUT_CHAN_NAMES, false);
    conf.isMidiEnabled = false;
    conf.isAutoConnectEnabled = false;
}


#endif // JACK_DEVICE_H
