/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
*/

//==============================================================================
static void* juce_libjackHandle = nullptr;

static void __attribute__((constructor)) tryload_libjack()
{
    if (getenv("SKIP_LIBJACK") == 0)
    {
        juce_libjackHandle = dlopen ("libjack.so.0", RTLD_LAZY);
    }
}

static void* juce_loadJackFunction (const char* const name)
{
    if (juce_libjackHandle == nullptr)
        return nullptr;

    return dlsym (juce_libjackHandle, name);
}

#define JUCE_DECL_JACK_FUNCTION(return_type, fn_name, argument_types, arguments)  \
  return_type fn_name argument_types                                              \
  {                                                                               \
      typedef return_type (*fn_type) argument_types;                              \
      static fn_type fn = (fn_type) juce_loadJackFunction (#fn_name);             \
      return (fn != nullptr) ? ((*fn) arguments) : (return_type) 0;               \
  }

#define JUCE_DECL_VOID_JACK_FUNCTION(fn_name, argument_types, arguments)          \
  void fn_name argument_types                                                     \
  {                                                                               \
      typedef void (*fn_type) argument_types;                                     \
      static fn_type fn = (fn_type) juce_loadJackFunction (#fn_name);             \
      if (fn != nullptr) (*fn) arguments;                                         \
  }

jack_transport_state_t jack_transport_query(const jack_client_t* client, jack_position_t* pos)
{
    typedef jack_transport_state_t (*jack_transport_query_ptr_t)(const jack_client_t* client, jack_position_t* pos);
    static jack_transport_query_ptr_t fn = (jack_transport_query_ptr_t) juce_loadJackFunction ("jack_transport_query");
    return (fn != nullptr) ? (*fn)(client, pos) : JackTransportStopped;
}

//==============================================================================
JUCE_DECL_JACK_FUNCTION (jack_client_t*, jack_client_open, (const char* client_name, jack_options_t options, jack_status_t* status, ...), (client_name, options, status));
JUCE_DECL_JACK_FUNCTION (int, jack_client_close, (jack_client_t *client), (client));
JUCE_DECL_JACK_FUNCTION (int, jack_activate, (jack_client_t* client), (client));
JUCE_DECL_JACK_FUNCTION (int, jack_deactivate, (jack_client_t* client), (client));
JUCE_DECL_JACK_FUNCTION (jack_nframes_t, jack_get_buffer_size, (jack_client_t* client), (client));
JUCE_DECL_JACK_FUNCTION (jack_nframes_t, jack_get_sample_rate, (jack_client_t* client), (client));
JUCE_DECL_VOID_JACK_FUNCTION (jack_on_shutdown, (jack_client_t* client, void (*function)(void* arg), void* arg), (client, function, arg));
JUCE_DECL_JACK_FUNCTION (void* , jack_port_get_buffer, (jack_port_t* port, jack_nframes_t nframes), (port, nframes));
JUCE_DECL_JACK_FUNCTION (jack_nframes_t, jack_port_get_total_latency, (jack_client_t* client, jack_port_t* port), (client, port));
JUCE_DECL_JACK_FUNCTION (jack_port_t* , jack_port_register, (jack_client_t* client, const char* port_name, const char* port_type, unsigned long flags, unsigned long buffer_size), (client, port_name, port_type, flags, buffer_size));
JUCE_DECL_JACK_FUNCTION (int, jack_port_unregister, (jack_client_t *client, jack_port_t *port), (client, port));
JUCE_DECL_VOID_JACK_FUNCTION (jack_set_error_function, (void (*func)(const char*)), (func));
JUCE_DECL_JACK_FUNCTION (int, jack_set_process_callback, (jack_client_t* client, JackProcessCallback process_callback, void* arg), (client, process_callback, arg));
JUCE_DECL_JACK_FUNCTION (const char**, jack_get_ports, (jack_client_t* client, const char* port_name_pattern, const char* type_name_pattern, unsigned long flags), (client, port_name_pattern, type_name_pattern, flags));
JUCE_DECL_JACK_FUNCTION (int, jack_connect, (jack_client_t* client, const char* source_port, const char* destination_port), (client, source_port, destination_port));
JUCE_DECL_JACK_FUNCTION (const char*, jack_port_name, (const jack_port_t* port), (port));
JUCE_DECL_JACK_FUNCTION (void*, jack_set_port_connect_callback, (jack_client_t* client, JackPortConnectCallback connect_callback, void* arg), (client, connect_callback, arg));
JUCE_DECL_JACK_FUNCTION (jack_port_t* , jack_port_by_id, (jack_client_t* client, jack_port_id_t port_id), (client, port_id));
JUCE_DECL_JACK_FUNCTION (int, jack_port_connected, (const jack_port_t* port), (port));
JUCE_DECL_JACK_FUNCTION (int, jack_port_connected_to, (const jack_port_t* port, const char* port_name), (port, port_name));
JUCE_DECL_JACK_FUNCTION (jack_nframes_t, jack_midi_get_event_count, (void* port_buffer), (port_buffer));
JUCE_DECL_JACK_FUNCTION (int, jack_midi_event_get, (jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index), (event, port_buffer, event_index));

#if JUCE_DEBUG
 #define JACK_LOGGING_ENABLED 1
#endif

#if JACK_LOGGING_ENABLED
namespace
{
    void jack_Log (const String& s)
    {
        std::cerr << s << std::endl;
    }

    const char* getJackErrorMessage (const jack_status_t status)
    {
        if (status & JackServerFailed
             || status & JackServerError)   return "Unable to connect to JACK server";
        if (status & JackVersionError)      return "Client's protocol version does not match";
        if (status & JackInvalidOption)     return "The operation contained an invalid or unsupported option";
        if (status & JackNameNotUnique)     return "The desired client name was not unique";
        if (status & JackNoSuchClient)      return "Requested client does not exist";
        if (status & JackInitFailure)       return "Unable to initialize client";
        return nullptr;
    }
}
 #define JUCE_JACK_LOG_STATUS(x)    { if (const char* m = getJackErrorMessage (x)) jack_Log (m); }
 #define JUCE_JACK_LOG(x)           jack_Log(x)
#else
 #define JUCE_JACK_LOG_STATUS(x)    {}
 #define JUCE_JACK_LOG(x)           {}
#endif


//==============================================================================
#ifndef JUCE_JACK_CLIENT_NAME
 #define JUCE_JACK_CLIENT_NAME "JUCEJack"
#endif

//==============================================================================
class JackAudioIODevice   : public AudioIODevice
{
public:
    JackAudioIODevice (const String& deviceName,
                       const String& jackClientName,
                       const bool jackAutoConnectEnabled,
                       const bool jackMidiEnabled)
        : AudioIODevice (deviceName, "JACK"),
          clientName (jackClientName),
          autoConnectEnabled (jackAutoConnectEnabled),
          midiEnabled (jackMidiEnabled),
          deviceIsOpen (false),
          client (nullptr),
          callback (nullptr),
          midiPortIn (nullptr),
          positionInfo (new jack_position_t),
          incomingMessageBufferSize (0),
          messageSampleOffset (0)
    {
        jassert (deviceName.isNotEmpty());

        jack_status_t status;
        client = juce::jack_client_open (clientName.toUTF8(), JackNoStartServer, &status);

        if (client == nullptr)
        {
            JUCE_JACK_LOG_STATUS (status);
        }
        else
        {
            juce::jack_set_error_function (errorCallback);

            if (midiEnabled)
            {
                midiPortIn = juce::jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
            }
        }
    }

    ~JackAudioIODevice()
    {
        close();
        if (client != nullptr)
        {
            juce::jack_client_close (client);
            client = nullptr;
        }
    }

    StringArray getOutputChannelNames() override         { return outputPortNames; }
    StringArray getInputChannelNames() override          { return inputPortNames; }

    Array<double> getAvailableSampleRates() override
    {
        Array<double> rates;

        if (client != nullptr)
            rates.add (juce::jack_get_sample_rate (client));

        return rates;
    }

    Array<int> getAvailableBufferSizes() override
    {
        Array<int> sizes;

        if (client != nullptr)
            sizes.add (juce::jack_get_buffer_size (client));

        return sizes;
    }

    int getDefaultBufferSize() override             { return getCurrentBufferSizeSamples(); }
    int getCurrentBufferSizeSamples() override      { return client != nullptr ? juce::jack_get_buffer_size (client) : 0; }
    double getCurrentSampleRate() override          { return client != nullptr ? juce::jack_get_sample_rate (client) : 0; }


    String open (const BigInteger& inputChannels, const BigInteger& outputChannels,
                 double /* sampleRate */, int /* bufferSizeSamples */) override
    {
        if (client == nullptr)
        {
            lastError = "JACK server is not running";
            return lastError;
        }

        lastError.clear();
        close();

        // Register input and output ports
        const int numInputs = inputChannels.countNumberOfSetBits();
        const int numOutputs = outputChannels.countNumberOfSetBits();

        for (int i=0; i < numInputs; ++i)
        {
            String portName = "in" + (i >= 2 ? "_" + String(i/2 + 1) : "") + String(i % 2 == 0 ? "_L" : "_R");

            jack_port_t* input = juce::jack_port_register (client, portName.toUTF8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

            inputPorts.add (input);
            inputPortNames.add (portName);
        }

        for (int i = 0; i < numOutputs; ++i)
        {
            String portName = "out" + (i >= 2 ? "_" + String(i/2 + 1) : "") + String(i % 2 == 0 ? "_L" : "_R");

            jack_port_t* output = juce::jack_port_register (client, portName.toUTF8(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

            outputPorts.add (output);
            outputPortNames.add (portName);
        }

        inChans.calloc (numInputs);
        outChans.calloc (numOutputs);

        // Activate client
        juce::jack_set_process_callback (client, processCallback, this);
        juce::jack_on_shutdown (client, shutdownCallback, this);
        juce::jack_activate (client);

        if (autoConnectEnabled)
        {
            const char** ports = juce::jack_get_ports (client, nullptr, nullptr, JackPortIsPhysical|JackPortIsInput);

            if (ports != nullptr)
            {
                for (int i=0; i < outputPorts.size() && ports[i] != nullptr; ++i)
                {
                    juce::jack_connect (client, juce::jack_port_name (outputPorts[i]), ports[i]);
                }

                ::free (ports);
            }
        }

        deviceIsOpen = true;

        return lastError;
    }

    void close() override
    {
        stop();

        if (client != nullptr)
        {
            juce::jack_deactivate (client);
            juce::jack_set_process_callback (client, processCallback, nullptr);
            juce::jack_on_shutdown (client, shutdownCallback, nullptr);
        }

        deviceIsOpen = false;

        Jack::g_currentBPM = 0.0;

        inChans.free();
        outChans.free();
    }

    void start (AudioIODeviceCallback* newCallback) override
    {
        if (deviceIsOpen && newCallback != callback)
        {
            if (newCallback != nullptr)
                newCallback->audioDeviceAboutToStart (this);

            AudioIODeviceCallback* const oldCallback = callback;

            {
                const ScopedLock sl (callbackLock);
                callback = newCallback;
            }

            if (oldCallback != nullptr)
                oldCallback->audioDeviceStopped();
        }
    }

    void stop() override
    {
        start (nullptr);
    }

    bool isOpen() override                           { return deviceIsOpen; }
    bool isPlaying() override                        { return callback != nullptr; }
    int getCurrentBitDepth() override                { return 32; }
    String getLastError() override                   { return lastError; }

    BigInteger getActiveOutputChannels() const override
    {
        BigInteger outputBits;
        outputBits.setRange(0, outputPorts.size(), true);
        return outputBits;
    }

    BigInteger getActiveInputChannels()  const override
    {
        BigInteger inputBits;
        inputBits.setRange(0, inputPorts.size(), true);
        return inputBits;
    }

    int getOutputLatencyInSamples() override
    {
        int latency = 0;

        for (int i = 0; i < outputPorts.size(); i++)
            latency = jmax (latency, (int) juce::jack_port_get_total_latency (client, (jack_port_t*) outputPorts [i]));

        return latency;
    }

    int getInputLatencyInSamples() override
    {
        int latency = 0;

        for (int i = 0; i < inputPorts.size(); i++)
            latency = jmax (latency, (int) juce::jack_port_get_total_latency (client, (jack_port_t*) inputPorts [i]));

        return latency;
    }

    bool canFillMidiBuffer() const override
    {
        return midiEnabled;
    }

    void fillMidiBuffer (MidiBuffer& bufferToFill, const int numSamples) override
    {
        //bufferToFill = incomingMessages;

        JUCE_JACK_LOG ("Fill MIDI Buffer Started!  numSamples: " + String(numSamples));

        incomingMessageBufferSize = numSamples;

        const uint8* midiData;
        int numBytes, samplePosition;
        MidiBuffer tempBuffer;

        MidiBuffer::Iterator iter (incomingMessages);

        while (iter.getNextEvent (midiData, numBytes, samplePosition))
        {
            if (samplePosition < numSamples)
            {
                bufferToFill.addEvent (midiData, numBytes, samplePosition);
            }
            else
            {
                tempBuffer.addEvent (midiData, numBytes, samplePosition - numSamples);
            }
        }

        messageSampleOffset -= jmin (numSamples, messageSampleOffset);

        incomingMessages.clear();
        incomingMessages = tempBuffer;

        JUCE_JACK_LOG ("Fill MIDI Buffer Finished!  Message Sample Offset: " + String (messageSampleOffset));
    }

    String inputId, outputId;

private:
    void process (const int numSamples)
    {
        juce::jack_transport_query (client, positionInfo);
        Jack::g_currentBPM = positionInfo->beats_per_minute;

        if (midiPortIn != nullptr && incomingMessageBufferSize > 0)
        {
            JUCE_JACK_LOG ("Process MIDI started!  numSamples: " + String(numSamples));

            void* buffer = juce::jack_port_get_buffer (midiPortIn, numSamples);
            jack_nframes_t numEvents = juce::jack_midi_get_event_count (buffer);
            jack_midi_event_t midiEvent;

            for (jack_nframes_t i = 0; i < numEvents; ++i)
            {
                juce::jack_midi_event_get (&midiEvent, buffer, i);

                incomingMessages.addEvent (midiEvent.buffer, midiEvent.size, midiEvent.time + messageSampleOffset);

                JUCE_JACK_LOG ("JACK MIDI event!  Size: " + String (midiEvent.size) +
                               "  Sample Position: " + String (midiEvent.time + messageSampleOffset));
            }

            messageSampleOffset += numSamples;

            JUCE_JACK_LOG ("Process MIDI Finished!  Message Sample Offset: " + String (messageSampleOffset));
        }

        int numInputPorts = inputPorts.size();
        int numOutputPorts = outputPorts.size();

        for (int i = 0; i < numInputPorts; ++i)
        {
            if (jack_default_audio_sample_t* in
                    = (jack_default_audio_sample_t*) juce::jack_port_get_buffer (inputPorts.getUnchecked(i), numSamples))
                inChans [i] = (float*) in;
        }

        for (int i = 0; i < numOutputPorts; ++i)
        {
            if (jack_default_audio_sample_t* out
                    = (jack_default_audio_sample_t*) juce::jack_port_get_buffer (outputPorts.getUnchecked(i), numSamples))
                outChans [i] = (float*) out;
        }

        const ScopedLock sl (callbackLock);

        if (callback != nullptr)
        {
            callback->audioDeviceIOCallback (const_cast<const float**> (inChans.getData()), numInputPorts,
                                             outChans, numOutputPorts, numSamples);
        }
        else
        {
            for (int i = 0; i < numOutputPorts; ++i)
                zeromem (outChans[i], sizeof (float) * numSamples);
        }
    }

    static int processCallback (jack_nframes_t nframes, void* callbackArgument)
    {
        if (callbackArgument != nullptr)
            ((JackAudioIODevice*) callbackArgument)->process (nframes);

        return 0;
    }

    static void threadInitCallback (void* /* callbackArgument */)
    {
        JUCE_JACK_LOG ("JackAudioIODevice::initialise");
    }

    static void shutdownCallback (void* callbackArgument)
    {
        JUCE_JACK_LOG ("JackAudioIODevice::shutdown");

        if (JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument)
        {
            device->client = nullptr;
            device->close();
        }
    }

    static void errorCallback (const char* msg)
    {
        JUCE_JACK_LOG ("JackAudioIODevice::errorCallback " + String (msg));
    }

    const String clientName;
    const bool autoConnectEnabled;
    const bool midiEnabled;

    bool deviceIsOpen;
    jack_client_t* client;
    String lastError;
    AudioIODeviceCallback* callback;
    CriticalSection callbackLock;

    HeapBlock<float*> inChans, outChans;
    Array<jack_port_t*> inputPorts, outputPorts;
    jack_port_t* midiPortIn;

    StringArray inputPortNames;
    StringArray outputPortNames;

    ScopedPointer<jack_position_t> positionInfo;

    MidiBuffer incomingMessages;
    int incomingMessageBufferSize;
    int messageSampleOffset;
};


//==============================================================================
class JackAudioIODeviceType  : public AudioIODeviceType
{
public:
    JackAudioIODeviceType()
        : AudioIODeviceType ("JACK"),
          hasScanned (false)
    {
        deviceNames.add ("JACK Audio + MIDI (auto-connect outputs)");
        deviceNames.add ("JACK Audio Only (auto-connect outputs)");
        deviceNames.add ("JACK Audio + MIDI");
        deviceNames.add ("JACK Audio Only");
    }

    ~JackAudioIODeviceType()
    {
    }

    void scanForDevices()
    {
        hasScanned = true;
    }

    StringArray getDeviceNames (bool /* wantInputNames */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return deviceNames;
    }

    int getDefaultDeviceIndex (bool /* forInput */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this
        return 0;
    }

    bool hasSeparateInputsAndOutputs() const    { return false; }

    int getIndexOfDevice (AudioIODevice* device, bool /* asInput */) const
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        if (device == nullptr)
            return -1;

        return deviceNames.indexOf (device->getName());
    }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& /* inputDeviceName */)
    {
        jassert (hasScanned); // need to call scanForDevices() before doing this

        String clientName;

        if (!Jack::g_clientId.isEmpty())
        {
            clientName = Jack::g_clientId.toLocal8Bit().data();
        }
        else
        {
            clientName = JUCE_JACK_CLIENT_NAME;
        }

        bool isAutoConnectEnabled = outputDeviceName.contains ("auto-connect");
        bool isMidiEnabled = outputDeviceName.contains ("MIDI");

        return new JackAudioIODevice (outputDeviceName, clientName, isAutoConnectEnabled, isMidiEnabled);
    }

private:
    StringArray deviceNames;
    bool hasScanned;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JackAudioIODeviceType)
};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()
{
    return (juce_libjackHandle != nullptr) ? new JackAudioIODeviceType() : nullptr;
}
