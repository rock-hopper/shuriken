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

#include "midifilehandler.h"
#include "globals.h"
#include <QDir>


//==================================================================================================
// Public Static:

bool MidiFileHandler::SaveMidiFile( const QString fileBaseName,
                                    const QString outputDirPath,
                                    const QList<SharedSampleBuffer> sampleBufferList,
                                    const int numSampleBuffers,
                                    const qreal sampleRate,
                                    const qreal bpm,
                                    const int timeSigNumerator,
                                    const int timeSigDenominator,
                                    const MidiFileType midiFileType,
                                    const bool isOverwriteEnabled )
{
    Q_ASSERT( ! sampleBufferList.isEmpty() );
    Q_ASSERT( numSampleBuffers <= sampleBufferList.size() );
    Q_ASSERT( sampleRate > 0.0 );
    Q_ASSERT( bpm > 0.0 );

    const QString filePath = QDir( outputDirPath ).absoluteFilePath( fileBaseName + getFileExtension() );

    File file( filePath.toLocal8Bit().data() );

    if ( file.exists() && ! isOverwriteEnabled )
    {
        return false;
    }

    file.deleteFile();

    FileOutputStream outputStream( file );
    MidiFile midiFile;
    MidiMessageSequence midiSeq;

    const int microsecsPerQuartNote = roundToIntAccurate( qreal(timeSigDenominator * MICROSECS_PER_MINUTE) / (4 * bpm) );
    const int chanNum = 1;

    midiFile.setTicksPerQuarterNote( TICKS_PER_QUART_NOTE );

    midiSeq.addEvent( MidiMessage::timeSignatureMetaEvent( timeSigNumerator, timeSigDenominator ) );
    midiSeq.addEvent( MidiMessage::tempoMetaEvent( microsecsPerQuartNote ) );

    if ( midiFileType == MIDI_FILE_TYPE_1 )
    {
        midiSeq.addEvent( MidiMessage::endOfTrack(), midiSeq.getEndTime() );

        midiFile.addTrack( midiSeq );

        midiSeq.clear();
    }

    midiSeq.addEvent( MidiMessage::midiChannelMetaEvent( chanNum ) );

    int startNote = Midi::MIDDLE_C;

    if ( numSampleBuffers > Midi::MAX_POLYPHONY - Midi::MIDDLE_C )
    {
        startNote = qMax( Midi::MAX_POLYPHONY - numSampleBuffers, 0 );
    }

    const qreal secondsPerTick = ( SECONDS_PER_MINUTE / bpm ) / TICKS_PER_QUART_NOTE;
    const float velocity = 1.0f;

    qreal noteStart = 0.0;
    int i = 0;

    while (  i < numSampleBuffers && i < Midi::MAX_POLYPHONY )
    {
        const qreal noteLength = ( sampleBufferList.at( i )->getNumFrames() / sampleRate ) / secondsPerTick;

        midiSeq.addEvent( MidiMessage::noteOn( chanNum, startNote + i, velocity ),
                          roundToIntAccurate( noteStart ) );

        midiSeq.addEvent( MidiMessage::noteOff( chanNum, startNote + i ),
                          roundToIntAccurate( noteStart + noteLength ) );

        midiSeq.updateMatchedPairs();

        noteStart += noteLength;
        i++;
    }

    midiSeq.addEvent( MidiMessage::endOfTrack(), midiSeq.getEndTime() );

    midiFile.addTrack( midiSeq );

    return midiFile.writeTo( outputStream );
}
