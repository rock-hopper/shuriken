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
#include "applygaindialog.h"
#include <QDebug>
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;


//==================================================================================================
// Public:

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    mUI( new Ui::MainWindow ),
    mLastOpenedImportDir( QDir::homePath() ),
    mLastOpenedProjDir( QDir::homePath() ),
    mAppliedBPM( 0.0 )
{
    setupUI();
    initialiseAudio();
}



MainWindow::~MainWindow()
{
    closeProject();
    delete mUI;
}



void MainWindow::connectWaveformToMainWindow( const SharedWaveformItem item )
{
    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( recordWaveformItemMove(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( reorderSampleRangeList(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( playSampleRange(int,int,QPointF) ),
                      this, SLOT( playSampleRange(int,int,QPointF) ) );
}



void MainWindow::openProject( const QString xmlFilePath )
{
    QFileInfo xmlFileInfo( xmlFilePath );
    QDir projectDir = xmlFileInfo.absoluteDir();

    QDir parentDir( projectDir );
    parentDir.cdUp();

    mLastOpenedProjDir = parentDir.absolutePath();

    ScopedPointer<XmlElement> docElement;
    docElement = XmlDocument::parse( File( xmlFilePath.toLocal8Bit().data() ) );

    // If the xml file was successfully read
    if ( docElement != NULL )
    {
        // If the main document element has a valid "project" tag
        if ( docElement->hasTagName( "project" ) )
        {
            QString projectName = docElement->getStringAttribute( "name" ).toRawUTF8();
            QString audioFileName;
            qreal originalBpm = 0.0;
            qreal newBpm = 0.0;
            qreal appliedBpm = 0.0;
            bool isTimeStretchChecked = false;
            bool isPitchCorrectionChecked = false;
            bool isRealtimeModeEnabled = false;

            QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

            closeProject();

            forEachXmlChildElement( *docElement, elem )
            {
                if ( elem->hasTagName( "sample_range" ) )
                {
                    SharedSampleRange sampleRange( new SampleRange );

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
                else if ( elem->hasTagName( "applied_bpm" ) )
                {
                    appliedBpm = elem->getDoubleAttribute( "value" );
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

                    // Only one sample range is defined - waveform has not been sliced
                    if ( mSampleRangeList.size() == 1 )
                    {
                        const SharedWaveformItem item =
                                mUI->waveGraphicsView->createWaveform( sampleBuffer, mSampleRangeList.first() );
                        connectWaveformToMainWindow( item );

                        setUpSampler( sampleBuffer, sampleHeader );

                        enableUI();
                    }
                    else // Multiple sample ranges are defined - waveform has been sliced
                    {
                        const QList<SharedWaveformItem> waveformItemList =
                                mUI->waveGraphicsView->createWaveforms( sampleBuffer, mSampleRangeList );

                        foreach ( SharedWaveformItem item, waveformItemList )
                        {
                            connectWaveformToMainWindow( item );
                        }

                        setUpSampler( sampleBuffer, sampleHeader );

                        enableUI();
                        mUI->actionAdd_Slice_Point->setEnabled( false );
                        mUI->pushButton_FindBeats->setEnabled( false );
                        mUI->pushButton_FindOnsets->setEnabled( false );
                    }

                    mAppliedBPM = appliedBpm;

                    if ( mOptionsDialog != NULL )
                    {
                        mOptionsDialog->enableRealtimeMode( isRealtimeModeEnabled );
                    }

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

                    mCurrentAudioFilePath = audioFilePath;
                    mCurrentProjDirPath = projectDir.absolutePath();

                    mUI->statusBar->showMessage( tr("Project: ") + projectName );

                    QApplication::restoreOverrideCursor();
                }
                else // Error loading audio file
                {
                    QApplication::restoreOverrideCursor();
                    showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
                }
            }
            else // The xml file doesn't have a valid "sample" element
            {
                QApplication::restoreOverrideCursor();
                showWarningDialog( tr("Couldn't open project ") + projectName, tr("The project file is invalid") );
            }
        }
        else // The xml file doesn't have a valid "project" tag
        {
            showWarningDialog( tr("Couldn't open project!"), tr("The project file is invalid") );
        }
    }
    else // The xml file couldn't be read
    {
        showWarningDialog( tr("Couldn't open project!"), tr("The project file is unreadable") );
    }
}



//==================================================================================================
// Public Static:

void MainWindow::showWarningDialog( const QString text, const QString infoText )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Shuriken Beat Slicer" );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setText( text );
    msgBox.setInformativeText( infoText );
    msgBox.exec();
}



int MainWindow::showUnsavedChangesDialog()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Shuriken Beat Slicer" );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setText( "The project has been modified" );
    msgBox.setInformativeText( "Do you want to save your changes?" );
    msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel) ;
    msgBox.setDefaultButton( QMessageBox::Save );
    const int buttonClicked = msgBox.exec();

    return buttonClicked;
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



void MainWindow::closeEvent( QCloseEvent* event )
{
    // Check for unsaved changes before continuing
    if ( mUndoStack.isClean() )
    {
        event->accept();
    }
    else
    {
        const int buttonClicked = showUnsavedChangesDialog();

        switch ( buttonClicked )
        {
        case QMessageBox::Save:
            on_actionSave_Project_triggered();
            event->accept();
            break;
        case QMessageBox::Discard:
            event->accept();
            break;
        case QMessageBox::Cancel:
            event->ignore();
            break;
        default:
            // Should never be reached
            event->ignore();
            break;
        }
    }
}



//void MainWindow::keyPressEvent( QKeyEvent* event )
//{
//    if ( event->key() == Qt::Key_Space )
//    {
//        mSamplerAudioSource->playAll();
//    }
//    else
//    {
//        QMainWindow::keyPressEvent( event );
//    }
//}



//==================================================================================================
// Private:

void MainWindow::initialiseAudio()
{
    // Read audio config file
    ScopedPointer<XmlElement> stateXml;

    File configFile( AUDIO_CONFIG_FILE_PATH );
    if ( configFile.existsAsFile() )
    {
        stateXml = XmlDocument::parse( configFile );
    }

    // Initialise the audio device manager
    const String error = mDeviceManager.initialise( NUM_INPUT_CHANS, NUM_OUTPUT_CHANS, stateXml, true );

    if ( error.isNotEmpty() )
    {
        showWarningDialog( tr("Error initialising audio device manager!"), error.toRawUTF8() );
        mUI->actionOptions->setDisabled( true );
        mIsAudioInitialised = false;
    }
    else
    {
        mOptionsDialog = new AudioSetupDialog( mDeviceManager, this );

        QObject::connect( mOptionsDialog.get(), SIGNAL( realtimeModeToggled(bool) ),
                          this, SLOT( enableRealtimeControls(bool) ) );

        QObject::connect( mOptionsDialog.get(), SIGNAL( jackSyncToggled(bool) ),
                          mUI->doubleSpinBox_NewBPM, SLOT( setHidden(bool) ) );

        QObject::connect( mOptionsDialog.get(), SIGNAL( jackSyncToggled(bool) ),
                          mUI->label_JackSync, SLOT( setVisible(bool) ) );

        mIsAudioInitialised = true;
    }

    // Check there were no errors while the audio file handler was being initialised
    if ( ! mFileHandler.getLastErrorTitle().isEmpty() )
    {
        showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



void MainWindow::setUpSampler( const SharedSampleBuffer sampleBuffer, const SharedSampleHeader sampleHeader )
{
    if ( mIsAudioInitialised )
    {
        mSamplerAudioSource = new SamplerAudioSource();
        mSamplerAudioSource->setSample( sampleBuffer, sampleHeader->sampleRate );

        if ( mSampleRangeList.size() > 1 )
        {
            mSamplerAudioSource->setSampleRanges( mSampleRangeList );
        }

        if ( mOptionsDialog->isRealtimeModeEnabled() ) // Realtime timestretch mode
        {
            const int numChans = sampleBuffer->getNumChannels();
            const RubberBandStretcher::Options options = mOptionsDialog->getStretcherOptions();
            const bool isJackSyncEnabled = mOptionsDialog->isJackSyncEnabled();

            mRubberbandAudioSource = new RubberbandAudioSource( mSamplerAudioSource, numChans, options, isJackSyncEnabled );
            mAudioSourcePlayer.setSource( mRubberbandAudioSource );

            QObject::connect( mOptionsDialog.get(), SIGNAL( transientsOptionChanged(RubberBandStretcher::Options) ),
                              mRubberbandAudioSource.get(), SLOT( setTransientsOption(RubberBandStretcher::Options) ) );

            QObject::connect( mOptionsDialog.get(), SIGNAL( phaseOptionChanged(RubberBandStretcher::Options) ),
                              mRubberbandAudioSource.get(), SLOT( setPhaseOption(RubberBandStretcher::Options) ) );

            QObject::connect( mOptionsDialog.get(), SIGNAL( formantOptionChanged(RubberBandStretcher::Options) ),
                              mRubberbandAudioSource.get(), SLOT( setFormantOption(RubberBandStretcher::Options) ) );

            QObject::connect( mOptionsDialog.get(), SIGNAL( pitchOptionChanged(RubberBandStretcher::Options) ),
                              mRubberbandAudioSource.get(), SLOT( setPitchOption(RubberBandStretcher::Options) ) );

            QObject::connect( mOptionsDialog.get(), SIGNAL( jackSyncToggled(bool) ),
                              mRubberbandAudioSource.get(), SLOT( enableJackSync(bool) ) );

            on_checkBox_TimeStretch_toggled( mUI->checkBox_TimeStretch->isChecked() );
        }
        else // Offline timestretch mode
        {
            mAudioSourcePlayer.setSource( mSamplerAudioSource );
        }

        mDeviceManager.addAudioCallback( &mAudioSourcePlayer );
        mDeviceManager.addMidiInputCallback( String::empty, mSamplerAudioSource->getMidiMessageCollector() );
    }
}



void MainWindow::tearDownSampler()
{
    if ( mIsAudioInitialised )
    {
        mAudioSourcePlayer.setSource( NULL );

        mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );
        mDeviceManager.removeMidiInputCallback( String::empty, mSamplerAudioSource->getMidiMessageCollector() );

        mRubberbandAudioSource = NULL;
        mSamplerAudioSource = NULL;
    }
}



void MainWindow::setupUI()
{
    // Initialise user interface
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


    // Hide widgets
    mUI->label_JackSync->setVisible( false );
    mUI->checkBox_TimeStretch->setVisible( false );


    // Connect signals to slots
    QObject::connect( mUI->waveGraphicsView, SIGNAL( slicePointOrderChanged(SharedSlicePointItem,int,int) ),
                      this, SLOT( recordSlicePointItemMove(SharedSlicePointItem,int,int) ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( minDetailLevelReached() ),
                      this, SLOT( disableZoomOut() ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( disableZoomIn() ) );

    QObject::connect( mUI->waveGraphicsView->scene(), SIGNAL( selectionChanged() ),
                      this, SLOT( enableEditActions() ) );

    QObject::connect( &mUndoStack, SIGNAL( canUndoChanged(bool) ),
                      mUI->actionUndo, SLOT( setEnabled(bool) ) );

    QObject::connect( &mUndoStack, SIGNAL( canRedoChanged(bool) ),
                      mUI->actionRedo, SLOT( setEnabled(bool) ) );

    QObject::connect( mUI->actionUndo, SIGNAL( triggered() ),
                      &mUndoStack, SLOT( undo() ) );

    QObject::connect( mUI->actionRedo, SIGNAL( triggered() ),
                      &mUndoStack, SLOT( redo() ) );

    QObject::connect( &mUndoStack, SIGNAL( cleanChanged(bool) ),
                      mUI->actionSave_Project, SLOT( setDisabled(bool) ) );


    // Create help form
    mHelpForm = new HelpForm();

    if ( mHelpForm != NULL )
    {
        mUI->actionHelp->setEnabled( true );
    }
}



void MainWindow::enableUI()
{
    if ( mIsAudioInitialised )
    {
        mUI->pushButton_Play->setEnabled( true );
        mUI->pushButton_Stop->setEnabled( true );
        mUI->pushButton_TimestretchOptions->setEnabled( true );

        if ( mOptionsDialog->isRealtimeModeEnabled() )
        {
            mUI->checkBox_TimeStretch->setVisible( true );
        }
        else
        {
            mUI->pushButton_Apply->setEnabled( true );
            mUI->checkBox_TimeStretch->setVisible( false );
        }
    }

    mUI->doubleSpinBox_OriginalBPM->setEnabled( true );
    mUI->doubleSpinBox_NewBPM->setEnabled( true );
    mUI->pushButton_CalcBPM->setEnabled( true );
    mUI->checkBox_TimeStretch->setEnabled( true );
    mUI->checkBox_PitchCorrection->setEnabled( true );
    mUI->comboBox_DetectMethod->setEnabled( true );
    mUI->comboBox_WindowSize->setEnabled( true );
    mUI->comboBox_HopSize->setEnabled( true );
    mUI->lcdNumber_Threshold->setEnabled( true );
    mUI->horizontalSlider_Threshold->setEnabled( true );
    mUI->pushButton_FindOnsets->setEnabled( true );
    mUI->pushButton_FindBeats->setEnabled( true );

    mUI->actionSave_As->setEnabled( true );
    mUI->actionClose_Project->setEnabled( true );
    mUI->actionSelect_All->setEnabled( true );
    mUI->actionSelect_None->setEnabled( true );
    mUI->actionAdd_Slice_Point->setEnabled( true );
    mUI->actionZoom_Original->setEnabled( true );
    mUI->actionZoom_In->setEnabled( true );

    mUI->actionAudition->trigger();
}



void MainWindow::disableUI()
{
    mUI->pushButton_Play->setEnabled( false );
    mUI->pushButton_Stop->setEnabled( false );
    mUI->doubleSpinBox_OriginalBPM->setValue( 0.0 );
    mUI->doubleSpinBox_OriginalBPM->setEnabled( false );
    mUI->doubleSpinBox_NewBPM->setValue( 0.0 );
    mUI->doubleSpinBox_NewBPM->setEnabled( false );
    mUI->pushButton_CalcBPM->setEnabled( false );
    mUI->pushButton_Apply->setEnabled( false );
    mUI->checkBox_TimeStretch->setEnabled( false );
    mUI->checkBox_PitchCorrection->setEnabled( false );
    mUI->pushButton_TimestretchOptions->setEnabled( false );
    mUI->pushButton_Slice->setEnabled( false );
    mUI->comboBox_DetectMethod->setEnabled( false );
    mUI->comboBox_WindowSize->setEnabled( false );
    mUI->comboBox_HopSize->setEnabled( false );
    mUI->lcdNumber_Threshold->setEnabled( false );
    mUI->horizontalSlider_Threshold->setEnabled( false );
    mUI->pushButton_FindOnsets->setEnabled( false );
    mUI->pushButton_FindBeats->setEnabled( false );

    mUI->actionSave_As->setEnabled( false );
    mUI->actionClose_Project->setEnabled( false );
    mUI->actionSelect_All->setEnabled( false );
    mUI->actionSelect_None->setEnabled( false );
    mUI->actionAdd_Slice_Point->setEnabled( false );
    mUI->actionDelete->setEnabled( false );
    mUI->actionJoin->setEnabled( false );
    mUI->actionSplit->setEnabled( false );
    mUI->actionReverse->setEnabled( false );
    mUI->actionZoom_Original->setEnabled( false );
    mUI->actionZoom_Out->setEnabled( false );
    mUI->actionZoom_In->setEnabled( false );
    mUI->actionMove->setEnabled( false );
    mUI->actionSelect->setEnabled( false );
    mUI->actionAudition->setEnabled( false );
}



void MainWindow::getDetectionSettings( AudioAnalyser::DetectionSettings& settings )
{
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



void MainWindow::importAudioFile()
{
    // Open file dialog
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
            showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
        }
        else
        {
            closeProject();

            mCurrentSampleBuffer = sampleBuffer;
            mCurrentSampleHeader = sampleHeader;

            SharedSampleRange sampleRange( new SampleRange );
            sampleRange->startFrame = 0;
            sampleRange->numFrames = sampleBuffer->getNumFrames();
            mSampleRangeList << sampleRange;

            const SharedWaveformItem item = mUI->waveGraphicsView->createWaveform( sampleBuffer, sampleRange );
            connectWaveformToMainWindow( item );

            setUpSampler( sampleBuffer, sampleHeader );

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



void MainWindow::closeProject()
{
    mCurrentSampleBuffer.clear();
    mCurrentSampleHeader.clear();
    mSampleRangeList.clear();
    tearDownSampler();

    mUI->waveGraphicsView->clearAll();
    on_actionZoom_Original_triggered();
    disableUI();
    mUI->statusBar->clearMessage();

    mUndoStack.clear();

    mAppliedBPM = 0.0;

    mCurrentAudioFilePath.clear();
    mCurrentProjDirPath.clear();
}



void MainWindow::saveProject( const QString projDirPath )
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    QDir projectDir( projDirPath );
    QString projectName = projectDir.dirName();

    QString xmlFilePath = projectDir.absoluteFilePath( "shuriken.xml" );

    const QString audioFilePath = mFileHandler.saveAudioFile( projDirPath,
                                                              "audio",
                                                              mCurrentSampleBuffer,
                                                              mCurrentSampleHeader );

    if ( ! audioFilePath.isEmpty() )
    {
        const bool isRealtimeModeEnabled = mOptionsDialog->isRealtimeModeEnabled();

        XmlElement docElement( "project" );
        docElement.setAttribute( "name", projectName.toLocal8Bit().data() );

        XmlElement* realtimeModeElement = new XmlElement( "realtime_mode" );
        realtimeModeElement->setAttribute( "enabled", isRealtimeModeEnabled );
        docElement.addChildElement( realtimeModeElement );

        XmlElement* origBpmElement = new XmlElement( "original_bpm" );
        XmlElement* newBpmElement = new XmlElement( "new_bpm" );
        XmlElement* appliedBpmElement = new XmlElement( "applied_bpm" );

        if ( isRealtimeModeEnabled )
        {
            origBpmElement->setAttribute( "value", mUI->doubleSpinBox_OriginalBPM->value() );
            newBpmElement->setAttribute( "value", mUI->doubleSpinBox_NewBPM->value() );
            appliedBpmElement->setAttribute( "value", mAppliedBPM );
        }
        else
        {
            origBpmElement->setAttribute( "value", mAppliedBPM );
            newBpmElement->setAttribute( "value", mAppliedBPM );
            appliedBpmElement->setAttribute( "value", mAppliedBPM );
        }
        docElement.addChildElement( origBpmElement );
        docElement.addChildElement( newBpmElement );
        docElement.addChildElement( appliedBpmElement );

        XmlElement* timeStretchElement = new XmlElement( "time_stretch" );
        timeStretchElement->setAttribute( "checked", mUI->checkBox_TimeStretch->isChecked() );
        docElement.addChildElement( timeStretchElement );

        XmlElement* pitchCorrectionElement = new XmlElement( "pitch_correction" );
        pitchCorrectionElement->setAttribute( "checked", mUI->checkBox_PitchCorrection->isChecked() );
        docElement.addChildElement( pitchCorrectionElement );

        XmlElement* sampleElement = new XmlElement( "sample" );
        sampleElement->setAttribute( "filename", "audio.wav" );
        docElement.addChildElement( sampleElement );

        foreach ( SharedSampleRange sampleRange, mSampleRangeList )
        {
            XmlElement* rangeElement = new XmlElement( "sample_range" );
            rangeElement->setAttribute( "start_frame", sampleRange->startFrame );
            rangeElement->setAttribute( "num_frames", sampleRange->numFrames );
            docElement.addChildElement( rangeElement );
        }

        File file( xmlFilePath.toLocal8Bit().data() );

        docElement.writeToFile( file, String::empty );

        mUndoStack.setClean();

        mCurrentProjDirPath = projDirPath;

        QApplication::restoreOverrideCursor();
    }
    else // An error occurred while writing the audio file
    {
        QApplication::restoreOverrideCursor();
        showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



void MainWindow::saveProjectDialog()
{
    // Save file dialog
    const QString filePath = QFileDialog::getSaveFileName( this, tr("Save Project"), mLastOpenedProjDir,
                                                           tr("Shuriken Project (*.*)") );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QDir projectDir( filePath );

        QDir parentDir( projectDir );
        parentDir.cdUp();

        QString projectName = projectDir.dirName();

        bool isOkToSave = true;

        // If the directory already exists, ask the user if it should be overwritten
        if ( projectDir.exists() )
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle( "Shuriken Beat Slicer" );
            msgBox.setIcon( QMessageBox::Question );
            msgBox.setText( "The directory \"" + projectName + "\"" + " already exists" );
            msgBox.setInformativeText( "Do you want to overwrite the contents of " + projectName + "?" );
            msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
            msgBox.setDefaultButton( QMessageBox::Ok );
            const int buttonFlag = msgBox.exec();

            if ( buttonFlag == QMessageBox::Ok )
            {
                foreach ( QString fileName, projectDir.entryList( QDir::NoDotAndDotDot ) )
                {
                    const bool isSuccessful = projectDir.remove( fileName );

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
            isOkToSave = parentDir.mkdir( projectName );
        }

        if ( isOkToSave )
        {
            saveProject( filePath );
            mLastOpenedProjDir = parentDir.absolutePath();
        }
        else
        {
            showWarningDialog( tr("Could not save project ") + "\"" + projectName + "\"",
                               tr("Failed to create directory ") + "\"" + filePath + "\"" );
        }
    }
}



void MainWindow::openProjectDialog()
{
    // Open file dialog
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Open Project"), mLastOpenedProjDir,
                                                           tr("Shuriken Project (shuriken.xml)") );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        openProject( filePath );
    }
}



//==================================================================================================
// Public Slots:

void MainWindow::reorderSampleRangeList( QList<int> oldOrderPositions, const int numPlacesMoved )
{
    const int numSelectedItems = oldOrderPositions.size();

    // If waveform items have been dragged to the left...
    if ( numPlacesMoved < 0 )
    {
        for ( int i = 0; i < numSelectedItems; i++ )
        {
            const int orderPos = oldOrderPositions.at( i );
            mSampleRangeList.move( orderPos, orderPos + numPlacesMoved );
        }
    }
    else // If waveform items have been dragged to the right...
    {
        const int lastIndex = numSelectedItems - 1;

        for ( int i = lastIndex; i >= 0; i-- )
        {
            const int orderPos = oldOrderPositions.at( i );
            mSampleRangeList.move( orderPos, orderPos + numPlacesMoved );
        }
    }

    mSamplerAudioSource->setSampleRanges( mSampleRangeList );
}



//==================================================================================================
// Private Slots:

void MainWindow::recordWaveformItemMove( QList<int> oldOrderPositions, const int numPlacesMoved )
{
    QUndoCommand* command = new MoveWaveformItemCommand( oldOrderPositions, numPlacesMoved, mUI->waveGraphicsView, this );
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
    if ( slicePointFrameNumList.size() > 0 && mSampleRangeList.size() == 1 )
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



void MainWindow::enableRealtimeControls( const bool isEnabled )
{
    if ( isEnabled ) // Realtime mode
    {
        mUI->checkBox_TimeStretch->setVisible( true );
        mUI->pushButton_Apply->setVisible( false );

        QObject::connect( mOptionsDialog.get(), SIGNAL( windowOptionChanged() ),
                          this, SLOT( resetSampler() ) );
    }
    else // Offline mode
    {
        mUI->checkBox_TimeStretch->setVisible( false );
        mUI->pushButton_Apply->setVisible( true );

        QObject::disconnect( mOptionsDialog.get(), SIGNAL( windowOptionChanged() ),
                             this, SLOT( resetSampler() ) );
    }

    resetSampler();
}



void MainWindow::resetSampler()
{
    if ( ! mCurrentSampleBuffer.isNull() && ! mCurrentSampleHeader.isNull() )
    {
        tearDownSampler();
        setUpSampler( mCurrentSampleBuffer, mCurrentSampleHeader );
    }
}



void MainWindow::enableEditActions()
{
    const SharedSlicePointItem slicePointItem = mUI->waveGraphicsView->getSelectedSlicePoint();

    if ( ! slicePointItem.isNull() )
    {
        mUI->actionDelete->setEnabled( true );
    }
    else
    {
        mUI->actionDelete->setEnabled( false );
    }


    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    if ( ! orderPositions.isEmpty() )
    {
        mUI->actionApply_Gain->setEnabled( true );
        mUI->actionReverse->setEnabled( true );

        bool isAnySelectedItemJoined = false;
        foreach ( int orderPos, orderPositions )
        {
            if ( mUI->waveGraphicsView->getWaveformAt( orderPos )->isJoined() )
            {
                isAnySelectedItemJoined = true;
            }
        }
        mUI->actionSplit->setEnabled( isAnySelectedItemJoined );
    }
    else
    {
        mUI->actionApply_Gain->setEnabled( false );
        mUI->actionReverse->setEnabled( false );
        mUI->actionSplit->setEnabled( false );
    }

    if ( orderPositions.size() > 1 )
    {
        mUI->actionJoin->setEnabled( true );
    }
    else
    {
        mUI->actionJoin->setEnabled( false );
    }
}



//====================
// "File" menu:

void MainWindow::on_actionOpen_Project_triggered()
{
    // Check for unsaved changes before continuing
    if ( mUndoStack.isClean() )
    {
        openProjectDialog();
    }
    else
    {
        const int buttonClicked = showUnsavedChangesDialog();

        switch ( buttonClicked )
        {
        case QMessageBox::Save:
            on_actionSave_Project_triggered();
            openProjectDialog();
            break;
        case QMessageBox::Discard:
            openProjectDialog();
            break;
        case QMessageBox::Cancel:
            // Do nothing
            break;
        default:
            // Should never be reached
            break;
        }
    }
}



void MainWindow::on_actionSave_Project_triggered()
{
    if ( ! mCurrentSampleBuffer.isNull() )
    {
        if ( mCurrentProjDirPath.isEmpty() )
        {
            saveProjectDialog();
        }
        else
        {
            saveProject( mCurrentProjDirPath );
        }
    }
}



void MainWindow::on_actionSave_As_triggered()
{
    if ( ! mCurrentSampleBuffer.isNull() )
    {
        saveProjectDialog();
    }
}



void MainWindow::on_actionClose_Project_triggered()
{
    // Check for unsaved changes before continuing
    if ( mUndoStack.isClean() )
    {
        closeProject();
    }
    else
    {
        const int buttonClicked = showUnsavedChangesDialog();

        switch ( buttonClicked )
        {
        case QMessageBox::Save:
            on_actionSave_Project_triggered();
            closeProject();
            break;
        case QMessageBox::Discard:
            closeProject();
            break;
        case QMessageBox::Cancel:
            // Do nothing
            break;
        default:
            // Should never be reached
            break;
        }
    }
}



void MainWindow::on_actionImport_Audio_File_triggered()
{
    // Check for unsaved changes before continuing
    if ( mUndoStack.isClean() )
    {
        importAudioFile();
    }
    else
    {
        const int buttonClicked = showUnsavedChangesDialog();

        switch ( buttonClicked )
        {
        case QMessageBox::Save:
            on_actionSave_Project_triggered();
            importAudioFile();
            break;
        case QMessageBox::Discard:
            importAudioFile();
            break;
        case QMessageBox::Cancel:
            // Do nothing
            break;
        default:
            // Should never be reached
            break;
        }
    }
}



void MainWindow::on_actionExport_As_triggered()
{

}



void MainWindow::on_actionQuit_triggered()
{
    // Check for unsaved changes before continuing
    if ( mUndoStack.isClean() )
    {
        QCoreApplication::quit();
    }
    else
    {
        const int buttonClicked = showUnsavedChangesDialog();

        switch ( buttonClicked )
        {
        case QMessageBox::Save:
            on_actionSave_Project_triggered();
            QCoreApplication::quit();
            break;
        case QMessageBox::Discard:
            QCoreApplication::quit();
            break;
        case QMessageBox::Cancel:
            // Do nothing
            break;
        default:
            // Should never be reached
            break;
        }
    }
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
    mUI->waveGraphicsView->selectAll();
}



void MainWindow::on_actionSelect_None_triggered()
{
    mUI->waveGraphicsView->selectNone();
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
    const QString tempDirPath = mOptionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        ApplyGainDialog dialog;

        if ( dialog.exec() == QDialog::Accepted )
        {
            const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName;
                fileBaseName.setNum( mUndoStack.index() );

                QUndoCommand* command = new ApplyGainCommand( dialog.getGainValue(),
                                                              orderPos,
                                                              mUI->waveGraphicsView,
                                                              mCurrentSampleHeader,
                                                              mFileHandler,
                                                              tempDirPath,
                                                              fileBaseName );
                mUndoStack.push( command );
            }
        }
    }
    else
    {
        showWarningDialog( tr("Temp dir invalid!"),
                           tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
    }
}



void MainWindow::on_actionEnvelope_triggered()
{

}



void MainWindow::on_actionJoin_triggered()
{
    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    QUndoCommand* command = new JoinCommand( orderPositions, mUI->waveGraphicsView, this );
    mUndoStack.push( command );
}



void MainWindow::on_actionSplit_triggered()
{
    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    for ( int i = orderPositions.size() - 1; i >= 0; i-- )
    {
        const int orderPos = orderPositions.at( i );

        if ( mUI->waveGraphicsView->getWaveformAt( orderPos )->isJoined() )
        {
            QUndoCommand* command = new SplitCommand( orderPos, mUI->waveGraphicsView, this );
            mUndoStack.push( command );
        }
    }
}



void MainWindow::on_actionReverse_triggered()
{
    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    foreach ( int orderPos, orderPositions )
    {
        QUndoCommand* command = new ReverseCommand( orderPos, mUI->waveGraphicsView );
        mUndoStack.push( command );
    }
}



void MainWindow::on_actionNormalise_triggered()
{

}



//====================
// "Options" menu:

void MainWindow::on_actionOptions_triggered()
{
    QPoint pos = mOptionsDialog->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    mOptionsDialog->move( pos );
    mOptionsDialog->setCurrentTab( AudioSetupDialog::AUDIO_SETUP );
    mOptionsDialog->show();
}



//====================
// "Help" menu:

void MainWindow::on_actionHelp_triggered()
{
    QPoint pos = mHelpForm->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    mHelpForm->move( pos );
    mHelpForm->show();
}



void MainWindow::on_actionAbout_triggered()
{

}



//====================
// Main window widgets:

void MainWindow::on_pushButton_CalcBPM_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    AudioAnalyser::DetectionSettings settings;
    getDetectionSettings( settings );

    const qreal bpm = AudioAnalyser::calcBPM( mCurrentSampleBuffer, settings );

    mUI->doubleSpinBox_OriginalBPM->setValue( bpm );
    mUI->doubleSpinBox_NewBPM->setValue( bpm );

    if ( mRubberbandAudioSource != NULL )
    {
        mRubberbandAudioSource->setOriginalBPM( bpm );
    }

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_pushButton_Slice_clicked()
{
    QUndoCommand* command = new SliceCommand( this,
                                              mUI->waveGraphicsView,
                                              mUI->pushButton_Slice,
                                              mUI->actionAdd_Slice_Point,
                                              mUI->actionMove,
                                              mUI->actionSelect,
                                              mUI->actionAudition );
    mUndoStack.push( command );
}



void MainWindow::on_horizontalSlider_Threshold_valueChanged( const int value )
{
    mUI->lcdNumber_Threshold->display( qreal( value ) / 1000.0 );
}



void MainWindow::on_pushButton_FindOnsets_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    AudioAnalyser::DetectionSettings settings;
    getDetectionSettings( settings );

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

    AudioAnalyser::DetectionSettings settings;
    getDetectionSettings( settings );

    const QList<int> slicePointFrameNumList = AudioAnalyser::findBeatFrameNums( mCurrentSampleBuffer, settings );

    QUndoCommand* command = new AddSlicePointItemsCommand( mUI->pushButton_FindOnsets, mUI->pushButton_FindBeats );

    foreach ( int frameNum, slicePointFrameNumList )
    {
        new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice, command );
    }
    mUndoStack.push( command );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_doubleSpinBox_OriginalBPM_valueChanged( const double originalBPM )
{
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();

    if ( isTimeStretchEnabled && mRubberbandAudioSource != NULL )
    {
        mRubberbandAudioSource->setOriginalBPM( originalBPM );

        if ( ! mOptionsDialog->isJackSyncEnabled() && newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal timeRatio = originalBPM / newBPM;

            mRubberbandAudioSource->setTimeRatio( timeRatio );
        }
    }
}



void MainWindow::on_doubleSpinBox_NewBPM_valueChanged( const double newBPM )
{
    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();

    if ( isTimeStretchEnabled && mRubberbandAudioSource != NULL )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal timeRatio = originalBPM / newBPM;

            mRubberbandAudioSource->setTimeRatio( timeRatio );
        }
    }
}



void MainWindow::on_checkBox_TimeStretch_toggled( const bool isChecked )
{
    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = isChecked;

    if ( mRubberbandAudioSource != NULL )
    {
        qreal timeRatio = 1.0;
        bool isPitchCorrectionEnabled = true;

        if ( isTimeStretchEnabled )
        {
            if ( newBPM > 0.0 && originalBPM > 0.0 )
                timeRatio = originalBPM / newBPM;
            isPitchCorrectionEnabled = mUI->checkBox_PitchCorrection->isChecked();
        }

        mRubberbandAudioSource->setTimeRatio( timeRatio );
        mRubberbandAudioSource->enablePitchCorrection( isPitchCorrectionEnabled );
    }
}



void MainWindow::on_checkBox_PitchCorrection_toggled( const bool isChecked )
{
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();

    if ( isTimeStretchEnabled && mRubberbandAudioSource != NULL )
    {
        mRubberbandAudioSource->enablePitchCorrection( isChecked );
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
        const QString tempDirPath = mOptionsDialog->getTempDirPath();

        if ( ! tempDirPath.isEmpty() )
        {
            QString fileBaseName;
            fileBaseName.setNum( mUndoStack.index() );

            QUndoCommand* command = new ApplyTimeStretchCommand( this,
                                                                 mUI->waveGraphicsView,
                                                                 mUI->doubleSpinBox_OriginalBPM,
                                                                 mUI->doubleSpinBox_NewBPM,
                                                                 mUI->checkBox_PitchCorrection,
                                                                 tempDirPath,
                                                                 fileBaseName );
            mUndoStack.push( command );
        }
        else
        {
            showWarningDialog( tr("Temp dir invalid!"),
                               tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
        }
    }
}



void MainWindow::on_actionMove_triggered()
{
    mUI->waveGraphicsView->setInteractionMode( WaveGraphicsView::MOVE_ITEMS );
}



void MainWindow::on_actionSelect_triggered()
{
    mUI->waveGraphicsView->setInteractionMode( WaveGraphicsView::SELECT_ITEMS );
}



void MainWindow::on_actionAudition_triggered()
{
    mUI->waveGraphicsView->setInteractionMode( WaveGraphicsView::AUDITION_ITEMS );
}



void MainWindow::on_pushButton_TimestretchOptions_clicked()
{
    QPoint pos = mOptionsDialog->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    mOptionsDialog->move( pos );
    mOptionsDialog->setCurrentTab( AudioSetupDialog::TIMESTRETCH );
    mOptionsDialog->show();
}
