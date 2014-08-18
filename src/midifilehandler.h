#ifndef MIDIFILEHANDLER_H
#define MIDIFILEHANDLER_H

#include "JuceHeader.h"
#include "samplebuffer.h"


class MidiFileHandler
{
public:
    static bool SaveMidiFile( const QString filePath,
                              const QList<SharedSampleRange> sampleRangeList,
                              const qreal sampleRate,
                              const qreal bpm,
                              const int midiFileType );

    static const int SECONDS_PER_MINUTE = 60;
    static const int MICROSECS_PER_MINUTE = 60000000;
};

#endif // MIDIFILEHANDLER_H
