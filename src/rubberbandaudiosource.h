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

#ifndef RUBBERBANDAUDIOSOURCE_H
#define RUBBERBANDAUDIOSOURCE_H

#include "samplebuffer.h"
#include "JuceHeader.h"
#include <rubberband/RubberBandStretcher.h>
#include <QtGlobal>

using namespace RubberBand;


class RubberbandAudioSource : public AudioSource
{
public:
    // Caller is responsible for deleting 'source' after RubberbandAudioSource has been deleted
    RubberbandAudioSource( AudioSource* const source, const int numChans );
    ~RubberbandAudioSource();

    void setRubberbandOptions( const RubberBandStretcher::Options options )     { mOptions = options; }
    RubberBandStretcher::Options getRubberbandOptions() const                   { return mOptions; }

    void setTimeRatio( const qreal ratio )                                      { mTimeRatio = ratio; }
    void setPitchScale( const qreal scale )                                     { mPitchScale = scale; }
    void enablePitchCorrection( const bool isEnabled )                          { mIsPitchCorrectionEnabled = isEnabled; }

    // Only has an effect when JACK Sync is enabled
    void setOriginalBPM( const qreal bpm )                                      { mOriginalBPM = bpm; }
    void enableJackSync( const bool isEnabled )                                 { mIsJackSyncEnabled = isEnabled; }

    void prepareToPlay( int samplesPerBlockExpected, double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill ) override;

private:
    void readNextBufferChunk();

    AudioSource* mSource;

    int mNumChans;

    RubberBandStretcher::Options mOptions;
    RubberBandStretcher* mStretcher;

    SampleBuffer mInputBuffer;

    volatile qreal mTimeRatio;
    qreal mPrevTimeRatio;

    volatile qreal mPitchScale;
    qreal mPrevPitchScale;

    volatile bool mIsPitchCorrectionEnabled;

    volatile qreal mOriginalBPM;

    volatile bool mIsJackSyncEnabled;

    int mTotalNumRetrieved;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( RubberbandAudioSource );
};


#endif // RUBBERBANDAUDIOSOURCE_H
