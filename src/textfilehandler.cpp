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

#include "textfilehandler.h"
#include "JuceHeader.h"
#include "messageboxes.h"
#include <QDir>


//==================================================================================================
// Public Static:

bool TextFileHandler::createProjectXmlFile( const QString filePath, const ProjectSettings& settings )
{
    XmlElement docElement( "project" );
    docElement.setAttribute( "name", settings.projectName.toLocal8Bit().data() );

    XmlElement* origBpmElement = new XmlElement( "original_bpm" );
    origBpmElement->setAttribute( "value", settings.originalBpm );
    docElement.addChildElement( origBpmElement );

    XmlElement* newBpmElement = new XmlElement( "new_bpm" );
    newBpmElement->setAttribute( "value", settings.newBpm );
    docElement.addChildElement( newBpmElement );

    XmlElement* appliedBpmElement = new XmlElement( "applied_bpm" );
    appliedBpmElement->setAttribute( "value", settings.appliedBpm );
    docElement.addChildElement( appliedBpmElement );

    XmlElement* timeStretchElement = new XmlElement( "time_stretch" );
    timeStretchElement->setAttribute( "checked", settings.isTimeStretchChecked );
    docElement.addChildElement( timeStretchElement );

    XmlElement* pitchCorrectionElement = new XmlElement( "pitch_correction" );
    pitchCorrectionElement->setAttribute( "checked", settings.isPitchCorrectionChecked );
    docElement.addChildElement( pitchCorrectionElement );

    XmlElement* stretchOptionsElement = new XmlElement( "stretch_options" );
    stretchOptionsElement->setAttribute( "value", settings.options );
    docElement.addChildElement( stretchOptionsElement );

    XmlElement* jackSyncElement = new XmlElement( "jack_sync" );
    jackSyncElement->setAttribute( "checked", settings.isJackSyncChecked );
    docElement.addChildElement( jackSyncElement );

    XmlElement* numeratorElement = new XmlElement( "time_sig_numerator" );
    numeratorElement->setAttribute( "value", settings.timeSigNumerator );
    docElement.addChildElement( numeratorElement );

    XmlElement* denominatorElement = new XmlElement( "time_sig_denominator" );
    denominatorElement->setAttribute( "value", settings.timeSigDenominator );
    docElement.addChildElement( denominatorElement );

    XmlElement* lengthElement = new XmlElement( "length" );
    lengthElement->setAttribute( "value", settings.length );
    docElement.addChildElement( lengthElement );

    XmlElement* unitsElement = new XmlElement( "units" );
    unitsElement->setAttribute( "value", settings.units );
    docElement.addChildElement( unitsElement );

    foreach ( QString fileName, settings.audioFileNames )
    {
        XmlElement* sampleElement = new XmlElement( "sample" );
        sampleElement->setAttribute( "filename", fileName.toLocal8Bit().data() );
        docElement.addChildElement( sampleElement );
    }

    foreach ( int slicePointFrameNum, settings.slicePointFrameNums )
    {
        XmlElement* slicePointElement = new XmlElement( "slice_point" );
        slicePointElement->setAttribute( "frame_num", slicePointFrameNum );
        docElement.addChildElement( slicePointElement );
    }

    for ( int i = 0; i < settings.midiNotes.size() && i < settings.noteTimeRatios.size(); i++ )
    {
        XmlElement* noteTimeRatioElement = new XmlElement( "note_time_ratio" );
        noteTimeRatioElement->setAttribute( "note", settings.midiNotes.at( i ) );
        noteTimeRatioElement->setAttribute( "time_ratio", settings.noteTimeRatios.at( i ) );
        docElement.addChildElement( noteTimeRatioElement );
    }

    for ( int i = 0; i < settings.attackValues.size(); i++ )
    {
        XmlElement* envelopeElement = new XmlElement( "envelope" );

        envelopeElement->setAttribute( "attack", settings.attackValues.at( i ) );
        envelopeElement->setAttribute( "release", settings.releaseValues.at( i ) );
        envelopeElement->setAttribute( "one_shot", settings.oneShotSettings.at( i ) );

        docElement.addChildElement( envelopeElement );
    }

    File file( filePath.toLocal8Bit().data() );

    return docElement.writeToFile( file, String::empty );
}



bool TextFileHandler::readProjectXmlFile( const QString filePath, ProjectSettings& settings )
{
    ScopedPointer<XmlElement> docElement;
    docElement = XmlDocument::parse( File( filePath.toLocal8Bit().data() ) );

    bool isSuccessful = false;

    // If the xml file was successfully read
    if ( docElement != NULL )
    {
        // If the main document element has a valid "project" tag
        if ( docElement->hasTagName( "project" ) )
        {
            settings.projectName = docElement->getStringAttribute( "name" ).toRawUTF8();

            forEachXmlChildElement( *docElement, elem )
            {
                if ( elem->hasTagName( "sample_range" ) )   // Use of "sample_range" tag is deprecated
                {
                    SharedSampleRange sampleRange( new SampleRange );

                    sampleRange->startFrame = elem->getIntAttribute( "start_frame" );
                    sampleRange->numFrames = elem->getIntAttribute( "num_frames" );

                    settings.sampleRangeList << sampleRange;
                }
                else if ( elem->hasTagName( "slice_point" ) )
                {
                    settings.slicePointFrameNums << elem->getIntAttribute( "frame_num" );
                }
                else if ( elem->hasTagName( "note_time_ratio" ) )
                {
                    settings.midiNotes << elem->getIntAttribute( "note" );
                    settings.noteTimeRatios << elem->getDoubleAttribute( "time_ratio" );
                }
                else if ( elem->hasTagName( "envelope" ) )
                {
                    settings.attackValues << elem->getDoubleAttribute( "attack" );
                    settings.releaseValues << elem->getDoubleAttribute( "release" );
                    settings.oneShotSettings << elem->getBoolAttribute( "one_shot" );
                }
                else if ( elem->hasTagName( "sample" ) )
                {
                    settings.audioFileNames << elem->getStringAttribute( "filename" ).toRawUTF8();
                }
                else if ( elem->hasTagName( "original_bpm" ) )
                {
                    settings.originalBpm = elem->getDoubleAttribute( "value" );
                }
                else if ( elem->hasTagName( "new_bpm" ) )
                {
                    settings.newBpm = elem->getDoubleAttribute( "value" );
                }
                else if ( elem->hasTagName( "applied_bpm" ) )
                {
                    settings.appliedBpm = elem->getDoubleAttribute( "value" );
                }
                else if ( elem->hasTagName( "time_stretch" ) )
                {
                    settings.isTimeStretchChecked = elem->getBoolAttribute( "checked" );
                }
                else if ( elem->hasTagName( "pitch_correction" ) )
                {
                    settings.isPitchCorrectionChecked = elem->getBoolAttribute( "checked" );
                }
                else if ( elem->hasTagName( "stretch_options" ) )
                {
                    settings.options = elem->getIntAttribute( "value" );
                }
                else if ( elem->hasTagName( "jack_sync" ) )
                {
                    settings.isJackSyncChecked = elem->getBoolAttribute( "checked" );
                }
                else if ( elem->hasTagName( "time_sig_numerator" ) )
                {
                    settings.timeSigNumerator = elem->getIntAttribute( "value" );
                }
                else if ( elem->hasTagName( "time_sig_denominator" ) )
                {
                    settings.timeSigDenominator = elem->getIntAttribute( "value" );
                }
                else if ( elem->hasTagName( "length" ) )
                {
                    settings.length = elem->getIntAttribute( "value" );
                }
                else if ( elem->hasTagName( "units" ) )
                {
                    settings.units = elem->getIntAttribute( "value" );
                }
            }

            isSuccessful = true;
        }
        else // The xml file doesn't have a valid "project" tag
        {
            MessageBoxes::showWarningDialog( QObject::tr("Couldn't open project!"),
                                             QObject::tr("The project file is invalid") );
        }
    }
    else // The xml file couldn't be read
    {
        MessageBoxes::showWarningDialog( QObject::tr("Couldn't open project!"),
                                         QObject::tr("The project file is unreadable") );
    }

    return isSuccessful;
}



bool TextFileHandler::createPathsConfigFile( const PathsConfig& config )
{
    XmlElement docElement( "paths" );

    if ( ! config.tempDirPath.isEmpty() )
    {
        XmlElement* element = new XmlElement( "temp_dir" );
        element->setAttribute( "path", config.tempDirPath.toLocal8Bit().data() );
        docElement.addChildElement( element );
    }

    foreach ( QString path, config.recentProjectPaths )
    {
        XmlElement* element = new XmlElement( "recent_project" );
        element->setAttribute( "path", path.toLocal8Bit().data() );
        docElement.addChildElement( element );
    }

    File pathsConfigFile( PATHS_CONFIG_FILE_PATH );
    pathsConfigFile.create();

    return docElement.writeToFile( pathsConfigFile, String::empty );
}



bool TextFileHandler::readPathsConfigFile( PathsConfig& config )
{
    ScopedPointer<XmlElement> docElement;
    docElement = XmlDocument::parse( File( PATHS_CONFIG_FILE_PATH ) );

    bool isSuccessful = false;

    if ( docElement != NULL )
    {
        if ( docElement->hasTagName( "paths" ) )
        {
            forEachXmlChildElement( *docElement, elem )
            {
                if ( elem->hasTagName( "temp_dir" ) )
                {
                    config.tempDirPath = elem->getStringAttribute( "path" ).toRawUTF8();
                }
                else if ( elem->hasTagName( "recent_project" ) )
                {
                    config.recentProjectPaths << elem->getStringAttribute( "path" ).toRawUTF8();
                }
            }

            isSuccessful = true;
        }
    }

    return isSuccessful;
}



bool TextFileHandler::createH2DrumkitXmlFile( const QString dirPath, const QString kitName,
                                              const QStringList audioFileNames,
                                              const SamplerAudioSource::EnvelopeSettings& envelopes )
{
    Q_ASSERT( audioFileNames.size() == envelopes.attackValues.size() );

    const String infoFormattingText = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Lucida Grande'; font-size:10pt; font-weight:400; font-style:normal;\">\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p></body></html>";

    XmlElement docElement( "drumkit_info" );
    docElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
    docElement.setAttribute( "xmlns", "http://www.hydrogen-music.org/drumkit" );

    XmlElement* nameElement = new XmlElement( "name" );
    nameElement->addTextElement( kitName.toLocal8Bit().data() );
    docElement.addChildElement( nameElement );

    XmlElement* authorElement = new XmlElement( "author" );
    authorElement->addTextElement( "" );
    docElement.addChildElement( authorElement );

    XmlElement* infoElement = new XmlElement( "info" );
    infoElement->addTextElement( infoFormattingText );
    docElement.addChildElement( infoElement );

    XmlElement* licenseElement = new XmlElement( "license" );
    licenseElement->addTextElement( "" );
    docElement.addChildElement( licenseElement );

    XmlElement* instrumentListElement = new XmlElement( "instrumentList" );

    for ( int i = 0; i < audioFileNames.size(); i++ )
    {
        XmlElement* instrumentElement = new XmlElement( "instrument" );

        XmlElement* idElement = new XmlElement( "id" );
        idElement->addTextElement( String( i ) );
        instrumentElement->addChildElement( idElement );

        XmlElement* nameElement = new XmlElement( "name" );
        String name( kitName.toLocal8Bit().data() );
        name += " ";
        name += String( i + 1 );
        nameElement->addTextElement( name );
        instrumentElement->addChildElement( nameElement );

        XmlElement* volumeElement = new XmlElement( "volume" );
        volumeElement->addTextElement( "1" );
        instrumentElement->addChildElement( volumeElement );

        XmlElement* isMutedElement = new XmlElement( "isMuted" );
        isMutedElement->addTextElement( "false" );
        instrumentElement->addChildElement( isMutedElement );

        XmlElement* pan_LElement = new XmlElement( "pan_L" );
        pan_LElement->addTextElement( "1" );
        instrumentElement->addChildElement( pan_LElement );

        XmlElement* pan_RElement = new XmlElement( "pan_R" );
        pan_RElement->addTextElement( "1" );
        instrumentElement->addChildElement( pan_RElement );

        XmlElement* randomPitchFactorElement = new XmlElement( "randomPitchFactor" );
        randomPitchFactorElement->addTextElement( "0" );
        instrumentElement->addChildElement( randomPitchFactorElement );

        XmlElement* gainElement = new XmlElement( "gain" );
        gainElement->addTextElement( "1" );
        instrumentElement->addChildElement( gainElement );

        XmlElement* filterActiveElement = new XmlElement( "filterActive" );
        filterActiveElement->addTextElement( "false" );
        instrumentElement->addChildElement( filterActiveElement );

        XmlElement* filterCutoffElement = new XmlElement( "filterCutoff" );
        filterCutoffElement->addTextElement( "1" );
        instrumentElement->addChildElement( filterCutoffElement );

        XmlElement* filterResonanceElement = new XmlElement( "filterResonance" );
        filterResonanceElement->addTextElement( "0" );
        instrumentElement->addChildElement( filterResonanceElement );

        XmlElement* AttackElement = new XmlElement( "Attack" );
        const int attackValue = static_cast<int>( envelopes.attackValues.at( i ) * envelopes.attackValues.at( i ) * 100000 );
        AttackElement->addTextElement( String( attackValue ) );
        instrumentElement->addChildElement( AttackElement );

        XmlElement* DecayElement = new XmlElement( "Decay" );
        DecayElement->addTextElement( "0" );
        instrumentElement->addChildElement( DecayElement );

        XmlElement* SustainElement = new XmlElement( "Sustain" );
        SustainElement->addTextElement( "1" );
        instrumentElement->addChildElement( SustainElement );

        XmlElement* ReleaseElement = new XmlElement( "Release" );
        const int releaseValue = static_cast<int>( envelopes.releaseValues.at( i ) * envelopes.releaseValues.at( i ) * 100000 );
        ReleaseElement->addTextElement( String( releaseValue ) );
        instrumentElement->addChildElement( ReleaseElement );

        XmlElement* muteGroupElement = new XmlElement( "muteGroup" );
        muteGroupElement->addTextElement( "-1" );
        instrumentElement->addChildElement( muteGroupElement );

        XmlElement* midiOutChannelElement = new XmlElement( "midiOutChannel" );
        midiOutChannelElement->addTextElement( "-1" );
        instrumentElement->addChildElement( midiOutChannelElement );

        XmlElement* midiOutNoteElement = new XmlElement( "midiOutNote" );
        midiOutNoteElement->addTextElement( "60" );
        instrumentElement->addChildElement( midiOutNoteElement );

        XmlElement* isStopNoteElement = new XmlElement( "isStopNote" );
        isStopNoteElement->addTextElement( "false" );
        instrumentElement->addChildElement( isStopNoteElement );

        XmlElement* FX1LevelElement = new XmlElement( "FX1Level" );
        FX1LevelElement->addTextElement( "0" );
        instrumentElement->addChildElement( FX1LevelElement );

        XmlElement* FX2LevelElement = new XmlElement( "FX2Level" );
        FX2LevelElement->addTextElement( "0" );
        instrumentElement->addChildElement( FX2LevelElement );

        XmlElement* FX3LevelElement = new XmlElement( "FX3Level" );
        FX3LevelElement->addTextElement( "0" );
        instrumentElement->addChildElement( FX3LevelElement );

        XmlElement* FX4LevelElement = new XmlElement( "FX4Level" );
        FX4LevelElement->addTextElement( "0" );
        instrumentElement->addChildElement( FX4LevelElement );

        XmlElement* layerElement = new XmlElement( "layer" );

        {
            XmlElement* filenameElement = new XmlElement( "filename" );
            filenameElement->addTextElement( audioFileNames.at( i ).toLocal8Bit().data() );
            layerElement->addChildElement( filenameElement );

            XmlElement* minElement = new XmlElement( "min" );
            minElement->addTextElement( "0" );
            layerElement->addChildElement( minElement );

            XmlElement* maxElement = new XmlElement( "max" );
            maxElement->addTextElement( "1" );
            layerElement->addChildElement( maxElement );

            XmlElement* gainElement = new XmlElement( "gain" );
            gainElement->addTextElement( "1" );
            layerElement->addChildElement( gainElement );

            XmlElement* pitchElement = new XmlElement( "pitch" );
            pitchElement->addTextElement( "0" );
            layerElement->addChildElement( pitchElement );
        }

        instrumentElement->addChildElement( layerElement );

        instrumentListElement->addChildElement( instrumentElement );
    }

    docElement.addChildElement( instrumentListElement );

    File file( QDir( dirPath ).absoluteFilePath( "drumkit.xml" ).toLocal8Bit().data() );

    return docElement.writeToFile( file, String::empty );
}



bool TextFileHandler::createSFZFile( const QString sfzFilePath,
                                     const QString samplesDirName,
                                     const QStringList audioFileNames,
                                     const QList<SharedSampleBuffer> sampleBufferList,
                                     const qreal sampleRate,
                                     const SamplerAudioSource::EnvelopeSettings& envelopes )
{
    Q_ASSERT( sampleBufferList.size() == audioFileNames.size() &&
              sampleBufferList.size() == envelopes.attackValues.size() );

    File file( sfzFilePath.toLocal8Bit().data() );

    const bool isSuccessful = file.create();

    if ( isSuccessful )
    {
        int key = Midi::MIDDLE_C;

        if ( audioFileNames.size() > Midi::MAX_POLYPHONY - Midi::MIDDLE_C )
        {
            key = qMax( Midi::MAX_POLYPHONY - audioFileNames.size(), 0 );
        }

        for ( int i = 0; i < audioFileNames.size() && i < Midi::MAX_POLYPHONY; i++ )
        {
            const QString fileName = audioFileNames.at( i );
            const qreal attackValue = ( envelopes.attackValues.at( i ) * sampleBufferList.at( i )->getNumFrames() ) / sampleRate;
            const qreal releaseValue = ( envelopes.releaseValues.at( i ) * sampleBufferList.at( i )->getNumFrames() ) / sampleRate;
            const QString loopMode = envelopes.oneShotSettings.at( i ) ? "one_shot" : "no_loop";

            const QString groupText = "<group> key=" + QString::number( key ) +
                                      " ampeg_attack=" + QString::number( attackValue ) +
                                      " ampeg_release=" + QString::number( releaseValue ) +
                                      " loop_mode=" + loopMode + "\n";
            const QString regionText = "<region> sample=" + samplesDirName + "\\" + fileName + "\n";

            file.appendText( groupText.toLocal8Bit().data() );
            file.appendText( regionText.toLocal8Bit().data() );
            file.appendText( "\n" );

            key++;
        }
    }

    return isSuccessful;
}
