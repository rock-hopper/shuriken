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

#ifndef SAMPLERAUDIOSOURCE_H
#define SAMPLERAUDIOSOURCE_H

#include "JuceHeader.h"
#include "samplebuffer.h"


class SamplerAudioSource : public AudioSource
{
public:
    SamplerAudioSource();
    ~SamplerAudioSource();

    void setSamples( QList<SharedSampleBuffer> sampleBufferList, qreal sampleRate );

    void playSample( int sampleNum, SharedSampleRange sampleRange );
    void playAll();
    void stop();
    void setLooping( bool isLoopingDesired );

    void setAttack( int sampleNum, qreal value );   // Value should be 0.00 - 1.00
    void setRelease( int sampleNum, qreal value );  // Value should be 0.00 - 1.00

    qreal getAttack( int sampleNum ) const;
    qreal getRelease( int sampleNum ) const;

    void setOneShot( int sampleNum, bool set );
    bool isOneShotSet( int sampleNum ) const;

    struct EnvelopeSettings
    {
        QList<qreal> attackValues;      // Values in the range 0.00 - 1.00
        QList<qreal> releaseValues;     // Values in the range 0.00 - 1.00
        QList<bool> oneShotSettings;
    };

    void getEnvelopeSettings( EnvelopeSettings& settings ) const;
    void setEnvelopeSettings( const EnvelopeSettings& settings );

    int getLowestAssignedMidiNote() const           { return m_lowestAssignedNote; }

    MidiMessageCollector* getMidiInputCallback()    { return &m_midiCollector; }

    // For JUCE use only!
    void prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock( const AudioSourceChannelInfo& info ) override;
    void getNextAudioBlock( const AudioSourceChannelInfo& info, MidiBuffer& midiBuffer );

private:
    bool addNewSample( SharedSampleBuffer sampleBuffer, qreal sampleRate );
    void clearSamples();

    QList<SharedSampleBuffer> m_sampleBufferList;
    SharedSampleRange m_tempSampleRange;

    volatile qreal m_fileSampleRate;
    volatile qreal m_playbackSampleRate;

    MidiBuffer m_midiBuffer;
    MidiMessageCollector m_midiCollector;

    Synthesiser m_sampler;

    int m_nextFreeNote;
    volatile int m_lowestAssignedNote;

    volatile bool m_isPlaySeqEnabled;
    volatile bool m_isLoopingEnabled;
    volatile int m_seqStartNote;
    volatile int m_noteCounter;
    volatile int m_noteCounterEnd;
    volatile int m_frameCounter;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( SamplerAudioSource );
};

#endif // SAMPLERAUDIOSOURCE_H
