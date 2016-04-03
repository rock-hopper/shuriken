/*
  ==============================================================================

   This file contains code from the JUCE library.
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

  All modifications to the original code by Andrew M Taylor <a.m.taylor303@gmail.com>, 2014, 2016

  To the extent possible under law, Andrew M Taylor has waived all copyright
  and related or neighboring rights to the code modifications in this file.
  <https://creativecommons.org/publicdomain/zero/1.0/>
*/

#ifndef LINUX_MIDI_H
#define LINUX_MIDI_H


//==============================================================================

class MidiPortAndCallback;


//==============================================================================

class LinuxMidiClient : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<LinuxMidiClient> Ptr;

    LinuxMidiClient (bool forInput) : input (forInput) {}
    ~LinuxMidiClient() {}

    bool isInput() const noexcept    { return input; }

    virtual void registerCallback (MidiPortAndCallback* cb) = 0;
    virtual void unregisterCallback (MidiPortAndCallback* cb) = 0;
    virtual void handleIncomingMidiMessage (const MidiMessage& message, int port) = 0;

protected:
    bool input;
};



//==============================================================================

class AlsaMidiClient : public LinuxMidiClient
{
public:
    AlsaMidiClient (bool forInput);
    ~AlsaMidiClient();

    void setName (const String& name);
    snd_seq_t* getSeqHandle() const noexcept;
    void registerCallback (MidiPortAndCallback* cb);
    void unregisterCallback (MidiPortAndCallback* cb);
    void handleIncomingMidiMessage (const MidiMessage& message, int port);


private:
    Array<MidiPortAndCallback*> activeCallbacks;
    CriticalSection callbackLock;
    snd_seq_t* handle;

    //==============================================================================
    class MidiInputThread   : public Thread
    {
    public:
        MidiInputThread (AlsaMidiClient& c)
            : Thread ("Juce MIDI Input"), client (c)
        {
            jassert (client.input && client.getSeqHandle() != nullptr);
        }

        void run() override
        {
            const int maxEventSize = 16 * 1024;
            snd_midi_event_t* midiParser;
            snd_seq_t* seqHandle = client.getSeqHandle();

            if (snd_midi_event_new (maxEventSize, &midiParser) >= 0)
            {
                const int numPfds = snd_seq_poll_descriptors_count (seqHandle, POLLIN);
                HeapBlock<pollfd> pfd ((size_t) numPfds);
                snd_seq_poll_descriptors (seqHandle, pfd, (unsigned int) numPfds, POLLIN);

                HeapBlock<uint8> buffer (maxEventSize);

                while (! threadShouldExit())
                {
                    if (poll (pfd, (nfds_t) numPfds, 100) > 0) // there was a "500" here which is a bit long when we exit the program and have to wait for a timeout on this poll call
                    {
                        if (threadShouldExit())
                            break;

                        snd_seq_nonblock (seqHandle, 1);

                        do
                        {
                            snd_seq_event_t* inputEvent = nullptr;

                            if (snd_seq_event_input (seqHandle, &inputEvent) >= 0)
                            {
                                // xxx what about SYSEXes that are too big for the buffer?
                                const long numBytes = snd_midi_event_decode (midiParser, buffer,
                                                                             maxEventSize, inputEvent);

                                snd_midi_event_reset_decode (midiParser);

                                if (numBytes > 0)
                                {
                                    const MidiMessage message ((const uint8*) buffer, (int) numBytes,
                                                               Time::getMillisecondCounter() * 0.001);

                                    client.handleIncomingMidiMessage (message, inputEvent->dest.port);
                                }

                                snd_seq_free_event (inputEvent);
                            }
                        }
                        while (snd_seq_event_input_pending (seqHandle, 0) > 0);
                    }
                }

                snd_midi_event_free (midiParser);
            }
        };

    private:
        AlsaMidiClient& client;
    };

    ScopedPointer<MidiInputThread> inputThread;
};



//==============================================================================

class JackMidiClient : public LinuxMidiClient
{
public:
    JackMidiClient (bool forInput);
    ~JackMidiClient();

    void registerCallback (MidiPortAndCallback* cb);
    void unregisterCallback (MidiPortAndCallback* cb);
    void handleIncomingMidiMessage (const MidiMessage& message, int /*port*/);

private:
    MidiPortAndCallback* activeCallback;
};



//==============================================================================

// This gets set in:     juce_linux_Midi.cpp
//                       void JackMidiClient::registerCallback (MidiPortAndCallback* cb)
//                       void JackMidiClient::unregisterCallback (MidiPortAndCallback* cb)
//
// and gets called here: juce_linux_JackAudio.cpp
//                       void process (int numFrames)
//
static JackMidiClient* volatile g_jackMidiClient;


//==============================================================================

#endif // LINUX_MIDI_H
