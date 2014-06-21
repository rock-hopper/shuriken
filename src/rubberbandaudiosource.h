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

#include <QObject>
#include "samplebuffer.h"
#include "JuceHeader.h"
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;


class RubberbandAudioSource : public QObject, public AudioSource
{
    Q_OBJECT

public:
    // Caller is responsible for deleting 'source' after RubberbandAudioSource has been deleted
    RubberbandAudioSource( AudioSource* const source,
                           const int numChans,
                           const RubberBandStretcher::Options options,
                           const bool isJackSyncEnabled = false );
    ~RubberbandAudioSource();

    void setTimeRatio( const qreal ratio )                                      { mTimeRatio = ratio; }
    void setPitchScale( const qreal scale )                                     { mPitchScale = scale; }
    void enablePitchCorrection( const bool isEnabled )                          { mIsPitchCorrectionEnabled = isEnabled; }

    // Only has an effect when JACK Sync is enabled
    void setOriginalBPM( const qreal bpm )                                      { mOriginalBPM = bpm; }

    void prepareToPlay( int samplesPerBlockExpected, double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill ) override;

private:
    void readNextBufferChunk();

    AudioSource* const mSource;
    const int mNumChans;
    const RubberBandStretcher::Options mOptions;

    RubberBandStretcher* mStretcher;

    SampleBuffer mInputBuffer;

    volatile qreal mTimeRatio;
    qreal mPrevTimeRatio;

    volatile qreal mPitchScale;
    qreal mPrevPitchScale;

    volatile bool mIsPitchCorrectionEnabled;

    volatile RubberBandStretcher::Options mTransientsOption;
    RubberBandStretcher::Options mPrevTransientsOption;

    volatile RubberBandStretcher::Options mPhaseOption;
    RubberBandStretcher::Options mPrevPhaseOption;

    volatile RubberBandStretcher::Options mFormantOption;
    RubberBandStretcher::Options mPrevFormantOption;

    volatile RubberBandStretcher::Options mPitchOption;
    RubberBandStretcher::Options mPrevPitchOption;

    volatile qreal mOriginalBPM;

    volatile bool mIsJackSyncEnabled;

    int mTotalNumRetrieved;

public slots:
    void setTransientsOption( const RubberBandStretcher::Options option )       { mTransientsOption = option; }
    void setPhaseOption( const RubberBandStretcher::Options option )            { mPhaseOption = option; }
    void setFormantOption( const RubberBandStretcher::Options option )          { mFormantOption = option; }
    void setPitchOption( const RubberBandStretcher::Options option )            { mPitchOption = option; }
    void enableJackSync( const bool isEnabled )                                 { mIsJackSyncEnabled = isEnabled; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( RubberbandAudioSource );
};


#endif // RUBBERBANDAUDIOSOURCE_H
