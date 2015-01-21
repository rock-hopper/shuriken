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

#include "rubberbandaudiosource.h"
#include "globals.h"
#include <QDebug>


//==================================================================================================
// Public:

RubberbandAudioSource::RubberbandAudioSource( SamplerAudioSource* const source,
                                              const int numChans,
                                              const RubberBandStretcher::Options options,
                                              const bool isJackSyncEnabled ) :
    QObject(),
    AudioSource(),
    m_source( source ),
    m_numChans( numChans ),
    m_options( RubberBandStretcher::OptionProcessRealTime | options ),
    m_stretcher( NULL ),
    m_inputBuffer( numChans, 8192 ),
    m_globalTimeRatio( 1.0 ),
    m_prevGlobalTimeRatio( 1.0 ),
    m_noteTimeRatio( 1.0 ),
    m_pitchScale( 1.0 ),
    m_prevPitchScale( 1.0 ),
    m_isPitchCorrectionEnabled( true ),
    m_transientsOption( 0 ),
    m_prevTransientsOption( 0 ),
    m_phaseOption( 0 ),
    m_prevPhaseOption( 0 ),
    m_formantOption( 0 ),
    m_prevFormantOption( 0 ),
    m_pitchOption( 0 ),
    m_prevPitchOption( 0 ),
    m_originalBPM( 0.0 ),
    m_isJackSyncEnabled( isJackSyncEnabled )
{
    m_samplePositions.resize( 20 );
    m_noteTimeRatios.resize( 20 );
    m_inputBufferPtrs.resize( numChans );
}



RubberbandAudioSource::~RubberbandAudioSource()
{
    releaseResources();
}



void RubberbandAudioSource::prepareToPlay( int samplesPerBlockExpected, double sampleRate )
{
    if ( m_stretcher == NULL )
    {
        m_stretcher = new RubberBandStretcher( sampleRate, m_numChans, m_options );
        m_stretcher->reset();

        m_noteTimeRatio = 1.0;
        m_prevGlobalTimeRatio = 1.0;
        m_prevPitchScale = 1.0;
        m_prevTransientsOption = 0;
        m_prevPhaseOption = 0;
        m_prevFormantOption = 0;
        m_prevPitchOption = 0;
    }

    m_source->prepareToPlay( samplesPerBlockExpected, sampleRate );
}



void RubberbandAudioSource::releaseResources()
{
    if ( m_stretcher != NULL )
    {
        delete m_stretcher;
        m_stretcher = NULL;
    }

    m_source->releaseResources();
}



void RubberbandAudioSource::getNextAudioBlock( const AudioSourceChannelInfo& info )
{
    // JACK Sync
    if ( m_isJackSyncEnabled && Jack::g_currentBPM > 0.0 && m_originalBPM > 0.0 )
    {
        m_globalTimeRatio = m_originalBPM / Jack::g_currentBPM;
    }

    // Time ratio
    if ( m_globalTimeRatio != m_prevGlobalTimeRatio)
    {
        m_stretcher->setTimeRatio( m_globalTimeRatio * m_noteTimeRatio );
        m_prevGlobalTimeRatio = m_globalTimeRatio;
    }

    // Pitch scale
    if ( m_isPitchCorrectionEnabled )
    {
        m_pitchScale = 1.0;
    }
    else
    {
        if ( m_globalTimeRatio > 0.0 ) m_pitchScale = 1 / m_globalTimeRatio;
    }

    if ( m_pitchScale != m_prevPitchScale)
    {
        m_stretcher->setPitchScale( m_pitchScale );
        m_prevPitchScale = m_pitchScale;
    }

    // Options
    if ( m_transientsOption != m_prevTransientsOption )
    {
        m_stretcher->setTransientsOption( m_transientsOption );
        m_prevTransientsOption = m_transientsOption;
    }

    if ( m_phaseOption != m_prevPhaseOption )
    {
        m_stretcher->setPhaseOption( m_phaseOption );
        m_prevPhaseOption = m_phaseOption;
    }

    if ( m_formantOption != m_prevFormantOption )
    {
        m_stretcher->setFormantOption( m_formantOption );
        m_prevFormantOption = m_formantOption;
    }

    if ( m_pitchOption != m_prevPitchOption )
    {
        m_stretcher->setPitchOption( m_pitchOption );
        m_prevPitchOption = m_pitchOption;
    }

//    const int latency = m_stretcher->getLatency() + m_reserveSize;
//    std::cerr << "latency = " << latency << std::endl;

    while ( m_stretcher->available() < info.numSamples )
    {
        processNextAudioBlock();
    }

    m_stretcher->retrieve( info.buffer->getArrayOfWritePointers(), info.numSamples );
}



//==================================================================================================
// Private:

void RubberbandAudioSource::processNextAudioBlock()
{
    const int numRequired = m_stretcher->getSamplesRequired();

    if ( numRequired > 0 )
    {
        AudioSourceChannelInfo info;
        info.buffer = &m_inputBuffer;
        info.startSample = 0;
        info.numSamples = qMin( m_inputBuffer.getNumFrames(), numRequired );

        MidiBuffer midiBuffer;

        m_source->getNextAudioBlock( info, midiBuffer );

        if ( midiBuffer.isEmpty() )
        {
            m_stretcher->process( m_inputBuffer.getArrayOfReadPointers(), info.numSamples, false );
        }
        else
        {
            int numEvents = 0;

            {
                int samplePos;
                MidiMessage midiMessage;

                int i = 0;
                MidiBuffer::Iterator iterator( midiBuffer );

                while ( iterator.getNextEvent( midiMessage, samplePos ) )
                {
                    if ( midiMessage.isNoteOn() )
                    {
                        const qreal noteTimeRatio = m_noteTimeRatioTable.value( midiMessage.getNoteNumber(), 1.0 );

                        m_samplePositions.setUnchecked( i, samplePos );
                        m_noteTimeRatios.setUnchecked( i, noteTimeRatio );

                        i++;
                    }
                }

                numEvents = i;
            }

            // Process tail-end of previous note
            if ( m_samplePositions.getFirst() > 0 )
            {
                m_stretcher->process( m_inputBuffer.getArrayOfReadPointers(), m_samplePositions.getFirst(), false );
            }

            // Set note-specific time ratio and start processing any new note
            for ( int i = 0; i < numEvents; i++ )
            {
                m_noteTimeRatio = m_noteTimeRatios.getUnchecked( i );
                m_stretcher->setTimeRatio( m_globalTimeRatio * m_noteTimeRatio );

                const int samplePos = m_samplePositions.getUnchecked( i );

                const int numSamples = i + 1 < numEvents ?
                                       m_samplePositions.getUnchecked( i + 1 ) - samplePos :
                                       info.numSamples - samplePos;

                for ( int chanNum = 0; chanNum < m_numChans; chanNum++ )
                {
                    m_inputBufferPtrs.setUnchecked( chanNum, m_inputBuffer.getReadPointer( chanNum, samplePos ) );
                }

                m_stretcher->process( m_inputBufferPtrs.getRawDataPointer(), numSamples, false );
            }
        }
    }
    else // numRequired == 0
    {
        m_stretcher->process( m_inputBuffer.getArrayOfReadPointers(), 0, false );
    }
}
