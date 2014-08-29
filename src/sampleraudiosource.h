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

#ifndef SAMPLERAUDIOSOURCE_H
#define SAMPLERAUDIOSOURCE_H

#include "JuceHeader.h"
#include "samplebuffer.h"


class SamplerAudioSource : public AudioSource
{
public:
    SamplerAudioSource();
    ~SamplerAudioSource();

    void setSample( const SharedSampleBuffer sampleBuffer, const qreal sampleRate );
    void setSampleRanges( const QList<SharedSampleRange> sampleRangeList );

    void playRange( const SharedSampleRange sampleRange );
    void playAll();
    void stop();
    void enableLooping( const bool isEnabled )          { mIsLoopingEnabled = isEnabled; }

    MidiMessageCollector* getMidiMessageCollector()     { return &mMidiCollector; }

    // For JUCE use only!
    void prepareToPlay( int /*samplesPerBlockExpected*/, double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill ) override;



private:
    bool addNewSample( const SharedSampleBuffer sampleBuffer,
                       const SharedSampleRange sampleRange,
                       const qreal sampleRate );
    void clearSampleRanges();

    SharedSampleBuffer mSampleBuffer;
    QList<SharedSampleRange> mSampleRangeList;

    qreal mFileSampleRate;
    volatile qreal mPlaybackSampleRate;

    MidiMessageCollector mMidiCollector;
    Synthesiser mSampler;

    int mNextFreeKey;
    int mStartKey;

    volatile bool mIsPlaySeqEnabled;
    volatile bool mIsLoopingEnabled;
    volatile int mNoteCounter;
    volatile int mFrameCounter;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( SamplerAudioSource );
};

#endif // SAMPLERAUDIOSOURCE_H
