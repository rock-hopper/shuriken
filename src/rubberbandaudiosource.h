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
#include "JuceHeader.h"
#include "samplebuffer.h"
#include "sampleraudiosource.h"
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;


class RubberbandAudioSource : public QObject, public AudioSource
{
    Q_OBJECT

public:
    // Caller is responsible for deleting 'source' after RubberbandAudioSource has been deleted
    RubberbandAudioSource( SamplerAudioSource* source,
                           int numChans,
                           RubberBandStretcher::Options options,
                           bool isJackSyncEnabled = false );

    ~RubberbandAudioSource();

    void setGlobalTimeRatio( qreal ratio )                          { m_globalTimeRatio = ratio; }
    void setPitchScale( qreal scale )                               { m_pitchScale = scale; }
    void enablePitchCorrection( bool isEnabled )                    { m_isPitchCorrectionEnabled = isEnabled; }

    qreal getNoteTimeRatio( int midiNote ) const                    { return m_noteTimeRatioTable.value( midiNote, 1.0 ); }
    void setNoteTimeRatio( int midiNote, qreal ratio )              { m_noteTimeRatioTable.insert( midiNote, ratio ); }

    // Only has an effect when JACK Sync is enabled
    void setOriginalBPM( qreal bpm )                                { m_originalBPM = bpm; }

    // For JUCE use only!
    void prepareToPlay( int samplesPerBlockExpected, double sampleRate ) override;
    void releaseResources() override;
    void getNextAudioBlock( const AudioSourceChannelInfo& info ) override;

private:
    void processNextAudioBlock();

    SamplerAudioSource* const m_source;
    const int m_numChans;
    const RubberBandStretcher::Options m_options;

    RubberBandStretcher* m_stretcher;

    SampleBuffer m_inputBuffer;

    MidiBuffer m_midiBuffer;

    volatile qreal m_globalTimeRatio;
    qreal m_prevGlobalTimeRatio;
    qreal m_noteTimeRatio;

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

    QHash<int, qreal> m_noteTimeRatioTable;

    Array<int> m_samplePositions;
    Array<qreal> m_noteTimeRatios;

    Array<const float*> m_inputBufferPtrs;

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
