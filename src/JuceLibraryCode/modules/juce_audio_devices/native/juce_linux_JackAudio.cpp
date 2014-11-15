/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================

  This file contains code originally written by "jpo"
  <http://www.juce.com/comment/296820#comment-296820>

  Additional modifications to this file by Andrew M Taylor <a.m.taylor303@gmail.com>, 2014

  All modifications to the original file are released into the public domain.
  Please read UNLICENSE for more details, or refer to <http://unlicense.org/>

*/

//==============================================================================

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/transport.h>
#include <jack/session.h>

#include "jack_device.h"
#include "linux_midi.h"
#include "globals.h"


extern "C" int libjack_is_present;
extern "C" int libjack_session_is_supported;


//==============================================================================

class JackAudioIODevice : public AudioIODevice
{
public:
    JackAudioIODevice (const String& deviceName, const JackClientConfig& config) :
        AudioIODevice (deviceName, "JACK"),
        mConfig (config),
        mIsDeviceOpen (false),
        mIsDevicePlaying (false),
        mIsClientActivated (false),
        mAudioIOCallback (nullptr),
        mJackClient (nullptr),
        mMidiPortIn (nullptr),
        mPositionInfo (new jack_position_t)
    {
        for (int i=0; i < mConfig.inputChannels.size(); ++i)
        {
            if (mConfig.inputChannels[i].length() == 0) mConfig.inputChannels.set(i, "in_"+String(i+1));
        }
        for (int i = 0; i < mConfig.outputChannels.size(); i++)
        {
            if (mConfig.outputChannels[i].length() == 0) mConfig.outputChannels.set(i, "out_"+String(i+1));
        }

        jack_set_error_function (JackAudioIODevice::errorCallback);
        jack_status_t status;

        if (mConfig.session_uuid.isNotEmpty() && libjack_session_is_supported)
        {
            //std::cerr << "JackAudioIODevice: opening with session_uuid: '" << config.session_uuid << "'\n";
            mJackClient = jack_client_open (mConfig.clientName.toUTF8().getAddress(), JackSessionID, &status, mConfig.session_uuid.toUTF8().getAddress());
        }
        else
        {
            //std::cerr << "JackAudioIODevice: opening WITHOUT session_uuid: '" << config.session_uuid << "'\n";
            mJackClient = jack_client_open (mConfig.clientName.toUTF8().getAddress(), JackNoStartServer, &status);
        }

        if (mJackClient == nullptr)
        {
            if ((status & JackServerFailed) || (status & JackServerError))
                printf ("Unable to connect to JACK server\n");
            else if ((status & JackVersionError))
                printf ("Client's protocol version does not match\n");
            else if ((status & JackInvalidOption))
                printf ("The operation contained an invalid or unsupported option\n");
            else if ((status & JackNameNotUnique))
                printf ("The desired client name was not unique\n");
            else if ((status & JackNoSuchClient))
                printf ("Requested client does not exist\n");
            else if ((status & JackInitFailure))
                printf ("Unable to initialize client\n");
            else printf ("Unknown jack error [%d]\n", (int)status);
        }
        else
        {

            for (int i=0; i < mConfig.inputChannels.size(); ++i)
            {
                jack_port_t* input =
                        jack_port_register (mJackClient, mConfig.inputChannels[i].toUTF8().getAddress(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                mInputPorts.add (input);
            }

            for (int i = 0; i < mConfig.outputChannels.size(); i++)
            {
                jack_port_t* output =
                        jack_port_register (mJackClient, mConfig.outputChannels[i].toUTF8().getAddress(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                mOutputPorts.add (output);
            }

            if (mConfig.isMidiEnabled)
            {
                mMidiPortIn = jack_port_register(mJackClient, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
            }

            if (libjack_session_is_supported && mConfig.sessionCallback)
            {
                jack_set_session_callback(mJackClient, sessionCallback, this);
            }
        }

        mInChanBuffers  = (float**)malloc(mConfig.inputChannels.size() *sizeof(float*));
        mOutChanBuffers = (float**)malloc(mConfig.outputChannels.size()*sizeof(float*));
    }



    ~JackAudioIODevice()
    {
        if (mJackClient)
        {
            close ();

            jack_client_close (mJackClient);
            mJackClient = nullptr;
        }
        free(mInChanBuffers);
        free(mOutChanBuffers);
    }



    StringArray getOutputChannelNames() { return mConfig.outputChannels; }



    StringArray getInputChannelNames() { return mConfig.inputChannels; }



    Array<double> getAvailableSampleRates() override
    {
        Array<double> rates;

        if (mJackClient != nullptr)
            rates.add (jack_get_sample_rate (mJackClient));

        return rates;
    }



    Array<int> getAvailableBufferSizes() override
    {
        Array<int> sizes;

        if (mJackClient != nullptr)
            sizes.add (jack_get_buffer_size (mJackClient));

        return sizes;
    }



    int getDefaultBufferSize() override             { return getCurrentBufferSizeSamples(); }



    int getCurrentBufferSizeSamples() override      { return mJackClient != nullptr ? jack_get_buffer_size (mJackClient) : 0; }



    double getCurrentSampleRate() override          { return mJackClient != nullptr ? jack_get_sample_rate (mJackClient) : 0; }



    String open (const BitArray& /*inputChannels*/,
                 const BitArray& /*outputChannels*/,
                 double /*sampleRate*/,
                 int /*bufferSizeSamples*/) override
    {
        if (! mJackClient)
        {
            return "Jack server is not running";
        }

        close();

        // activate client !
        jack_set_process_callback (mJackClient, JackAudioIODevice::processCallback, this);
        jack_on_shutdown (mJackClient, JackAudioIODevice::shutdownCallback, this);

        jack_activate (mJackClient); mIsClientActivated = true;

        if (mConfig.isAutoConnectEnabled)
        {
            const char **ports = jack_get_ports (mJackClient, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
            for (int i=0; i < 2 && i < mOutputPorts.size() && ports[i]; ++i)
            {
                jack_connect(mJackClient, jack_port_name(mOutputPorts[i]), ports[i]);
            }
        }

        mIsDeviceOpen = true;

        return String::empty;
    }



    void close() override
    {
        stop();

        if (mJackClient && mIsClientActivated)
        {
            jack_deactivate (mJackClient); mIsClientActivated = false;

            jack_set_process_callback (mJackClient, JackAudioIODevice::processCallback, nullptr);
            jack_on_shutdown (mJackClient, JackAudioIODevice::shutdownCallback, nullptr);
        }

        mIsDeviceOpen = false;

        gCurrentJackBPM = 0.0;
    }



    void start (AudioIODeviceCallback* callback) override
    {
        if (! mIsDeviceOpen)
            callback = nullptr;

        mAudioIOCallback = callback;

        if (mAudioIOCallback != nullptr)
            mAudioIOCallback->audioDeviceAboutToStart (this);

        mIsDevicePlaying = (mAudioIOCallback != nullptr);
    }



    void stop() override
    {
        AudioIODeviceCallback* const oldCallback = mAudioIOCallback;

        start (nullptr);

        if (oldCallback != nullptr)
            oldCallback->audioDeviceStopped();
    }



    bool isOpen() override
    {
        return mIsDeviceOpen;
    }



    bool isPlaying() override
    {
        return mIsDevicePlaying;
    }



    int getCurrentBitDepth() override
    {
        return 32;
    }



    String getLastError() override
    {
        return String::empty;
    }



    BitArray getActiveOutputChannels() const override
    {
        BitArray outputBits;
        outputBits.setRange(0, mOutputPorts.size(), true);
        return outputBits;
    }



    BitArray getActiveInputChannels() const override
    {
        BitArray inputBits;
        inputBits.setRange(0, mInputPorts.size(), true);
        return inputBits;
    }



    int getOutputLatencyInSamples() override
    {
        int latency = 0;

        for (int i = 0; i < mOutputPorts.size(); i++)
            latency = jmax (latency, (int) jack_port_get_total_latency (mJackClient, mOutputPorts [i]));

        return latency;
    }



    int getInputLatencyInSamples() override
    {
        int latency = 0;

        for (int i = 0; i < mInputPorts.size(); i++)
            latency = jmax (latency, (int) jack_port_get_total_latency (mJackClient, mInputPorts [i]));

        return latency;
    }



    String inputId, outputId;



private:
    void process (int numFrames)
    {
        jack_transport_query (mJackClient, mPositionInfo);
        gCurrentJackBPM = mPositionInfo->beats_per_minute;

        if (mMidiPortIn != nullptr && gJackMidiClient != nullptr)
        {
            void* buffer = jack_port_get_buffer (mMidiPortIn, numFrames);
            jack_nframes_t event_count = jack_midi_get_event_count (buffer);
            jack_midi_event_t in_event;

            for (jack_nframes_t i=0; i < event_count; ++i)
            {
                jack_midi_event_get (&in_event, buffer, i);

                //std::cerr << "add event : "<< (void*)*(const uint8*)in_event.buffer << ", sz=" << in_event.size << " sample: " << in_event.time << "\n";

                const MidiMessage message ((const uint8*) in_event.buffer, in_event.size, Time::getMillisecondCounterHiRes() * 0.001);
                gJackMidiClient->handleIncomingMidiMessage (message, 0);
            }
        }

        int i, numActiveInChans = 0, numActiveOutChans = 0;

        for (i = 0; i < mInputPorts.size(); ++i)
        {
            jack_default_audio_sample_t *in =
                (jack_default_audio_sample_t *) jack_port_get_buffer (mInputPorts.getUnchecked(i), numFrames);
            jassert (in != nullptr);
            mInChanBuffers [numActiveInChans++] = (float*) in;
        }

        for (i = 0; i < mOutputPorts.size(); ++i)
        {
            jack_default_audio_sample_t *out =
                (jack_default_audio_sample_t *) jack_port_get_buffer (mOutputPorts.getUnchecked(i), numFrames);
            jassert (out != nullptr);
            mOutChanBuffers [numActiveOutChans++] = (float*) out;
        }

        if (mAudioIOCallback != nullptr)
        {
            mAudioIOCallback->audioDeviceIOCallback ((const float**) mInChanBuffers,
                                             mInputPorts.size(),
                                             mOutChanBuffers,
                                             mOutputPorts.size(),
                                             numFrames);
        }
        else
        {
            for (i = 0; i < mOutputPorts.size(); ++i)
                zeromem (mOutChanBuffers[i], sizeof (float) * numFrames);
        }
    }



    static int processCallback (jack_nframes_t nframes, void* callbackArgument)
    {
        JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;

        if (device)
            device->process (nframes);

        return 0;
    }



    struct SessionCallbackMessage
    {
        jack_session_event_t *event;
    };



//    void handleMessage(const Message &msg) {
//      const SessionCallbackMessage *sm;
//      //printf("sessionCallback, received message\n");
//      if ((sm = dynamic_cast<const SessionCallbackMessage*>(&msg))) {
//        if (config.sessionCallback) {
//          JackSessionCallbackArg arg;
//          arg.session_directory = sm->event->session_dir;
//          arg.session_uuid = sm->event->client_uuid;
//          arg.quit = (sm->event->type == JackSessionSaveAndQuit);
//          config.sessionCallback(arg);
//
//          sm->event->command_line = strdup(arg.command_line.toUTF8().getAddress());
//        }
//        jack_session_reply(client, sm->event);
//        jack_session_event_free(sm->event);
//      }
//    }



    static void sessionCallback (jack_session_event_t *event, void *callbackArgument)
    {
//      //printf("sessionCallback, posting message\n");
//      JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;
//      SessionCallbackMessage *m = new SessionCallbackMessage;
//      device->postMessage(m);
    }



    static void threadInitCallback (void* /*callbackArgument*/) {}



    static void shutdownCallback (void* callbackArgument)
    {
        JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;

        if (device)
        {
            device->mJackClient = 0;
            device->close ();
        }
    }



    static void errorCallback (const char *msg)
    {
        char errmsg[1024];
        const char *extra_msg = "";
        if (strcmp(msg, "Only external clients need attach port segments")==0) {
            extra_msg = "\nThis probably means that you are trying to connect a 32-bit jack client to a 64-bit server -- you need to make sure that you are using a recent version of jack (at least 0.116)";
        }
        snprintf(errmsg, 1024, "Jack error: %s%s", msg, extra_msg);
        fprintf (stderr, "%s\n", errmsg);
    }



    JackClientConfig mConfig;
    bool mIsDeviceOpen, mIsDevicePlaying;
    bool mIsClientActivated;

    AudioIODeviceCallback* mAudioIOCallback;

    float** mInChanBuffers;
    float** mOutChanBuffers;

    jack_client_t* mJackClient;

    Array<jack_port_t*> mInputPorts;
    Array<jack_port_t*> mOutputPorts;
    jack_port_t* mMidiPortIn;

    ScopedPointer<jack_position_t> mPositionInfo;
};



//==============================================================================

class JackAudioIODeviceType  : public AudioIODeviceType
{
public:

    //==============================================================================

    JackAudioIODeviceType() : AudioIODeviceType("JACK") {}


    ~JackAudioIODeviceType() {}


    //==============================================================================

    void scanForDevices() {}


    StringArray getDeviceNames (const bool /*wantInputNames*/) const
    {
        StringArray deviceNames;

        deviceNames.add ("JACK Audio + MIDI, Auto-Connect Outputs");
        deviceNames.add ("JACK Audio Only, Auto-Connect Outputs");
        deviceNames.add ("JACK Audio + MIDI");
        deviceNames.add ("JACK Audio Only");

        return deviceNames;
    }



    int getDefaultDeviceIndex (const bool /*forInput*/) const
    {
        return 0; // "JACK Audio + MIDI, Auto-Connect Outputs"
    }



    bool hasSeparateInputsAndOutputs() const    { return false; }



    int getIndexOfDevice (AudioIODevice* device, const bool /*asInput*/) const
    {
        if (device == nullptr)
            return -1;

        return getDeviceNames (false).indexOf (device->getName());
    }



    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& /*inputDeviceName*/)
    {
        JackClientConfig config;
        getDefaultJackClientConfig (config);

        config.isMidiEnabled = outputDeviceName.contains ("MIDI");
        config.isAutoConnectEnabled = outputDeviceName.contains ("Auto-Connect");

        return new JackAudioIODevice (outputDeviceName, config);
    }



    //==============================================================================

    juce_UseDebuggingNewOperator

private:

    JackAudioIODeviceType (const JackAudioIODeviceType&);
    const JackAudioIODeviceType& operator= (const JackAudioIODeviceType&);
};



//==============================================================================

AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()
{
    // Detect if libjack.so is available using relaytool on linux
    if (!libjack_is_present)
        return nullptr;
    else
        return new JackAudioIODeviceType();
}
