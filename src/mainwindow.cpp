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
#include <QDesktopWidget>
#include "commands.h"
#include "globals.h"
#include "applygaindialog.h"
#include "applygainrampdialog.h"
#include "aboutdialog.h"
#include "messageboxes.h"
#include <rubberband/RubberBandStretcher.h>
//#include <QDebug>


using namespace RubberBand;


//==================================================================================================
// Public:

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    mUI( new Ui::MainWindow ),
    mLastOpenedImportDir( QDir::homePath() ),
    mLastOpenedProjDir( QDir::homePath() ),
    mAppliedBPM( 0.0 ),
    mIsProjectOpen( false )
{
    setupUI();
    initialiseAudio();
}



MainWindow::~MainWindow()
{
    closeProject();

    if ( mOptionsDialog != NULL)
    {
        const QString tempDirPath = mOptionsDialog->getTempDirPath();

        if ( ! tempDirPath.isEmpty() )
        {
            File( tempDirPath.toLocal8Bit().data() ).deleteRecursively();
        }
    }

    delete mUI;
}



void MainWindow::connectWaveformToMainWindow( const SharedWaveformItem item )
{
    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( recordWaveformItemMove(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( reorderSampleBufferList(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( stopPlayback() ) );

    QObject::connect( item.data(), SIGNAL( clicked(const WaveformItem*,QPointF) ),
                      this, SLOT( playSampleRange(const WaveformItem*,QPointF) ) );
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
        const int buttonClicked = MessageBoxes::showUnsavedChangesDialog();

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



void MainWindow::keyPressEvent( QKeyEvent* event )
{
    if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() && mSamplerAudioSource != NULL )
    {
        on_pushButton_PlayStop_clicked();
    }
    else
    {
        QMainWindow::keyPressEvent( event );
    }
}



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
    const String error = mDeviceManager.initialise( NUM_INPUT_CHANS, NUM_OUTPUT_CHANS, stateXml, false );

    if ( error.isNotEmpty() )
    {
        MessageBoxes::showWarningDialog( tr( "Error initialising audio device manager!" ), error.toRawUTF8() );

        mDeviceManager.setCurrentAudioDeviceType( "ALSA", true );
    }

    mOptionsDialog = new OptionsDialog( mDeviceManager );

    if ( mOptionsDialog != NULL )
    {
        // Centre form in desktop
        mOptionsDialog->setGeometry
        (
            QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, mOptionsDialog->size(), QApplication::desktop()->availableGeometry() )
        );

        QObject::connect( mOptionsDialog, SIGNAL( realtimeModeToggled(bool) ),
                          this, SLOT( enableRealtimeControls(bool) ) );

        QObject::connect( mOptionsDialog, SIGNAL( jackSyncToggled(bool) ),
                          mUI->doubleSpinBox_NewBPM, SLOT( setHidden(bool) ) );

        QObject::connect( mOptionsDialog, SIGNAL( jackSyncToggled(bool) ),
                          mUI->label_JackSync, SLOT( setVisible(bool) ) );

        QObject::connect( mOptionsDialog, SIGNAL( timeStretchOptionsChanged() ),
                          this, SLOT( enableSaveAction() ) );
    }

    // Check if any errors occurred while the audio file handler was being initialised
    if ( ! mFileHandler.getLastErrorTitle().isEmpty() )
    {
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



void MainWindow::setUpSampler()
{
    mSamplerAudioSource = new SamplerAudioSource();

    if ( ! mSampleBufferList.isEmpty() && ! mSampleHeader.isNull() )
    {
        mSamplerAudioSource->setSamples( mSampleBufferList, mSampleHeader->sampleRate );
    }

    on_pushButton_Loop_clicked( mUI->pushButton_Loop->isChecked() );

    if ( mOptionsDialog->isRealtimeModeEnabled() ) // Realtime timestretch mode
    {
        const int numChans = mSampleHeader->numChans;
        const RubberBandStretcher::Options options = mOptionsDialog->getStretcherOptions();
        const bool isJackSyncEnabled = mOptionsDialog->isJackSyncEnabled();

        mRubberbandAudioSource = new RubberbandAudioSource( mSamplerAudioSource, numChans, options, isJackSyncEnabled );
        mAudioSourcePlayer.setSource( mRubberbandAudioSource );

        QObject::connect( mOptionsDialog, SIGNAL( transientsOptionChanged(RubberBandStretcher::Options) ),
                          mRubberbandAudioSource, SLOT( setTransientsOption(RubberBandStretcher::Options) ) );

        QObject::connect( mOptionsDialog, SIGNAL( phaseOptionChanged(RubberBandStretcher::Options) ),
                          mRubberbandAudioSource, SLOT( setPhaseOption(RubberBandStretcher::Options) ) );

        QObject::connect( mOptionsDialog, SIGNAL( formantOptionChanged(RubberBandStretcher::Options) ),
                          mRubberbandAudioSource, SLOT( setFormantOption(RubberBandStretcher::Options) ) );

        QObject::connect( mOptionsDialog, SIGNAL( pitchOptionChanged(RubberBandStretcher::Options) ),
                          mRubberbandAudioSource, SLOT( setPitchOption(RubberBandStretcher::Options) ) );

        QObject::connect( mOptionsDialog, SIGNAL( jackSyncToggled(bool) ),
                          mRubberbandAudioSource, SLOT( enableJackSync(bool) ) );

        on_checkBox_TimeStretch_toggled( mUI->checkBox_TimeStretch->isChecked() );
    }
    else // Offline timestretch mode
    {
        mAudioSourcePlayer.setSource( mSamplerAudioSource );
    }

    mDeviceManager.addAudioCallback( &mAudioSourcePlayer );
    mDeviceManager.addMidiInputCallback( String::empty, mSamplerAudioSource->getMidiInputCallback() );
}



void MainWindow::tearDownSampler()
{
    stopPlayback();

    mAudioSourcePlayer.setSource( NULL );

    mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );
    mDeviceManager.removeMidiInputCallback( String::empty, mSamplerAudioSource->getMidiInputCallback() );

    mRubberbandAudioSource = NULL;
    mSamplerAudioSource = NULL;
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


    // Populate "Snap Loop Markers" combo box
    updateSnapLoopMarkersComboBox();


    // Populate "Time Signature" combo boxes
    QStringList timeSigNumeratorTextList;
    QStringList timeSigDenominatorTextList;

    timeSigNumeratorTextList << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    timeSigDenominatorTextList << "1" << "2" << "4" << "8" << "16";

    mUI->comboBox_TimeSigNumerator->addItems( timeSigNumeratorTextList );
    mUI->comboBox_TimeSigDenominator->addItems( timeSigDenominatorTextList );

    mUI->comboBox_TimeSigNumerator->setCurrentIndex( 3 );   // 4
    mUI->comboBox_TimeSigDenominator->setCurrentIndex( 2 ); // 4


    // Populate "Units" combo box
    QStringList unitsTextList;

    unitsTextList << "Bars" << "Beats";

    mUI->comboBox_Units->addItems( unitsTextList );


    // Hide widgets
    mUI->label_JackSync->setVisible( false );
    mUI->checkBox_TimeStretch->setVisible( false );


    // Connect signals to slots
    QObject::connect( mUI->waveGraphicsView, SIGNAL( slicePointOrderChanged(SharedSlicePointItem,int,int) ),
                      this, SLOT( recordSlicePointItemMove(SharedSlicePointItem,int,int) ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( loopMarkerPosChanged() ),
                      this, SLOT( stopPlayback() ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( minDetailLevelReached() ),
                      this, SLOT( disableZoomOut() ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( disableZoomIn() ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( playheadFinishedScrolling() ),
                      this, SLOT( resetPlayStopButtonIcon() ) );

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
        // Make sure help form isn't larger than desktop
        const int desktopWidth = QApplication::desktop()->availableGeometry().width();
        const int desktopHeight = QApplication::desktop()->availableGeometry().height();

        const int frameWidth = mHelpForm->frameSize().width();
        const int frameHeight = mHelpForm->frameSize().height();

        int formWidth = mHelpForm->size().width();
        int formHeight = mHelpForm->size().height();

        int maxWidth = mHelpForm->maximumWidth();
        int maxHeight = mHelpForm->maximumHeight();

        if ( frameWidth > desktopWidth )
        {
            formWidth = desktopWidth - ( frameWidth - formWidth );
            maxWidth = formWidth;
        }

        if ( frameHeight > desktopHeight )
        {
            formHeight = desktopHeight - ( frameHeight - formHeight );
            maxHeight = formHeight;
        }

        mHelpForm->resize( formWidth, formHeight );
        mHelpForm->setMaximumSize( maxWidth, maxHeight );

        // Centre form in desktop
        mHelpForm->setGeometry
        (
            QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, mHelpForm->size(), QApplication::desktop()->availableGeometry() )
        );

        mUI->actionHelp->setEnabled( true );
    }


    // Create export dialog
    mExportDialog = new ExportDialog();

    if ( mExportDialog != NULL )
    {
        // Centre form in desktop
        mExportDialog->setGeometry
        (
            QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, mExportDialog->size(), QApplication::desktop()->availableGeometry() )
        );
    }
}



void MainWindow::enableUI()
{
    mUI->pushButton_PlayStop->setEnabled( true );
    mUI->pushButton_Loop->setEnabled( true );
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
    mUI->checkBox_LoopMarkers->setEnabled( true );
    mUI->comboBox_TimeSigNumerator->setEnabled( true );
    mUI->comboBox_TimeSigDenominator->setEnabled( true );
    mUI->spinBox_Length->setEnabled( true );
    mUI->comboBox_Units->setEnabled( true );

    mUI->actionSave_As->setEnabled( true );
    mUI->actionClose_Project->setEnabled( true );
    mUI->actionExport_As->setEnabled( true );
    mUI->actionSelect_All->setEnabled( true );
    mUI->actionSelect_None->setEnabled( true );
    mUI->actionAdd_Slice_Point->setEnabled( true );
    mUI->actionZoom_Original->setEnabled( true );
    mUI->actionZoom_In->setEnabled( true );
    mUI->actionMove->setEnabled( true );
    mUI->actionAudition->setEnabled( true );

    mUI->actionAudition->trigger();
}



void MainWindow::disableUI()
{
    mUI->pushButton_PlayStop->setEnabled( false );
    mUI->pushButton_Loop->setEnabled( false );
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
    mUI->checkBox_LoopMarkers->setEnabled( false );
    mUI->checkBox_LoopMarkers->setChecked( false );
    mUI->comboBox_SnapLoopMarkers->setEnabled( false );
    mUI->comboBox_TimeSigNumerator->setEnabled( false );
    mUI->comboBox_TimeSigDenominator->setEnabled( false );
    mUI->spinBox_Length->setEnabled( false );
    mUI->spinBox_Length->setValue( 0 );
    mUI->comboBox_Units->setEnabled( false );

    mUI->actionSave_Project->setEnabled( false );
    mUI->actionSave_As->setEnabled( false );
    mUI->actionClose_Project->setEnabled( false );
    mUI->actionExport_As->setEnabled( false );
    mUI->actionSelect_All->setEnabled( false );
    mUI->actionSelect_None->setEnabled( false );
    mUI->actionAdd_Slice_Point->setEnabled( false );
    mUI->actionDelete->setEnabled( false );
    mUI->actionJoin->setEnabled( false );
    mUI->actionReverse->setEnabled( false );
    mUI->actionZoom_Original->setEnabled( false );
    mUI->actionZoom_Out->setEnabled( false );
    mUI->actionZoom_In->setEnabled( false );
    mUI->actionMove->setEnabled( false );
    mUI->actionSelect->setEnabled( false );
    mUI->actionAudition->setEnabled( false );
}



void MainWindow::updateSnapLoopMarkersComboBox()
{
    QStringList snapTextList;

    if ( mSampleBufferList.size() > 1 )
    {
        snapTextList << "Off" << "Markers -> Slice Points";
    }
    else
    {
        snapTextList << "Off" << "Markers -> Slice Points" << "Slice Points -> Markers";
    }

    mUI->comboBox_SnapLoopMarkers->clear();
    mUI->comboBox_SnapLoopMarkers->addItems( snapTextList );
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

    settings.sampleRate = (uint_t) mSampleHeader->sampleRate;
}



void MainWindow::closeProject()
{
    mSampleHeader.clear();
    mSampleBufferList.clear();
    mLoopSampleBufferList.clear();
    tearDownSampler();

    mUI->waveGraphicsView->clearAll();
    on_actionZoom_Original_triggered();
    disableUI();
    mUI->statusBar->clearMessage();

    mUndoStack.clear();

    mAppliedBPM = 0.0;

    mCurrentProjectFilePath.clear();
    mIsProjectOpen = false;
}



//==================================================================================================
// Public Slots:

void MainWindow::reorderSampleBufferList( QList<int> oldOrderPositions, const int numPlacesMoved )
{
    const int numSelectedItems = oldOrderPositions.size();

    // If waveform items have been dragged to the left...
    if ( numPlacesMoved < 0 )
    {
        for ( int i = 0; i < numSelectedItems; i++ )
        {
            const int orderPos = oldOrderPositions.at( i );
            mSampleBufferList.move( orderPos, orderPos + numPlacesMoved );
        }
    }
    else // If waveform items have been dragged to the right...
    {
        const int lastIndex = numSelectedItems - 1;

        for ( int i = lastIndex; i >= 0; i-- )
        {
            const int orderPos = oldOrderPositions.at( i );
            mSampleBufferList.move( orderPos, orderPos + numPlacesMoved );
        }
    }

    mSamplerAudioSource->setSamples( mSampleBufferList, mSampleHeader->sampleRate );
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



void MainWindow::playSampleRange( const WaveformItem* waveformItem, const QPointF mouseScenePos )
{
    SharedSampleRange sampleRange( new SampleRange );
    sampleRange->startFrame = 0;
    sampleRange->numFrames = waveformItem->getSampleBuffer()->getNumFrames();

    qreal startPosX = waveformItem->scenePos().x();
    qreal endPosX = startPosX + waveformItem->rect().width();

    const QList<int> slicePointFrameNums = mUI->waveGraphicsView->getSlicePointFrameNums();

    // If slice points are present and the waveform has not yet been sliced...
    if ( slicePointFrameNums.size() > 0 && mSampleBufferList.size() == 1 )
    {
        const int mousePosFrameNum = mUI->waveGraphicsView->getFrameNum( mouseScenePos.x() );
        int endFrame = sampleRange->numFrames;

        foreach ( int frameNum, slicePointFrameNums )
        {
            if ( frameNum <= mousePosFrameNum )
            {
                sampleRange->startFrame = frameNum;
            }
            else
            {
                endFrame = frameNum;
                break;
            }
        }

        sampleRange->numFrames = endFrame - sampleRange->startFrame;

        startPosX = mUI->waveGraphicsView->getScenePosX( sampleRange->startFrame );
        endPosX = mUI->waveGraphicsView->getScenePosX( endFrame );
    }

    // Play sample range and start playhead scrolling
    mSamplerAudioSource->playSample( waveformItem->getOrderPos(), sampleRange );
    mUI->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-stop.png" ) );

    if ( mOptionsDialog != NULL && mOptionsDialog->isRealtimeModeEnabled() &&
         mUI->checkBox_TimeStretch->isChecked() &&
         mUI->doubleSpinBox_OriginalBPM->value() > 0.0 &&
         mUI->doubleSpinBox_NewBPM->value() > 0.0 )
    {
        qreal stretchRatio = mUI->doubleSpinBox_OriginalBPM->value() / mUI->doubleSpinBox_NewBPM->value();
        mUI->waveGraphicsView->startPlayhead( startPosX, endPosX, sampleRange->numFrames, stretchRatio );
    }
    else
    {
        mUI->waveGraphicsView->startPlayhead( startPosX, endPosX, sampleRange->numFrames );
    }
}



void MainWindow::stopPlayback()
{
    if ( mSamplerAudioSource != NULL )
    {
        mSamplerAudioSource->stop();
    }
    mUI->waveGraphicsView->stopPlayhead();
    mUI->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
}



void MainWindow::resetPlayStopButtonIcon()
{
    mUI->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
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

        QObject::connect( mOptionsDialog, SIGNAL( windowOptionChanged() ),
                          this, SLOT( resetSampler() ) );
    }
    else // Offline mode
    {
        mUI->checkBox_TimeStretch->setVisible( false );
        mUI->pushButton_Apply->setVisible( true );

        QObject::disconnect( mOptionsDialog, SIGNAL( windowOptionChanged() ),
                             this, SLOT( resetSampler() ) );
    }

    resetSampler();
}



void MainWindow::resetSampler()
{
    tearDownSampler();
    setUpSampler();
}



void MainWindow::enableEditActions()
{
    const SharedSlicePointItem slicePoint = mUI->waveGraphicsView->getSelectedSlicePoint();
    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    mUI->actionDelete->setEnabled( false );

    if ( ! slicePoint.isNull() || ! orderPositions.isEmpty() )
    {
        if ( orderPositions.size() < mUI->waveGraphicsView->getNumWaveformItems() )
        {
            mUI->actionDelete->setEnabled( true );
        }
    }

    if ( ! orderPositions.isEmpty() )
    {
        mUI->actionApply_Gain->setEnabled( true );
        mUI->actionApply_Gain_Ramp->setEnabled( true );
        mUI->actionNormalise->setEnabled( true );
        mUI->actionReverse->setEnabled( true );
    }
    else
    {
        mUI->actionApply_Gain->setEnabled( false );
        mUI->actionApply_Gain_Ramp->setEnabled( false );
        mUI->actionNormalise->setEnabled( false );
        mUI->actionReverse->setEnabled( false );
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



void MainWindow::enableSaveAction()
{
    if ( mIsProjectOpen )
    {
        mUI->actionSave_Project->setEnabled( true );
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
        const int buttonClicked = MessageBoxes::showUnsavedChangesDialog();

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
    if ( ! mSampleBufferList.isEmpty() )
    {
        if ( mCurrentProjectFilePath.isEmpty() )
        {
            saveProjectDialog();
        }
        else
        {
            saveProject( mCurrentProjectFilePath );
        }
    }
}



void MainWindow::on_actionSave_As_triggered()
{
    if ( ! mSampleBufferList.isEmpty() )
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
        const int buttonClicked = MessageBoxes::showUnsavedChangesDialog();

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
        importAudioFileDialog();
    }
    else
    {
        const int buttonClicked = MessageBoxes::showUnsavedChangesDialog();

        switch ( buttonClicked )
        {
        case QMessageBox::Save:
            on_actionSave_Project_triggered();
            importAudioFileDialog();
            break;
        case QMessageBox::Discard:
            importAudioFileDialog();
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
    exportAsDialog();
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
        const int buttonClicked = MessageBoxes::showUnsavedChangesDialog();

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
    else
    {
        const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

        if ( ! orderPositions.isEmpty() )
        {
            foreach ( int orderPos, orderPositions )
            {
                mUI->waveGraphicsView->getWaveformAt( orderPos )->setSelected( false );
            }

            QUndoCommand* command = new DeleteWaveformItemCommand( orderPositions,
                                                                   mUI->waveGraphicsView,
                                                                   this );
            mUndoStack.push( command );
        }
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

        const int result = dialog.exec();

        if ( result == QDialog::Accepted )
        {
            const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

            QUndoCommand* parentCommand = new QUndoCommand();

            const QString stackIndex = QString::number( mUndoStack.index() );
            int i = 0;

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName = stackIndex + "_" + QString::number( i++ );

                new ApplyGainCommand( dialog.getGainValue(),
                                      orderPos,
                                      mUI->waveGraphicsView,
                                      mSampleHeader->sampleRate,
                                      mFileHandler,
                                      tempDirPath,
                                      fileBaseName,
                                      parentCommand );
            }

            mUndoStack.push( parentCommand );
        }
    }
    else
    {
        MessageBoxes::showWarningDialog( tr( "Temp dir invalid!" ),
                                         tr( "This operation needs to save temporary files, please change \"Temp Dir\" in options" ) );
    }
}



void MainWindow::on_actionApply_Gain_Ramp_triggered()
{
    const QString tempDirPath = mOptionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        ApplyGainRampDialog dialog;

        const int result = dialog.exec();

        if ( result == QDialog::Accepted )
        {
            const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

            QUndoCommand* parentCommand = new QUndoCommand();

            const QString stackIndex = QString::number( mUndoStack.index() );
            int i = 0;

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName = stackIndex + "_" + QString::number( i++ );

                new ApplyGainRampCommand( dialog.getStartGainValue(),
                                          dialog.getEndGainValue(),
                                          orderPos,
                                          mUI->waveGraphicsView,
                                          mSampleHeader->sampleRate,
                                          mFileHandler,
                                          tempDirPath,
                                          fileBaseName,
                                          parentCommand );
            }

            mUndoStack.push( parentCommand );
        }
    }
    else
    {
        MessageBoxes::showWarningDialog( tr( "Temp dir invalid!" ),
                                         tr( "This operation needs to save temporary files, please change \"Temp Dir\" in options" ) );
    }
}



void MainWindow::on_actionNormalise_triggered()
{
    const QString tempDirPath = mOptionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

        QUndoCommand* parentCommand = new QUndoCommand();

        const QString stackIndex = QString::number( mUndoStack.index() );
        int i = 0;

        foreach ( int orderPos, orderPositions )
        {
            QString fileBaseName = stackIndex + "_" + QString::number( i++ );

            new NormaliseCommand( orderPos,
                                  mUI->waveGraphicsView,
                                  mSampleHeader->sampleRate,
                                  mFileHandler,
                                  tempDirPath,
                                  fileBaseName,
                                  parentCommand );
        }

        mUndoStack.push( parentCommand );
    }
    else
    {
        MessageBoxes::showWarningDialog( tr( "Temp dir invalid!" ),
                                         tr( "This operation needs to save temporary files, please change \"Temp Dir\" in options" ) );
    }
}



void MainWindow::on_actionEnvelope_triggered()
{
    // TODO
}



void MainWindow::on_actionJoin_triggered()
{
    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    QUndoCommand* command = new JoinCommand( orderPositions, mUI->waveGraphicsView, this );
    mUndoStack.push( command );
}



void MainWindow::on_actionReverse_triggered()
{
    const QList<int> orderPositions = mUI->waveGraphicsView->getSelectedWaveformsOrderPositions();

    QUndoCommand* parentCommand = new QUndoCommand();

    foreach ( int orderPos, orderPositions )
    {
        new ReverseCommand( orderPos, mUI->waveGraphicsView, parentCommand );
    }

    mUndoStack.push( parentCommand );
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
    mOptionsDialog->setCurrentTab( OptionsDialog::AUDIO_SETUP );
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
    AboutDialog dialog;
    dialog.exec();
}



//====================
// Main window widgets:

void MainWindow::on_pushButton_CalcBPM_clicked()
{
    qreal bpm = 0.0;
    int numFrames = 0;

    if ( mUI->checkBox_LoopMarkers->isChecked() )
    {
        numFrames = mUI->waveGraphicsView->getNumFramesBetweenLoopMarkers();
    }
    else
    {
        numFrames = SampleUtils::getTotalNumFrames( mSampleBufferList );
    }

    const qreal numSeconds = numFrames / mSampleHeader->sampleRate;
    const int numerator = mUI->comboBox_TimeSigNumerator->currentText().toInt();

    int numBeats = 0;

    if ( mUI->comboBox_Units->currentIndex() == UNITS_BARS )
    {
        numBeats = mUI->spinBox_Length->value() * numerator;
    }
    else // UNITS_BEATS
    {
        numBeats = mUI->spinBox_Length->value();
    }

    bpm = numBeats / ( numSeconds / 60 );

    mUI->doubleSpinBox_OriginalBPM->setValue( bpm );
    mUI->doubleSpinBox_NewBPM->setValue( bpm );

    if ( mRubberbandAudioSource != NULL && bpm > 0.0 )
    {
        mRubberbandAudioSource->setOriginalBPM( bpm );
    }
}



void MainWindow::on_pushButton_Slice_clicked()
{
    QUndoCommand* command = new SliceCommand( this,
                                              mUI->waveGraphicsView,
                                              mUI->pushButton_Slice,
                                              mUI->pushButton_FindOnsets,
                                              mUI->pushButton_FindBeats,
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

    QUndoCommand* parentCommand = new QUndoCommand();

    // Remove current slice points if present
    {
        const QList<SharedSlicePointItem> slicePointItemList = mUI->waveGraphicsView->getSlicePointList();

        foreach ( SharedSlicePointItem item, slicePointItemList )
        {
            new DeleteSlicePointItemCommand( item, mUI->waveGraphicsView, mUI->pushButton_Slice, parentCommand );
        }
    }

    // Add new slice points
    {
        AudioAnalyser::DetectionSettings settings;
        getDetectionSettings( settings );

        const QList<int> slicePointFrameNumList = AudioAnalyser::findOnsetFrameNums( mSampleBufferList.first(), settings );

        foreach ( int frameNum, slicePointFrameNumList )
        {
            new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice, parentCommand );
        }
    }

    mUndoStack.push( parentCommand );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_pushButton_FindBeats_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    AudioAnalyser::DetectionSettings settings;
    getDetectionSettings( settings );

    // Get list of frame numbers for slice points to be added
    const QList<int> slicePointFrameNumList = AudioAnalyser::findBeatFrameNums( mSampleBufferList.first(), settings );

    // Get list of slice point items to be removed
    const QList<SharedSlicePointItem> slicePointItemList = mUI->waveGraphicsView->getSlicePointList();

    QUndoCommand* parentCommand = new QUndoCommand();

    foreach ( SharedSlicePointItem item, slicePointItemList )
    {
        new DeleteSlicePointItemCommand( item, mUI->waveGraphicsView, mUI->pushButton_Slice, parentCommand );
    }

    foreach ( int frameNum, slicePointFrameNumList )
    {
        new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice, parentCommand );
    }

    mUndoStack.push( parentCommand );

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

            mUI->waveGraphicsView->updatePlayheadSpeed( timeRatio );
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

            mUI->waveGraphicsView->updatePlayheadSpeed( timeRatio );
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



void MainWindow::on_pushButton_PlayStop_clicked()
{
    if ( mUI->waveGraphicsView->isPlayheadScrolling() )
    {
        mSamplerAudioSource->stop();
        mUI->waveGraphicsView->stopPlayhead();
        mUI->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
    }
    else
    {
        if ( mUI->checkBox_LoopMarkers->isChecked() )
        {
            int firstOrderPos;
            QList<SharedSampleRange> sampleRanges;

            mUI->waveGraphicsView->getSampleRangesBetweenLoopMarkers( firstOrderPos, sampleRanges );

            mSamplerAudioSource->playSamples( firstOrderPos, sampleRanges );
        }
        else
        {
            mSamplerAudioSource->playAll();
        }
        
        mUI->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-stop.png" ) );

        if ( mOptionsDialog != NULL && mOptionsDialog->isRealtimeModeEnabled() &&
             mUI->checkBox_TimeStretch->isChecked() &&
             mUI->doubleSpinBox_OriginalBPM->value() > 0.0 &&
             mUI->doubleSpinBox_NewBPM->value() > 0.0 )
        {
            qreal stretchRatio = mUI->doubleSpinBox_OriginalBPM->value() / mUI->doubleSpinBox_NewBPM->value();
            mUI->waveGraphicsView->startPlayhead( mUI->pushButton_Loop->isChecked(), stretchRatio );
        }
        else
        {
            mUI->waveGraphicsView->startPlayhead( mUI->pushButton_Loop->isChecked() );
        }
    }
}



void MainWindow::on_pushButton_Loop_clicked( const bool isChecked )
{
    if ( mSamplerAudioSource != NULL )
    {
        mSamplerAudioSource->setLooping( isChecked );
    }
    mUI->waveGraphicsView->setPlayheadLooping( isChecked );
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
    const qreal originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = mUI->doubleSpinBox_NewBPM->value();

    if ( newBPM > 0.0 && originalBPM > 0.0 )
    {
        const QString tempDirPath = mOptionsDialog->getTempDirPath();

        if ( ! tempDirPath.isEmpty() )
        {
            const QString fileBaseName = QString::number( mUndoStack.index() );

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
            MessageBoxes::showWarningDialog( tr("Temp dir invalid!"),
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
    mOptionsDialog->setCurrentTab( OptionsDialog::TIMESTRETCH );
    mOptionsDialog->show();
}



void MainWindow::on_checkBox_LoopMarkers_clicked( const bool isChecked )
{
    stopPlayback();

    if ( isChecked )
    {
        mUI->waveGraphicsView->showLoopMarkers();
        mUI->comboBox_SnapLoopMarkers->setEnabled( true );
    }
    else
    {
        mUI->waveGraphicsView->hideLoopMarkers();
        mUI->comboBox_SnapLoopMarkers->setEnabled( false );
    }
}



void MainWindow::on_comboBox_SnapLoopMarkers_currentIndexChanged( const int index )
{
    switch ( index )
    {
    case WaveGraphicsView::SNAP_OFF :
        mUI->waveGraphicsView->setLoopMarkerSnapMode( WaveGraphicsView::SNAP_OFF );
        break;
    case WaveGraphicsView::SNAP_MARKERS_TO_SLICES :
        mUI->waveGraphicsView->setLoopMarkerSnapMode( WaveGraphicsView::SNAP_MARKERS_TO_SLICES );
        break;
    case WaveGraphicsView::SNAP_SLICES_TO_MARKERS :
        mUI->waveGraphicsView->setLoopMarkerSnapMode( WaveGraphicsView::SNAP_SLICES_TO_MARKERS );
        break;
    default:
        break;
    }
}
