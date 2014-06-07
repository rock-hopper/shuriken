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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include "commands.h"
#include "globals.h"
//#include <QDebug>


//==================================================================================================
// Public:

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    mUI( new Ui::MainWindow ),
    mLastOpenedImportDir( QDir::homePath() ),
    mLastOpenedProjDir( QDir::homePath() ),
    mAppliedOriginalBPM( 0.0 ),
    mAppliedNewBPM( 0.0 )
{
    // Set up user interface
    mUI->setupUi( this );


    // Populate "Detection Method" combo box
    QStringList detectMethodTextList, detectMethodDataList;

    detectMethodTextList << "Broadband Energy" << "High Frequency Content" << "Complex Domain"
            << "Phase Based" << "Spectral Difference" << "Kullback-Liebler"
            << "Modified Kullback-Liebler" << "Spectral Flux";

    detectMethodDataList << "energy" << "hfc" << "complex" << "phase" << "specdiff"
            << "kl" << "mkl" << "specflux";

    for ( int i = 0; i < detectMethodTextList.size(); i++ )
    {
        mUI->comboBox_DetectMethod->addItem( detectMethodTextList[ i ], detectMethodDataList[ i ] );
    }


    // Populate "Window Size" combo box
    QStringList windowSizeTextList;
    QList<int> windowSizeDataList;

    windowSizeTextList << "128" << "256" << "512" << "1024" << "2048" << "4096" << "8192";
    windowSizeDataList << 128 << 256 << 512 << 1024 << 2048 << 4096 << 8192;

    for ( int i = 0; i < windowSizeTextList.size(); i++ )
    {
        mUI->comboBox_WindowSize->addItem( windowSizeTextList[ i ], windowSizeDataList[ i ] );
    }

    mUI->comboBox_WindowSize->setCurrentIndex( 3 ); // "1024"


    // Populate "Hop Size" combo box
    QStringList hopSizeTextList;
    QList<qreal> hopSizeDataList;

    hopSizeTextList << "50%" << "25%" << "12.5%" << "6.25%";
    hopSizeDataList << 50.0 << 25.0 << 12.5 << 6.25;

    for ( int i = 0; i < hopSizeTextList.size(); i++ )
    {
        mUI->comboBox_HopSize->addItem( hopSizeTextList[ i ], hopSizeDataList[ i ] );
    }

    mUI->comboBox_HopSize->setCurrentIndex( 0 ); // "50%"


    mUI->checkBox_AdvancedOptions->setChecked( false );


    // Connect signals to slots
    QObject::connect( mUI->waveGraphicsView, SIGNAL( slicePointOrderChanged(SharedSlicePointItem,int,int) ),
                      this, SLOT( recordSlicePointItemMove(SharedSlicePointItem,int,int) ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( minDetailLevelReached() ),
                      this, SLOT( disableZoomOut() ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( disableZoomIn() ) );

    QObject::connect( &mUndoStack, SIGNAL( canUndoChanged(bool) ),
                      mUI->actionUndo, SLOT( setEnabled(bool) ) );

    QObject::connect( &mUndoStack, SIGNAL( canRedoChanged(bool) ),
                      mUI->actionRedo, SLOT( setEnabled(bool) ) );

    QObject::connect( mUI->actionUndo, SIGNAL( triggered() ),
                      &mUndoStack, SLOT( undo() ) );

    QObject::connect( mUI->actionRedo, SIGNAL( triggered() ),
                      &mUndoStack, SLOT( redo() ) );


    // Initialise the audio device manager
    ScopedPointer<XmlElement> stateXml;

    File configFile( AUDIO_CONFIG_FILE_PATH );
    if ( configFile.existsAsFile() )
    {
        stateXml = XmlDocument::parse( configFile );
    }

    const String error = mDeviceManager.initialise( NUM_INPUT_CHANS, NUM_OUTPUT_CHANS, stateXml, true );

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error initialising audio device manager!"), error.toRawUTF8() );
        mUI->actionAudio_Setup->setDisabled( true );
        mIsAudioInitialised = false;
    }
    else
    {
        mAudioSetupDialog = new AudioSetupDialog( mDeviceManager, this );

        QObject::connect( mAudioSetupDialog.get(), SIGNAL( realtimeModeEnabled(bool) ),
                          this, SLOT( enableRealtimeMode(bool) ) );

        mSamplerAudioSource = new SamplerAudioSource();
        mIsAudioInitialised = true;
    }


    // Check there were no errors while initialising the audio file handler
    if ( ! mFileHandler.getLastErrorTitle().isEmpty() )
    {
        showWarningBox( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



MainWindow::~MainWindow()
{
    on_actionClose_Project_triggered();
    delete mUI;
}



//==================================================================================================
// Protected:

void MainWindow::changeEvent( QEvent* event )
{
    QMainWindow::changeEvent( event );
    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        mUI->retranslateUi( this );
        break;
    default:
        break;
    }
}



//==================================================================================================
// Private:

void MainWindow::setUpSampler( const int numChans )
{
    if ( mIsAudioInitialised )
    {
        if ( mAudioSetupDialog->isRealtimeModeEnabled() ) // Timestretch mode
        {
            mRubberbandAudioSource = new RubberbandAudioSource( mSamplerAudioSource, numChans );
            mAudioSourcePlayer.setSource( mRubberbandAudioSource );
        }
        else // Offline timestretch mode
        {
            mAudioSourcePlayer.setSource( mSamplerAudioSource );
        }

        mDeviceManager.addAudioCallback( &mAudioSourcePlayer );
        mDeviceManager.addMidiInputCallback( String::empty, mSamplerAudioSource->getMidiMessageCollector() );
    }
}



void MainWindow::tearDownSampler( const bool isSampleToBeCleared )
{
    if ( mIsAudioInitialised )
    {
        mAudioSourcePlayer.setSource( NULL );
        mRubberbandAudioSource = NULL;
        mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );
        mDeviceManager.removeMidiInputCallback( String::empty, mSamplerAudioSource->getMidiMessageCollector() );

        if ( isSampleToBeCleared )
            mSamplerAudioSource->clearSample();
    }
}



void MainWindow::enableUI()
{
    mUI->doubleSpinBox_OriginalBPM->setEnabled( true );
    mUI->doubleSpinBox_NewBPM->setEnabled( true );
    mUI->pushButton_CalcBPM->setEnabled( true );
    mUI->pushButton_Apply->setEnabled( true );
    mUI->pushButton_FindOnsets->setEnabled( true );
    mUI->pushButton_FindBeats->setEnabled( true );

    if ( mIsAudioInitialised )
    {
        mUI->pushButton_Play->setEnabled( true );
        mUI->pushButton_Stop->setEnabled( true );
    }

    mUI->actionSave_Project->setEnabled( true );
    mUI->actionClose_Project->setEnabled( true );
    mUI->actionAdd_Slice_Point->setEnabled( true );
    mUI->actionDelete->setEnabled( true );
    mUI->actionReverse->setEnabled( true );
    mUI->actionZoom_Original->setEnabled( true );
    mUI->actionZoom_In->setEnabled( true );
}



void MainWindow::disableUI()
{
    mUI->doubleSpinBox_OriginalBPM->setValue( 0.0 );
    mUI->doubleSpinBox_OriginalBPM->setEnabled( false );
    mUI->doubleSpinBox_NewBPM->setValue( 0.0 );
    mUI->doubleSpinBox_NewBPM->setEnabled( false );
    mUI->pushButton_CalcBPM->setEnabled( false );
    mUI->pushButton_Apply->setEnabled( false );
    mUI->pushButton_Slice->setEnabled( false );
    mUI->pushButton_FindOnsets->setEnabled( false );
    mUI->pushButton_FindBeats->setEnabled( false );
    mUI->pushButton_Play->setEnabled( false );
    mUI->pushButton_Stop->setEnabled( false );

    mUI->actionSave_Project->setEnabled( false );
    mUI->actionClose_Project->setEnabled( false );
    mUI->actionAdd_Slice_Point->setEnabled( false );
    mUI->actionDelete->setEnabled( false );
    mUI->actionReverse->setEnabled( false );
    mUI->actionZoom_Original->setEnabled( false );
    mUI->actionZoom_Out->setEnabled( false );
    mUI->actionZoom_In->setEnabled( false );
}



AudioAnalyser::DetectionSettings MainWindow::getDetectionSettings()
{
    AudioAnalyser::DetectionSettings settings;
    int currentIndex;

    // From aubio website: "Typical threshold values are within 0.001 and 0.900." Default is 0.3 in aubio-0.4.0
    currentIndex = mUI->comboBox_DetectMethod->currentIndex();
    settings.detectionMethod = mUI->comboBox_DetectMethod->itemData( currentIndex ).toString().toLocal8Bit();

    settings.threshold = qreal( mUI->horizontalSlider_Threshold->value() ) / 1000.0;

    currentIndex = mUI->comboBox_WindowSize->currentIndex();
    settings.windowSize = (uint_t) mUI->comboBox_WindowSize->itemData( currentIndex ).toInt();

    currentIndex = mUI->comboBox_HopSize->currentIndex();
    const qreal percentage = mUI->comboBox_HopSize->itemData( currentIndex ).toReal();
    settings.hopSize = (uint_t) ( settings.windowSize * ( percentage / 100.0 ) );

    settings.sampleRate = (uint_t) mCurrentSampleHeader->sampleRate;

    return settings;
}



void MainWindow::getSampleRanges( QList<SharedSampleRange>& sampleRangeList )
{
    const QList<int> slicePointFrameNumList = mUI->waveGraphicsView->getSlicePointFrameNumList();
    const int totalNumFrames = mCurrentSampleBuffer->getNumFrames();
    const qreal sampleRate = mCurrentSampleHeader->sampleRate;
    const int minFramesBetweenSlicePoints = roundToInt( sampleRate * AudioAnalyser::MIN_INTER_ONSET_SECS );

    QList<int> startFramesList;
    int prevSlicePointFrameNum = 0;

    foreach ( int slicePointFrameNum, slicePointFrameNumList )
    {
        if ( slicePointFrameNum > minFramesBetweenSlicePoints &&
             slicePointFrameNum < totalNumFrames - minFramesBetweenSlicePoints )
        {
            if ( slicePointFrameNum > prevSlicePointFrameNum + minFramesBetweenSlicePoints )
            {
                startFramesList.append( slicePointFrameNum );
                prevSlicePointFrameNum = slicePointFrameNum;
            }
        }
    }

    if ( startFramesList.first() != 0 )
    {
        startFramesList.prepend( 0 );
    }

    sampleRangeList.clear();

    for ( int i = 0; i < startFramesList.size(); ++i )
    {
        SharedSampleRange sampleRange( new SampleRange() );

        sampleRange->startFrame = startFramesList.at( i );
        sampleRange->numFrames  = i < startFramesList.size() - 1 ?
                                  startFramesList.at( i + 1 ) - startFramesList.at( i ) :
                                  totalNumFrames - startFramesList.at( i );

        sampleRangeList << sampleRange;
    }
}



//==================================================================================================
// Private Static:

void MainWindow::showWarningBox( const QString text, const QString infoText )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Shuriken Beat Slicer" );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setText( text );
    msgBox.setInformativeText( infoText );
    msgBox.exec();
}



//==================================================================================================
// Public Slots:

void MainWindow::reorderSampleRangeList( const int startOrderPos, const int destOrderPos )
{
    mSampleRangeList.move( startOrderPos, destOrderPos );
    mSamplerAudioSource->setSampleRanges( mSampleRangeList );
}



//==================================================================================================
// Private Slots:

void MainWindow::recordWaveformItemMove( const int startOrderPos, const int destOrderPos )
{
    QUndoCommand* command = new MoveWaveformItemCommand( startOrderPos, destOrderPos, mUI->waveGraphicsView, this );
    mUndoStack.push( command );
}



void MainWindow::recordSlicePointItemMove( const SharedSlicePointItem slicePointItem,
                                           const int oldFrameNum,
                                           const int newFrameNum )
{
    QUndoCommand* command = new MoveSlicePointItemCommand( slicePointItem, oldFrameNum, newFrameNum, mUI->waveGraphicsView );
    mUndoStack.push( command );
}



void MainWindow::playSampleRange( const int waveformItemStartFrame, const int waveformItemNumFrames, const QPointF mouseScenePos )
{
    SharedSampleRange sampleRange( new SampleRange );
    sampleRange->startFrame = waveformItemStartFrame;
    sampleRange->numFrames = waveformItemNumFrames;
    int endFrame = waveformItemStartFrame + waveformItemNumFrames;

    const QList<int> slicePointFrameNumList = mUI->waveGraphicsView->getSlicePointFrameNumList();

    // If slice points are present and the waveform has not yet been sliced...
    if ( slicePointFrameNumList.size() > 0 && mSampleRangeList.isEmpty() )
    {
        const int mousePosFrameNum = mUI->waveGraphicsView->getFrameNum( mouseScenePos.x() );

        foreach (  int slicePointFrameNum, slicePointFrameNumList )
        {
            if ( slicePointFrameNum <= mousePosFrameNum )
            {
                sampleRange->startFrame = slicePointFrameNum;
            }
            else
            {
                endFrame = slicePointFrameNum;
                break;
            }
        }
    }

    sampleRange->numFrames = endFrame - sampleRange->startFrame;

    mSamplerAudioSource->playRange( sampleRange );
}



void MainWindow::disableZoomIn()
{
    mUI->actionZoom_In->setEnabled( false );
}



void MainWindow::disableZoomOut()
{
    mUI->actionZoom_Out->setEnabled( false );
}



void MainWindow::enableRealtimeMode( const bool isEnabled )
{
    if ( ! mCurrentSampleBuffer.isNull() )
    {
        if ( isEnabled )
        {
            mUI->pushButton_Apply->setEnabled( false );
            mUI->doubleSpinBox_OriginalBPM->setValue( mAppliedNewBPM );
            mUI->doubleSpinBox_NewBPM->setValue( mAppliedNewBPM );
        }
        else
        {
            mUI->pushButton_Apply->setEnabled( true );
            mUI->doubleSpinBox_OriginalBPM->setValue( mAppliedOriginalBPM );
            mUI->doubleSpinBox_NewBPM->setValue( mAppliedNewBPM );
        }

        const bool isSampleToBeCleared = false;

        tearDownSampler( isSampleToBeCleared );
        setUpSampler( mCurrentSampleBuffer->getNumChannels() );
    }
}



//====================
// "File" menu:

void MainWindow::on_actionOpen_Project_triggered()
{
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Open Project"), mLastOpenedProjDir,
                                                           tr("Shuriken Project (shuriken.xml)") );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QFileInfo projFileInfo( filePath );
        QDir projectDir = projFileInfo.absoluteDir();

        QDir parentDir( projectDir );
        parentDir.cdUp();

        mLastOpenedProjDir = parentDir.absolutePath();

        ScopedPointer<XmlElement> docElement;
        docElement = XmlDocument::parse( File( filePath.toUtf8().data() ) );

        // If the xml file was successfully read
        if ( docElement.get() != NULL )
        {
            // If the main document element has a valid "project" tag
            if ( docElement->hasTagName( "project" ) )
            {
                QString projectName = docElement->getStringAttribute( "name" ).toRawUTF8();
                QString audioFileName;
                qreal originalBpm = 0.0;
                qreal newBpm = 0.0;
                bool isTimeStretchChecked = false;
                bool isPitchCorrectionChecked = false;
                bool isRealtimeModeEnabled = false;

                QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

                on_actionClose_Project_triggered();

                forEachXmlChildElement( *docElement, elem )
                {
                    if ( elem->hasTagName( "sample_range" ) )
                    {
                        SharedSampleRange sampleRange( new SampleRange() );

                        sampleRange->startFrame = elem->getIntAttribute( "start_frame" );
                        sampleRange->numFrames = elem->getIntAttribute( "num_frames" );

                        mSampleRangeList << sampleRange;
                    }
                    else if ( elem->hasTagName( "sample" ) )
                    {
                        audioFileName = elem->getStringAttribute( "filename" ).toRawUTF8();
                    }
                    else if ( elem->hasTagName( "original_bpm" ) )
                    {
                        originalBpm = elem->getDoubleAttribute( "value" );
                    }
                    else if ( elem->hasTagName( "new_bpm" ) )
                    {
                        newBpm = elem->getDoubleAttribute( "value" );
                    }
                    else if ( elem->hasTagName( "time_stretch" ) )
                    {
                        isTimeStretchChecked = elem->getBoolAttribute( "checked" );
                    }
                    else if ( elem->hasTagName( "pitch_correction" ) )
                    {
                        isPitchCorrectionChecked = elem->getBoolAttribute( "checked" );
                    }
                    else if ( elem->hasTagName( "realtime_mode" ) )
                    {
                        isRealtimeModeEnabled = elem->getBoolAttribute( "enabled" );
                    }
                }

                if ( ! audioFileName.isEmpty() )
                {
                    const QString audioFilePath = projectDir.absoluteFilePath( audioFileName );
                    SharedSampleBuffer sampleBuffer = mFileHandler.getSampleData( audioFilePath );
                    SharedSampleHeader sampleHeader = mFileHandler.getSampleHeader( audioFilePath );

                    // If the audio file was loaded successfully
                    if ( ! sampleBuffer.isNull() && ! sampleHeader.isNull() )
                    {
                        mCurrentSampleBuffer = sampleBuffer;
                        mCurrentSampleHeader = sampleHeader;

                        // If no sample ranges are defined
                        if ( mSampleRangeList.isEmpty() )
                        {
                            const SharedWaveformItem item = mUI->waveGraphicsView->createWaveform( sampleBuffer );

                            QObject::connect( item.data(), SIGNAL( rightMousePressed(int,int,QPointF) ),
                                              this, SLOT( playSampleRange(int,int,QPointF) ) );

                            setUpSampler( sampleBuffer->getNumChannels() );
                            mSamplerAudioSource->setSample( sampleBuffer, sampleHeader->sampleRate );

                            enableUI();
                        }
                        else // Sample ranges are defined
                        {
                            const QList<SharedWaveformItem> waveformItemList =
                                    mUI->waveGraphicsView->createWaveforms( sampleBuffer, mSampleRangeList );

                            foreach ( SharedWaveformItem item, waveformItemList )
                            {
                                QObject::connect( item.data(), SIGNAL( orderPosHasChanged(int,int) ),
                                                  this, SLOT( recordWaveformItemMove(int,int) ) );

                                QObject::connect( item.data(), SIGNAL( orderPosHasChanged(int,int) ),
                                                  this, SLOT( reorderSampleRangeList(int,int) ) );

                                QObject::connect( item.data(), SIGNAL( rightMousePressed(int,int,QPointF) ),
                                                  this, SLOT( playSampleRange(int,int,QPointF) ) );
                            }

                            setUpSampler( sampleHeader->numChans );
                            mSamplerAudioSource->setSample( sampleBuffer, sampleHeader->sampleRate );
                            mSamplerAudioSource->setSampleRanges( mSampleRangeList );

                            enableUI();
                            mUI->actionAdd_Slice_Point->setEnabled( false );
                            mUI->pushButton_FindBeats->setEnabled( false );
                            mUI->pushButton_FindOnsets->setEnabled( false );
                        }

                        mAppliedOriginalBPM = originalBpm;
                        mAppliedNewBPM = originalBpm;

                        if ( mAudioSetupDialog != NULL )
                            mAudioSetupDialog->enableRealtimeMode( isRealtimeModeEnabled );

                        mUI->checkBox_TimeStretch->setChecked( isTimeStretchChecked );
                        mUI->checkBox_PitchCorrection->setChecked( isPitchCorrectionChecked );

                        if ( originalBpm > 0.0 )
                        {
                            mUI->doubleSpinBox_OriginalBPM->setValue( originalBpm );
                        }
                        if ( newBpm > 0.0 )
                        {
                            mUI->doubleSpinBox_NewBPM->setValue( newBpm );
                        }

                        mUI->statusBar->showMessage( tr("Project: ") + projectName );

                        mCurrentAudioFilePath = filePath;

                        QApplication::restoreOverrideCursor();
                    }
                    else // Error loading audio file
                    {
                        QApplication::restoreOverrideCursor();
                        showWarningBox( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
                    }
                }
                else // The xml file doesn't have a valid "sample" element
                {
                    QApplication::restoreOverrideCursor();
                    showWarningBox( tr("Couldn't open project ") + projectName, tr("The project file is invalid") );
                }
            }
            else // The xml file doesn't have a valid "project" tag
            {
                showWarningBox( tr("Couldn't open project!"), tr("The project file is invalid") );
            }
        }
        else // The xml file couldn't be read
        {
            showWarningBox( tr("Couldn't open project!"), tr("The project file is unreadable") );
        }
    }
}



void MainWindow::on_actionSave_Project_triggered()
{
    const QString dirPath = QFileDialog::getSaveFileName( this, tr("Save Project"), mLastOpenedProjDir,
                                                     tr("Shuriken Project (*.*)") );

    // If user didn't click "Cancel"
    if ( ! dirPath.isEmpty() )
    {
        QDir projectDir( dirPath );
        QString projDirName = projectDir.dirName();

        QDir parentDir( projectDir );
        parentDir.cdUp();

        mLastOpenedProjDir = parentDir.absolutePath();

        bool isOkToSave = true;

        // If the directory already exists, ask the user if it should be overwritten
        if ( parentDir.exists( projDirName ) )
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle( "Shuriken Beat Slicer" );
            msgBox.setIcon( QMessageBox::Question );
            msgBox.setText( "The directory \"" + projDirName + "\"" + " already exists" );
            msgBox.setInformativeText( "Do you want to overwrite the contents of " + projDirName + "?" );
            msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
            msgBox.setDefaultButton( QMessageBox::Ok );
            const int buttonFlag = msgBox.exec();

            if ( buttonFlag == QMessageBox::Ok )
            {
                bool isSuccessful;

                QStringList nameFilters;
                nameFilters << "*.wav" << "*.xml";

                foreach ( QString dirEntry, projectDir.entryList( nameFilters ) )
                {
                    isSuccessful = projectDir.remove( dirEntry );

                    if ( ! isSuccessful )
                        isOkToSave = false;
                }
            }
            else
            {
                isOkToSave = false;
            }
        }
        else // Directory does not yet exist
        {
            isOkToSave = parentDir.mkdir( projDirName );
        }

        if ( isOkToSave )
        {
            const QString filePath = projectDir.absoluteFilePath( "audio.wav" );

            QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

            const bool isSuccessful = mFileHandler.saveAudioFile( filePath, mCurrentSampleBuffer, mCurrentSampleHeader );

            if ( isSuccessful )
            {
                const bool isRealtimeModeEnabled = mAudioSetupDialog->isRealtimeModeEnabled();

                XmlElement docElement( "project" );
                docElement.setAttribute( "name", projDirName.toUtf8().data() );

                XmlElement* realtimeModeElement = new XmlElement( "realtime_mode" );
                realtimeModeElement->setAttribute( "enabled", isRealtimeModeEnabled );
                docElement.addChildElement( realtimeModeElement );

                XmlElement* origBpmElement = new XmlElement( "original_bpm" );
                XmlElement* newBpmElement = new XmlElement( "new_bpm" );

                if ( isRealtimeModeEnabled )
                {
                    origBpmElement->setAttribute( "value", mUI->doubleSpinBox_OriginalBPM->value() );
                    newBpmElement->setAttribute( "value", mUI->doubleSpinBox_NewBPM->value() );
                }
                else
                {
                    origBpmElement->setAttribute( "value", mAppliedNewBPM );
                    newBpmElement->setAttribute( "value", mAppliedNewBPM );
                }
                docElement.addChildElement( origBpmElement );
                docElement.addChildElement( newBpmElement );

                XmlElement* timeStretchElement = new XmlElement( "time_stretch" );
                timeStretchElement->setAttribute( "checked", mUI->checkBox_TimeStretch->isChecked() );
                docElement.addChildElement( timeStretchElement );

                XmlElement* pitchCorrectionElement = new XmlElement( "pitch_correction" );
                pitchCorrectionElement->setAttribute( "checked", mUI->checkBox_PitchCorrection->isChecked() );
                docElement.addChildElement( pitchCorrectionElement );

                XmlElement* sampleElement = new XmlElement( "sample" );
                sampleElement->setAttribute( "filename", "audio.wav" );
                docElement.addChildElement( sampleElement );

                if ( ! mSampleRangeList.isEmpty() )
                {
                    foreach ( SharedSampleRange sampleRange, mSampleRangeList )
                    {
                        XmlElement* rangeElement = new XmlElement( "sample_range" );
                        rangeElement->setAttribute( "start_frame", sampleRange->startFrame );
                        rangeElement->setAttribute( "num_frames", sampleRange->numFrames );
                        docElement.addChildElement( rangeElement );
                    }
                }

                File file( projectDir.absoluteFilePath( "shuriken.xml" ).toUtf8().data() );

                docElement.writeToFile( file, String::empty );

                QApplication::restoreOverrideCursor();
            }
            else // An error occurred while writing the audio file
            {
                QApplication::restoreOverrideCursor();
                showWarningBox( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
            }
        }
        else // It isn't possible to save the project
        {
            showWarningBox( tr("Could not save project ") + "\"" + projDirName + "\"",
                            tr("Failed to create directory ") + "\"" + projectDir.absolutePath() + "\"" );
        }
    }
}



void MainWindow::on_actionClose_Project_triggered()
{
    mCurrentSampleBuffer.clear();
    mCurrentSampleHeader.clear();
    mSampleRangeList.clear();
    tearDownSampler( true );

    mUI->waveGraphicsView->clearAll();
    on_actionZoom_Original_triggered();
    disableUI();
    mUI->statusBar->clearMessage();

    mUndoStack.clear();

    mAppliedOriginalBPM = 0.0;
    mAppliedNewBPM = 0.0;
}



void MainWindow::on_actionImport_Audio_File_triggered()
{
    // Open audio file dialog
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Import Audio File"), mLastOpenedImportDir,
                                                           tr("All Files (*.*)") );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        const QFileInfo fileInfo( filePath );
        const QString fileName = fileInfo.fileName();

        mLastOpenedImportDir = fileInfo.absolutePath();

        SharedSampleBuffer sampleBuffer = mFileHandler.getSampleData( filePath );
        SharedSampleHeader sampleHeader = mFileHandler.getSampleHeader( filePath );

        if ( sampleBuffer.isNull() || sampleHeader.isNull() )
        {
            QApplication::restoreOverrideCursor();
            showWarningBox( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
        }
        else
        {
            on_actionClose_Project_triggered();

            mCurrentSampleBuffer = sampleBuffer;
            mCurrentSampleHeader = sampleHeader;

            const SharedWaveformItem item = mUI->waveGraphicsView->createWaveform( sampleBuffer );

            QObject::connect( item.data(), SIGNAL( rightMousePressed(int,int,QPointF) ),
                              this, SLOT( playSampleRange(int,int,QPointF) ) );

            setUpSampler( sampleBuffer->getNumChannels() );
            mSamplerAudioSource->setSample( sampleBuffer, sampleHeader->sampleRate );

            enableUI();

            // Set status bar message
            QString chanString = sampleHeader->numChans == 1 ? "Mono" : "Stereo";

            QString bitsString;
            bitsString.setNum( sampleHeader->bitsPerSample );
            bitsString += " bits";

            QString rateString;
            rateString.setNum( sampleHeader->sampleRate );
            rateString += " Hz";

            QString message = fileName + ", " + chanString + ", " + bitsString + ", " + rateString +
                              ", " + sampleHeader->format;
            mUI->statusBar->showMessage( message );


            mCurrentAudioFilePath = filePath;

            QApplication::restoreOverrideCursor();
        }
    }
}



void MainWindow::on_actionExport_As_triggered()
{

}



void MainWindow::on_actionQuit_triggered()
{
    QCoreApplication::quit();
}



//====================
// "Edit" menu:

void MainWindow::on_actionUndo_triggered()
{

}



void MainWindow::on_actionRedo_triggered()
{

}



void MainWindow::on_actionSelect_All_triggered()
{

}



void MainWindow::on_actionClear_Selection_triggered()
{

}



void MainWindow::on_actionDelete_triggered()
{    
    const SharedSlicePointItem selectedSlicePoint = mUI->waveGraphicsView->getSelectedSlicePoint();

    if ( ! selectedSlicePoint.isNull() )
    {
        selectedSlicePoint->setSelected( false );

        QUndoCommand* command = new DeleteSlicePointItemCommand( selectedSlicePoint,
                                                                 mUI->waveGraphicsView,
                                                                 mUI->pushButton_Slice );
        mUndoStack.push( command );
    }
}



void MainWindow::on_actionAdd_Slice_Point_triggered()
{
    const QPoint mousePos = mUI->waveGraphicsView->mapFromGlobal( QCursor::pos() );
    const QPointF mouseScenePos = mUI->waveGraphicsView->mapToScene( mousePos );
    const int frameNum = mUI->waveGraphicsView->getFrameNum( mouseScenePos.x() );

    QUndoCommand* command = new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice );
    mUndoStack.push( command );
}



void MainWindow::on_actionApply_Gain_triggered()
{

}



void MainWindow::on_actionApply_Ramp_triggered()
{

}



void MainWindow::on_actionEnvelope_triggered()
{

}



void MainWindow::on_actionJoin_triggered()
{

}



void MainWindow::on_actionReverse_triggered()
{
    const SharedWaveformItem selectedWaveform = mUI->waveGraphicsView->getSelectedWaveform();

    if ( ! selectedWaveform.isNull() )
    {
        QUndoCommand* command = new ReverseCommand( mCurrentSampleBuffer,
                                                    selectedWaveform,
                                                    mUI->waveGraphicsView );
        mUndoStack.push( command );
    }
}



void MainWindow::on_actionNormalise_triggered()
{

}



//====================
// "Options" menu:

void MainWindow::on_actionAudio_Setup_triggered()
{
    QPoint pos = mAudioSetupDialog->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    mAudioSetupDialog->move( pos );
    mAudioSetupDialog->show();
}



void MainWindow::on_actionUser_Interface_triggered()
{

}



//====================
// "Help" menu:

void MainWindow::on_actionHelp_triggered()
{

}



void MainWindow::on_actionAbout_triggered()
{

}



//====================
// Main window widgets:

void MainWindow::on_pushButton_CalcBPM_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const AudioAnalyser::DetectionSettings settings = getDetectionSettings();
    const qreal bpm = AudioAnalyser::calcBPM( mCurrentSampleBuffer, settings );
    mUI->doubleSpinBox_OriginalBPM->setValue( bpm );
    mUI->doubleSpinBox_NewBPM->setValue( bpm );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_pushButton_Slice_clicked()
{
    QUndoCommand* command = new CreateSlicesCommand( this,
                                                     mUI->waveGraphicsView,
                                                     mUI->pushButton_Slice,
                                                     mUI->actionAdd_Slice_Point );
    mUndoStack.push( command );
}



void MainWindow::on_horizontalSlider_Threshold_valueChanged( const int value )
{
    mUI->lcdNumber_Threshold->display( qreal( value ) / 1000.0 );
}



void MainWindow::on_pushButton_FindOnsets_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const AudioAnalyser::DetectionSettings settings = getDetectionSettings();
    const QList<int> slicePointFrameNumList = AudioAnalyser::findOnsetFrameNums( mCurrentSampleBuffer, settings );

    QUndoCommand* command = new AddSlicePointItemsCommand( mUI->pushButton_FindOnsets, mUI->pushButton_FindBeats );

    foreach ( int frameNum, slicePointFrameNumList )
    {
        new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice, command );
    }
    mUndoStack.push( command );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_pushButton_FindBeats_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const AudioAnalyser::DetectionSettings settings = getDetectionSettings();
    const QList<int> slicePointFrameNumList = AudioAnalyser::findBeatFrameNums( mCurrentSampleBuffer, settings );

    QUndoCommand* command = new AddSlicePointItemsCommand( mUI->pushButton_FindOnsets, mUI->pushButton_FindBeats );

    foreach ( int frameNum, slicePointFrameNumList )
    {
        new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice, command );
    }
    mUndoStack.push( command );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_checkBox_AdvancedOptions_toggled( const bool isChecked )
{
    if ( isChecked ) // Show advanced options
    {
        const int numWidgets = mUI->horizontalLayout_AdvancedOptions->count();
        for ( int i = 0; i < numWidgets; i++ )
            mUI->horizontalLayout_AdvancedOptions->itemAt( i )->widget()->setVisible( true );

        mUI->horizontalLayout_AdvancedOptions->addSpacerItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding ) );
    }
    else // Hide advanced options
    {
        const int numWidgets = mUI->horizontalLayout_AdvancedOptions->count() - 1; // Don't include spacer
        for ( int i = 0; i < numWidgets; i++ )
            mUI->horizontalLayout_AdvancedOptions->itemAt( i )->widget()->setVisible( false );

        QLayoutItem* spacerItem = mUI->horizontalLayout_AdvancedOptions->itemAt( numWidgets );
        mUI->horizontalLayout_AdvancedOptions->removeItem( spacerItem );
        delete spacerItem;
    }
}



void MainWindow::on_doubleSpinBox_OriginalBPM_valueChanged( const double originalBPM )
{
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();
    const bool isPitchCorrectionEnabled = mUI->checkBox_PitchCorrection->isChecked();

    if ( isTimeStretchEnabled && mRubberbandAudioSource != NULL )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal timeRatio = originalBPM / newBPM;
            const qreal pitchScale = isPitchCorrectionEnabled ? 1.0 : newBPM / originalBPM;

            mRubberbandAudioSource->setTimeRatio( timeRatio );
            mRubberbandAudioSource->setPitchScale( pitchScale );
        }
    }
}



void MainWindow::on_doubleSpinBox_NewBPM_valueChanged( const double newBPM )
{
    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();
    const bool isPitchCorrectionEnabled = mUI->checkBox_PitchCorrection->isChecked();

    if ( isTimeStretchEnabled && mRubberbandAudioSource != NULL )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal timeRatio = originalBPM / newBPM;
            const qreal pitchScale = isPitchCorrectionEnabled ? 1.0 : newBPM / originalBPM;

            mRubberbandAudioSource->setTimeRatio( timeRatio );
            mRubberbandAudioSource->setPitchScale( pitchScale );
        }
    }
}



void MainWindow::on_checkBox_TimeStretch_toggled( const bool isChecked )
{
    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = isChecked;
    const bool isPitchCorrectionEnabled = mUI->checkBox_PitchCorrection->isChecked();

    if ( newBPM > 0.0 && originalBPM > 0.0 && mRubberbandAudioSource != NULL )
    {
        qreal timeRatio = 1.0;
        qreal pitchScale = 1.0;

        if ( isTimeStretchEnabled )
        {
            timeRatio = originalBPM / newBPM;
            pitchScale = isPitchCorrectionEnabled ? 1.0 : newBPM / originalBPM;
        }

        mRubberbandAudioSource->setTimeRatio( timeRatio );
        mRubberbandAudioSource->setPitchScale( pitchScale );
    }
}



void MainWindow::on_checkBox_PitchCorrection_toggled( const bool isChecked )
{
    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();
    const bool isPitchCorrectionEnabled = isChecked;

    if ( isTimeStretchEnabled && mRubberbandAudioSource != NULL )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal pitchScale = isPitchCorrectionEnabled ? 1.0 : newBPM / originalBPM;

            mRubberbandAudioSource->setPitchScale( pitchScale );
        }
    }
}



void MainWindow::on_pushButton_Play_clicked()
{
    mSamplerAudioSource->playAll();
}



void MainWindow::on_pushButton_Stop_clicked()
{
    mSamplerAudioSource->stop();
}



void MainWindow::on_actionZoom_In_triggered()
{
    mUI->waveGraphicsView->zoomIn();
    mUI->actionZoom_Out->setEnabled( true );
}



void MainWindow::on_actionZoom_Out_triggered()
{
    mUI->waveGraphicsView->zoomOut();
    mUI->actionZoom_In->setEnabled( true );
}



void MainWindow::on_actionZoom_Original_triggered()
{
    mUI->waveGraphicsView->zoomOriginal();
    mUI->actionZoom_In->setEnabled( true );
    mUI->actionZoom_Out->setEnabled( false );
}



void MainWindow::on_pushButton_Apply_clicked()
{
    Q_ASSERT( ! mCurrentAudioFilePath.isEmpty() );

    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();

    if ( newBPM > 0.0 && originalBPM > 0.0 )
    {
        QUndoCommand* command = new ApplyTimeStretchCommand( this,
                                                             mUI->waveGraphicsView,
                                                             mUI->doubleSpinBox_OriginalBPM,
                                                             mUI->doubleSpinBox_NewBPM,
                                                             mUI->checkBox_PitchCorrection );
        mUndoStack.push( command );

        mAppliedOriginalBPM = originalBPM;
        mAppliedNewBPM = newBPM;
    }
}


