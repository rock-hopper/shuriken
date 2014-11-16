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

  All modifications to this file by Andrew M Taylor <a.m.taylor303@gmail.com>, 2014

  All modifications to the original file are released into the public domain.
  Please read UNLICENSE for more details, or refer to <http://unlicense.org/>

*/

#if JUCE_ALSA

// You can define these strings in your app if you want to override the default names:
#ifndef JUCE_ALSA_MIDI_INPUT_NAME
 #define JUCE_ALSA_MIDI_INPUT_NAME  "Juce Midi Input"
#endif

#ifndef JUCE_ALSA_MIDI_OUTPUT_NAME
 #define JUCE_ALSA_MIDI_OUTPUT_NAME "Juce Midi Output"
#endif

#include "linux_midi.h"

//==============================================================================
//namespace
//{

//==============================================================================

AlsaMidiClient::AlsaMidiClient (bool forInput) : LinuxMidiClient (forInput), handle (nullptr)
{
    snd_seq_open (&handle, "default", forInput ? SND_SEQ_OPEN_INPUT
                                                   : SND_SEQ_OPEN_OUTPUT, 0);
}

AlsaMidiClient::~AlsaMidiClient()
{
    if (handle != nullptr)
    {
        snd_seq_close (handle);
        handle = nullptr;
    }

    jassert (activeCallbacks.size() == 0);

    if (inputThread)
    {
        inputThread->stopThread (3000);
        inputThread = nullptr;
    }
}

void AlsaMidiClient::setName (const String& name)
{
    snd_seq_set_client_name (handle, name.toUTF8());
}

void AlsaMidiClient::registerCallback (MidiPortAndCallback* cb)
{
    if (cb != nullptr)
    {
        {
            const ScopedLock sl (callbackLock);
            activeCallbacks.add (cb);

            if (inputThread == nullptr)
                inputThread = new MidiInputThread (*this);
        }

        inputThread->startThread();
    }
}

void AlsaMidiClient::unregisterCallback (MidiPortAndCallback* cb)
{
    const ScopedLock sl (callbackLock);

    jassert (activeCallbacks.contains (cb));
    activeCallbacks.removeAllInstancesOf (cb);

    if (activeCallbacks.size() == 0 && inputThread->isThreadRunning())
        inputThread->signalThreadShouldExit();
}

snd_seq_t* AlsaMidiClient::getSeqHandle() const noexcept     { return handle; }


static AlsaMidiClient::Ptr globalAlsaSequencerIn()
{
    static AlsaMidiClient::Ptr global (new AlsaMidiClient (true));
    return global;
}

static AlsaMidiClient::Ptr globalAlsaSequencerOut ()
{
    static AlsaMidiClient::Ptr global (new AlsaMidiClient (false));
    return global;
}

static AlsaMidiClient::Ptr globalAlsaSequencer(bool input)
{
    return input ? globalAlsaSequencerIn()
                 : globalAlsaSequencerOut();
}



//==============================================================================

JackMidiClient::JackMidiClient (bool forInput) : LinuxMidiClient (forInput)
{
}

JackMidiClient::~JackMidiClient()
{
    jassert (activeCallback == nullptr);
}

void JackMidiClient::registerCallback (MidiPortAndCallback* cb)
{
    if (cb != nullptr)
    {
        activeCallback = cb;
        g_jackMidiClient = this;
    }
}

void JackMidiClient::unregisterCallback (MidiPortAndCallback* cb)
{
    jassert (activeCallback == cb);
    activeCallback = nullptr;

    g_jackMidiClient = nullptr;
}


static JackMidiClient::Ptr globalJackSequencerIn()
{
    static JackMidiClient::Ptr global (new JackMidiClient (true));
    return global;
}

static JackMidiClient::Ptr globalJackSequencerOut ()
{
    static JackMidiClient::Ptr global (new JackMidiClient (false));
    return global;
}

static JackMidiClient::Ptr globalJackSequencer(bool input)
{
    return input ? globalJackSequencerIn()
                 : globalJackSequencerOut();
}



//==============================================================================
// represents an input or output port of the supplied LinuxMidiClient
class LinuxMidiPort
{
public:
    enum MidiPortType { ALSA_MIDI_PORT, JACK_MIDI_PORT };

    LinuxMidiPort() noexcept  : portId (-1)  {}
    LinuxMidiPort (const LinuxMidiClient::Ptr& c, int port) noexcept  : client (c), portId (port) {}

    void createAlsaPort (const LinuxMidiClient::Ptr& c, const String& name, bool forInput)
    {
        portType = ALSA_MIDI_PORT;
        client = c;

        if (snd_seq_t* handle = (dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle())
            portId = snd_seq_create_simple_port (handle, name.toUTF8(),
                                                 forInput ? (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)
                                                          : (SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ),
                                                 SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    }

    void createJackPort (const LinuxMidiClient::Ptr& c)
    {
        portType = JACK_MIDI_PORT;
        client = c;
    }

    void deletePort()
    {
        if (isValid())
        {
            if (portType == ALSA_MIDI_PORT)
            {
                snd_seq_delete_simple_port ((dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle(), portId);
                portId = -1;
            }
        }
    }

    void connectWith (int sourceClient, int sourcePort)
    {
        if (portType == ALSA_MIDI_PORT)
        {
            if (client->isInput())
                snd_seq_connect_from ((dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle(), portId, sourceClient, sourcePort);
            else
                snd_seq_connect_to ((dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle(), portId, sourceClient, sourcePort);
        }
    }

    bool isValid() const noexcept
    {
        if (portType == ALSA_MIDI_PORT)
        {
            return client != nullptr && (dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle() != nullptr && portId >= 0;
        }
        else // (portType == JACK_MIDI_PORT)
        {
            return client != nullptr;
        }
    }

    MidiPortType getType()
    {
        return portType;
    }


    LinuxMidiClient::Ptr client;
    int portId;

private:
    MidiPortType portType;
};



//==============================================================================
class MidiPortAndCallback
{
public:
    MidiPortAndCallback (LinuxMidiPort p, MidiInput* in, MidiInputCallback* cb)
        : port (p), midiInput (in), callback (cb), callbackEnabled (false)
    {
    }

    ~MidiPortAndCallback()
    {
        enableCallback (false);
        port.deletePort();
    }

    void enableCallback (bool enable)
    {
        if (callbackEnabled != enable)
        {
            callbackEnabled = enable;

            if (enable)
            {
                port.client->registerCallback (this);
            }
            else
            {
                port.client->unregisterCallback (this);
            }
        }
    }

    void handleIncomingMidiMessage (const MidiMessage& message) const
    {
        callback->handleIncomingMidiMessage (midiInput, message);
    }

private:
    LinuxMidiPort port;
    MidiInput* midiInput;
    MidiInputCallback* callback;
    bool callbackEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiPortAndCallback)
};



void AlsaMidiClient::handleIncomingMidiMessage (const MidiMessage& message, int port)
{
    const ScopedLock sl (callbackLock);

    if (MidiPortAndCallback* const cb = activeCallbacks[port])
        cb->handleIncomingMidiMessage (message);
}



void JackMidiClient::handleIncomingMidiMessage (const MidiMessage& message, int /*port*/)
{
    if (activeCallback != nullptr)
        activeCallback->handleIncomingMidiMessage (message);
}



//==============================================================================
static LinuxMidiPort iterateMidiClient (const LinuxMidiClient::Ptr& seq,
                                   snd_seq_client_info_t* clientInfo,
                                   const bool forInput,
                                   StringArray& deviceNamesFound,
                                   const int deviceIndexToOpen)
{
    LinuxMidiPort port;

    snd_seq_t* seqHandle = (dynamic_cast<AlsaMidiClient*> (seq.getObject()))->getSeqHandle();
    snd_seq_port_info_t* portInfo = nullptr;

    if (snd_seq_port_info_malloc (&portInfo) == 0)
    {
        int numPorts = snd_seq_client_info_get_num_ports (clientInfo);
        const int client = snd_seq_client_info_get_client (clientInfo);

        snd_seq_port_info_set_client (portInfo, client);
        snd_seq_port_info_set_port (portInfo, -1);

        while (--numPorts >= 0)
        {
            if (snd_seq_query_next_port (seqHandle, portInfo) == 0
                && (snd_seq_port_info_get_capability (portInfo) & (forInput ? SND_SEQ_PORT_CAP_READ
                                                                            : SND_SEQ_PORT_CAP_WRITE)) != 0)
            {
                const String deviceName = snd_seq_client_info_get_name (clientInfo);
                deviceNamesFound.add (deviceName);

                if (deviceNamesFound.size() == deviceIndexToOpen + 1)
                {
                    if (deviceName == "jackmidi" || deviceName == "a2jmidid")
                    {
                        const LinuxMidiClient::Ptr client (globalJackSequencer (forInput));
                        port.createJackPort (client);
                    }
                    else
                    {
                        const int sourcePort   = snd_seq_port_info_get_port (portInfo);
                        const int sourceClient = snd_seq_client_info_get_client (clientInfo);

                        if (sourcePort != -1)
                        {
                            const String name (forInput ? JUCE_ALSA_MIDI_INPUT_NAME
                                                        : JUCE_ALSA_MIDI_OUTPUT_NAME);
                            (dynamic_cast<AlsaMidiClient*> (seq.getObject()))->setName (APPLICATION_NAME);
                            port.createAlsaPort (seq, name, forInput);
                            port.connectWith (sourceClient, sourcePort);
                        }
                    }
                }
            }
        }

        snd_seq_port_info_free (portInfo);
    }

    return port;
}

static LinuxMidiPort iterateMidiDevices (const bool forInput,
                                    StringArray& deviceNamesFound,
                                    const int deviceIndexToOpen)
{
    LinuxMidiPort port;
    const LinuxMidiClient::Ptr client (globalAlsaSequencer (forInput));

    if (snd_seq_t* const seqHandle = (dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle())
    {
        snd_seq_system_info_t* systemInfo = nullptr;
        snd_seq_client_info_t* clientInfo = nullptr;

        if (snd_seq_system_info_malloc (&systemInfo) == 0)
        {
            if (snd_seq_system_info (seqHandle, systemInfo) == 0
                 && snd_seq_client_info_malloc (&clientInfo) == 0)
            {
                int numClients = snd_seq_system_info_get_cur_clients (systemInfo);

                while (--numClients >= 0 && ! port.isValid())
                    if (snd_seq_query_next_client (seqHandle, clientInfo) == 0)
                        port = iterateMidiClient (client, clientInfo, forInput,
                                                  deviceNamesFound, deviceIndexToOpen);

                snd_seq_client_info_free (clientInfo);
            }

            snd_seq_system_info_free (systemInfo);
        }

    }

    deviceNamesFound.appendNumbersToDuplicates (true, true);

    return port;
}

LinuxMidiPort createMidiDevice (const bool forInput, const String& deviceNameToOpen)
{
    LinuxMidiPort port;
    LinuxMidiClient::Ptr client (new AlsaMidiClient (forInput));

    if ((dynamic_cast<AlsaMidiClient*> (client.getObject()))->getSeqHandle())
    {
        (dynamic_cast<AlsaMidiClient*> (client.getObject()))->setName (deviceNameToOpen + (forInput ? " Input" : " Output"));
        port.createAlsaPort (client, forInput ? "in" : "out", forInput);
    }

    return port;
}



//==============================================================================
class MidiOutputDevice
{
public:
    MidiOutputDevice (MidiOutput* const output, const LinuxMidiPort& p)
        : midiOutput (output), port (p),
          maxEventSize (16 * 1024)
    {
        jassert (port.isValid() && midiOutput != nullptr);
        snd_midi_event_new (maxEventSize, &midiParser);
    }

    ~MidiOutputDevice()
    {
        snd_midi_event_free (midiParser);
        port.deletePort();
    }

    void sendMessageNow (const MidiMessage& message)
    {
        if (message.getRawDataSize() > maxEventSize)
        {
            maxEventSize = message.getRawDataSize();
            snd_midi_event_free (midiParser);
            snd_midi_event_new (maxEventSize, &midiParser);
        }

        snd_seq_event_t event;
        snd_seq_ev_clear (&event);

        long numBytes = (long) message.getRawDataSize();
        const uint8* data = message.getRawData();

        snd_seq_t* seqHandle = (dynamic_cast<AlsaMidiClient*> (port.client.getObject()))->getSeqHandle();

        while (numBytes > 0)
        {
            const long numSent = snd_midi_event_encode (midiParser, data, numBytes, &event);
            if (numSent <= 0)
                break;

            numBytes -= numSent;
            data += numSent;

            snd_seq_ev_set_source (&event, 0);
            snd_seq_ev_set_subs (&event);
            snd_seq_ev_set_direct (&event);

            snd_seq_event_output (seqHandle, &event);
        }

        snd_seq_drain_output (seqHandle);
        snd_midi_event_reset_encode (midiParser);
    }

private:
    MidiOutput* const midiOutput;
    LinuxMidiPort port;
    snd_midi_event_t* midiParser;
    int maxEventSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiOutputDevice);
};

//} // namespace

StringArray MidiOutput::getDevices()
{
    StringArray devices;
    iterateMidiDevices (false, devices, -1);
    return devices;
}

int MidiOutput::getDefaultDeviceIndex()
{
    return 0;
}

MidiOutput* MidiOutput::openDevice (int deviceIndex)
{
    MidiOutput* newDevice = nullptr;

    StringArray devices;
    LinuxMidiPort port (iterateMidiDevices (false, devices, deviceIndex));

    if (port.isValid())
    {
        newDevice = new MidiOutput();
        newDevice->internal = new MidiOutputDevice (newDevice, port);
    }

    return newDevice;
}

MidiOutput* MidiOutput::createNewDevice (const String& deviceName)
{
    MidiOutput* newDevice = nullptr;

    LinuxMidiPort port (createMidiDevice (false, deviceName));

    if (port.isValid())
    {
        newDevice = new MidiOutput();
        newDevice->internal = new MidiOutputDevice (newDevice, port);
    }

    return newDevice;
}

MidiOutput::~MidiOutput()
{
    stopBackgroundThread();

    delete static_cast<MidiOutputDevice*> (internal);
}

void MidiOutput::sendMessageNow (const MidiMessage& message)
{
    static_cast<MidiOutputDevice*> (internal)->sendMessageNow (message);
}




//==============================================================================
MidiInput::MidiInput (const String& nm)
    : name (nm), internal (nullptr)
{
}

MidiInput::~MidiInput()
{
    stop();
    delete static_cast<MidiPortAndCallback*> (internal);
}

void MidiInput::start()
{
    static_cast<MidiPortAndCallback*> (internal)->enableCallback (true);
}

void MidiInput::stop()
{
    static_cast<MidiPortAndCallback*> (internal)->enableCallback (false);
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

StringArray MidiInput::getDevices()
{
    StringArray devices;
    iterateMidiDevices (true, devices, -1);
    return devices;
}

MidiInput* MidiInput::openDevice (int deviceIndex, MidiInputCallback* callback)
{
    MidiInput* newDevice = nullptr;

    StringArray devices;
    LinuxMidiPort port (iterateMidiDevices (true, devices, deviceIndex));

    if (port.isValid())
    {
        newDevice = new MidiInput (devices [deviceIndex]);
        newDevice->internal = new MidiPortAndCallback (port, newDevice, callback);
    }

    return newDevice;
}

MidiInput* MidiInput::createNewDevice (const String& deviceName, MidiInputCallback* callback)
{
    MidiInput* newDevice = nullptr;

    LinuxMidiPort port (createMidiDevice (true, deviceName));

    if (port.isValid())
    {
        newDevice = new MidiInput (deviceName);
        newDevice->internal = new MidiPortAndCallback (port, newDevice, callback);
    }

    return newDevice;
}


//==============================================================================
#else

// (These are just stub functions if ALSA is unavailable...)

StringArray MidiOutput::getDevices()                                { return StringArray(); }
int MidiOutput::getDefaultDeviceIndex()                             { return 0; }
MidiOutput* MidiOutput::openDevice (int)                            { return nullptr; }
MidiOutput* MidiOutput::createNewDevice (const String&)             { return nullptr; }
MidiOutput::~MidiOutput()   {}
void MidiOutput::sendMessageNow (const MidiMessage&)    {}

MidiInput::MidiInput (const String& nm) : name (nm), internal (nullptr)  {}
MidiInput::~MidiInput() {}
void MidiInput::start() {}
void MidiInput::stop()  {}
int MidiInput::getDefaultDeviceIndex()      { return 0; }
StringArray MidiInput::getDevices()         { return StringArray(); }
MidiInput* MidiInput::openDevice (int, MidiInputCallback*)                  { return nullptr; }
MidiInput* MidiInput::createNewDevice (const String&, MidiInputCallback*)   { return nullptr; }

#endif
