#include "midifilehandler.h"
#include "globals.h"


bool MidiFileHandler::SaveMidiFile( const QString filePath,
                                    const QList<SharedSampleRange> sampleRangeList,
                                    const qreal sampleRate,
                                    const qreal bpm,
                                    const int midiFileType )
{
    Q_ASSERT( sampleRangeList.size() != 0 );
    Q_ASSERT( sampleRate > 0.0 );
    Q_ASSERT( bpm > 0.0 );


    File file( filePath.toLocal8Bit().data() );
    FileOutputStream fos( file );
    MidiFile midiFile;
    MidiMessageSequence midiSeq;

    const int microsecsPerQuartNote = roundToInt( MICROSECS_PER_MINUTE / bpm );
    const int chanNum = 1;

    midiFile.setTicksPerQuarterNote( Midi::TICKS_PER_QUART_NOTE );

    midiSeq.addEvent( MidiMessage::timeSignatureMetaEvent( 4, 4 ) );
    midiSeq.addEvent( MidiMessage::tempoMetaEvent( microsecsPerQuartNote ) );
    midiSeq.addEvent( MidiMessage::midiChannelMetaEvent( chanNum ) );

    int startNote = Midi::MIDDLE_C;

    if ( sampleRangeList.size() > Midi::MAX_POLYPHONY - Midi::MIDDLE_C )
    {
        startNote = qMax( Midi::MAX_POLYPHONY - sampleRangeList.size(), 0 );
    }

    const qreal secondsPerTick = ( SECONDS_PER_MINUTE / bpm ) / Midi::TICKS_PER_QUART_NOTE;
    const float velocity = 1.0f;

    int noteCounter = 0;
    int i = 0;

    while (  i < sampleRangeList.size() && i < Midi::MAX_POLYPHONY )
    {
        const qreal noteStart  = ( sampleRangeList.at( i )->startFrame / sampleRate ) / secondsPerTick;
        const qreal noteLength = ( sampleRangeList.at( i )->numFrames / sampleRate ) / secondsPerTick;

        midiSeq.addEvent( MidiMessage::noteOn( chanNum, startNote + noteCounter, velocity ),
                          roundToInt( noteStart ) );

        midiSeq.addEvent( MidiMessage::noteOff( chanNum, startNote + noteCounter ),
                          roundToInt( noteStart + noteLength ) );

        midiSeq.updateMatchedPairs();

        noteCounter++;
        i++;
    }

    midiSeq.addEvent( MidiMessage::endOfTrack(), midiSeq.getEndTime() );

    midiFile.addTrack( midiSeq );

    return midiFile.writeTo( fos );
}
