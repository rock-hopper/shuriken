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
    RubberbandAudioSource( AudioSource* source,
                           int numChans,
                           RubberBandStretcher::Options options,
                           bool isJackSyncEnabled = false );
    ~RubberbandAudioSource();

    void setTimeRatio( qreal ratio )                                      { m_timeRatio = ratio; }
    void setPitchScale( qreal scale )                                     { m_pitchScale = scale; }
    void enablePitchCorrection( bool isEnabled )                          { m_isPitchCorrectionEnabled = isEnabled; }

    // Only has an effect when JACK Sync is enabled
    void setOriginalBPM( qreal bpm )                                      { m_originalBPM = bpm; }

    void prepareToPlay( int samplesPerBlockExpected, double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock( const AudioSourceChannelInfo& bufferToFill ) override;

private:
    void readNextBufferChunk();

    AudioSource* const m_source;
    const int m_numChans;
    const RubberBandStretcher::Options m_options;

    RubberBandStretcher* m_stretcher;

    SampleBuffer m_inputBuffer;

    volatile qreal m_timeRatio;
    qreal m_prevTimeRatio;

    volatile qreal m_pitchScale;
    qreal m_prevPitchScale;

    volatile bool m_isPitchCorrectionEnabled;

    volatile RubberBandStretcher::Options m_transientsOption;
    RubberBandStretcher::Options m_prevTransientsOption;

    volatile RubberBandStretcher::Options m_phaseOption;
    RubberBandStretcher::Options m_prevPhaseOption;

    volatile RubberBandStretcher::Options m_formantOption;
    RubberBandStretcher::Options m_prevFormantOption;

    volatile RubberBandStretcher::Options m_pitchOption;
    RubberBandStretcher::Options m_prevPitchOption;

    volatile qreal m_originalBPM;

    volatile bool m_isJackSyncEnabled;

    int m_totalNumRetrieved;

public slots:
    void setTransientsOption( RubberBandStretcher::Options option )       { m_transientsOption = option; }
    void setPhaseOption( RubberBandStretcher::Options option )            { m_phaseOption = option; }
    void setFormantOption( RubberBandStretcher::Options option )          { m_formantOption = option; }
    void setPitchOption( RubberBandStretcher::Options option )            { m_pitchOption = option; }
    void enableJackSync( bool isEnabled )                                 { m_isJackSyncEnabled = isEnabled; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( RubberbandAudioSource );
};


#endif // RUBBERBANDAUDIOSOURCE_H
