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
#include <QDebug>


using namespace RubberBand;


//==================================================================================================
// Public:

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    m_ui( new Ui::MainWindow ),
    m_lastOpenedImportDir( QDir::homePath() ),
    m_lastOpenedProjDir( QDir::homePath() ),
    m_appliedBPM( 0.0 ),
    m_isProjectOpen( false )
{
    initialiseAudio();
    setupUI();
}



MainWindow::~MainWindow()
{
    closeProject();

    if ( m_optionsDialog != NULL)
    {
        const QString tempDirPath = m_optionsDialog->getTempDirPath();

        if ( ! tempDirPath.isEmpty() )
        {
            File( tempDirPath.toLocal8Bit().data() ).deleteRecursively();
        }
    }

    delete m_ui;
}



//==================================================================================================
// Protected:

void MainWindow::changeEvent( QEvent* event )
{
    QMainWindow::changeEvent( event );
    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;
    default:
        break;
    }
}



void MainWindow::closeEvent( QCloseEvent* event )
{
    // Check for unsaved changes before continuing
    if ( m_undoStack.isClean() )
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
    if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() && m_samplerAudioSource != NULL )
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
    const String error = m_deviceManager.initialise( NUM_INPUT_CHANS, NUM_OUTPUT_CHANS, stateXml, false );

    if ( error.isNotEmpty() )
    {
        MessageBoxes::showWarningDialog( tr( "Error initialising audio device manager!" ), error.toRawUTF8() );

        m_deviceManager.setCurrentAudioDeviceType( "ALSA", true );
    }

    // Check if any errors occurred while the audio file handler was being initialised
    if ( ! m_fileHandler.getLastErrorTitle().isEmpty() )
    {
        MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
    }
}



void MainWindow::setUpSampler()
{
    m_samplerAudioSource = new SamplerAudioSource();

    if ( ! m_sampleBufferList.isEmpty() && ! m_sampleHeader.isNull() )
    {
        m_samplerAudioSource->setSamples( m_sampleBufferList, m_sampleHeader->sampleRate );
    }

    on_pushButton_Loop_clicked( m_ui->pushButton_Loop->isChecked() );

    if ( m_optionsDialog->isRealtimeModeEnabled() ) // Real-time time stretch mode
    {
        const int numChans = m_sampleHeader->numChans;
        const RubberBandStretcher::Options options = m_optionsDialog->getStretcherOptions();
        const bool isJackSyncEnabled = m_optionsDialog->isJackSyncEnabled();

        m_rubberbandAudioSource = new RubberbandAudioSource( m_samplerAudioSource, numChans, options, isJackSyncEnabled );
        m_audioSourcePlayer.setSource( m_rubberbandAudioSource );

        QObject::connect( m_optionsDialog, SIGNAL( transientsOptionChanged(RubberBandStretcher::Options) ),
                          m_rubberbandAudioSource, SLOT( setTransientsOption(RubberBandStretcher::Options) ) );

        QObject::connect( m_optionsDialog, SIGNAL( phaseOptionChanged(RubberBandStretcher::Options) ),
                          m_rubberbandAudioSource, SLOT( setPhaseOption(RubberBandStretcher::Options) ) );

        QObject::connect( m_optionsDialog, SIGNAL( formantOptionChanged(RubberBandStretcher::Options) ),
                          m_rubberbandAudioSource, SLOT( setFormantOption(RubberBandStretcher::Options) ) );

        QObject::connect( m_optionsDialog, SIGNAL( pitchOptionChanged(RubberBandStretcher::Options) ),
                          m_rubberbandAudioSource, SLOT( setPitchOption(RubberBandStretcher::Options) ) );

        QObject::connect( m_optionsDialog, SIGNAL( jackSyncToggled(bool) ),
                          m_rubberbandAudioSource, SLOT( enableJackSync(bool) ) );

        on_checkBox_TimeStretch_toggled( m_ui->checkBox_TimeStretch->isChecked() );
    }
    else // Offline time stretch mode
    {
        m_audioSourcePlayer.setSource( m_samplerAudioSource );
    }

    m_deviceManager.addAudioCallback( &m_audioSourcePlayer );
    m_deviceManager.addMidiInputCallback( String::empty, m_samplerAudioSource->getMidiInputCallback() );
}



void MainWindow::tearDownSampler()
{
    stopPlayback();

    m_audioSourcePlayer.setSource( NULL );

    m_deviceManager.removeAudioCallback( &m_audioSourcePlayer );
    m_deviceManager.removeMidiInputCallback( String::empty, m_samplerAudioSource->getMidiInputCallback() );

    m_rubberbandAudioSource = NULL;
    m_samplerAudioSource = NULL;
}



void MainWindow::setupUI()
{
    // Initialise user interface
    m_ui->setupUi( this );
    m_scene = m_ui->waveGraphicsView->getScene();


    // Set up interaction mode buttons to work like radio buttons
    m_interactionGroup = new QActionGroup( this );
    m_interactionGroup->addAction( m_ui->actionSelect_Move );
    m_interactionGroup->addAction( m_ui->actionMulti_Select );
    m_interactionGroup->addAction( m_ui->actionAudition );


    // Populate "Detection Method" combo box
    QStringList detectMethodTextList, detectMethodDataList;

    detectMethodTextList << "Broadband Energy" << "High Frequency Content" << "Complex Domain"
            << "Phase Based" << "Spectral Difference" << "Kullback-Liebler"
            << "Modified Kullback-Liebler" << "Spectral Flux";

    detectMethodDataList << "energy" << "hfc" << "complex" << "phase" << "specdiff"
            << "kl" << "mkl" << "specflux";

    for ( int i = 0; i < detectMethodTextList.size(); i++ )
    {
        m_ui->comboBox_DetectMethod->addItem( detectMethodTextList[ i ], detectMethodDataList[ i ] );
    }


    // Populate "Window Size" combo box
    QStringList windowSizeTextList;
    QList<int> windowSizeDataList;

    windowSizeTextList << "128" << "256" << "512" << "1024" << "2048" << "4096" << "8192";
    windowSizeDataList << 128 << 256 << 512 << 1024 << 2048 << 4096 << 8192;

    for ( int i = 0; i < windowSizeTextList.size(); i++ )
    {
        m_ui->comboBox_WindowSize->addItem( windowSizeTextList[ i ], windowSizeDataList[ i ] );
    }
    m_ui->comboBox_WindowSize->setCurrentIndex( 3 ); // "1024"


    // Populate "Hop Size" combo box
    QStringList hopSizeTextList;
    QList<qreal> hopSizeDataList;

    hopSizeTextList << "50%" << "25%" << "12.5%" << "6.25%";
    hopSizeDataList << 50.0 << 25.0 << 12.5 << 6.25;

    for ( int i = 0; i < hopSizeTextList.size(); i++ )
    {
        m_ui->comboBox_HopSize->addItem( hopSizeTextList[ i ], hopSizeDataList[ i ] );
    }
    m_ui->comboBox_HopSize->setCurrentIndex( 0 ); // "50%"


    // Populate "Snap Values" combo box
    QStringList snapValuesTextList;

    snapValuesTextList << "Beats / 64" << "Beats / 32" << "Beats / 16" << "Beats / 12" << "Beats / 10" << "Beats / 9" << "Beats / 8" << "Beats / 7" << "Beats / 6" << "Beats / 5" << "Beats / 4" << "Beats / 3" << "Beats / 2" << "Beats" << "Off";

    m_ui->comboBox_SnapValues->addItems( snapValuesTextList );

    m_ui->comboBox_SnapValues->setCurrentIndex( snapValuesTextList.size() - 1 );


    // Populate "Time Signature" combo boxes
    QStringList timeSigNumeratorTextList;
    QStringList timeSigDenominatorTextList;

    timeSigNumeratorTextList << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    timeSigDenominatorTextList << "1" << "2" << "4" << "8" << "16";

    m_ui->comboBox_TimeSigNumerator->addItems( timeSigNumeratorTextList );
    m_ui->comboBox_TimeSigDenominator->addItems( timeSigDenominatorTextList );

    m_ui->comboBox_TimeSigNumerator->setCurrentIndex( 3 );   // 4
    m_ui->comboBox_TimeSigDenominator->setCurrentIndex( 2 ); // 4


    // Populate "Units" combo box
    QStringList unitsTextList;

    unitsTextList << "Bars" << "Beats";

    m_ui->comboBox_Units->addItems( unitsTextList );


    // Hide widgets / menu items
    m_ui->label_JackSync->setVisible( false );
    m_ui->checkBox_TimeStretch->setVisible( false );


    // Connect signals to slots
    QObject::connect( m_scene, SIGNAL( slicePointPosChanged(SharedSlicePointItem,int,int,int,int) ),
                      this, SLOT( recordSlicePointItemMove(SharedSlicePointItem,int,int,int,int) ) );

    QObject::connect( m_ui->waveGraphicsView, SIGNAL( minDetailLevelReached() ),
                      this, SLOT( disableZoomOut() ) );

    QObject::connect( m_ui->waveGraphicsView, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( disableZoomIn() ) );

    QObject::connect( m_scene, SIGNAL( playheadFinishedScrolling() ),
                      this, SLOT( resetPlayStopButtonIcon() ) );

    QObject::connect( m_scene, SIGNAL( selectionChanged() ),
                      this, SLOT( enableEditActions() ) );

    QObject::connect( &m_undoStack, SIGNAL( canUndoChanged(bool) ),
                      m_ui->actionUndo, SLOT( setEnabled(bool) ) );

    QObject::connect( &m_undoStack, SIGNAL( canRedoChanged(bool) ),
                      m_ui->actionRedo, SLOT( setEnabled(bool) ) );

    QObject::connect( m_ui->actionUndo, SIGNAL( triggered() ),
                      &m_undoStack, SLOT( undo() ) );

    QObject::connect( m_ui->actionRedo, SIGNAL( triggered() ),
                      &m_undoStack, SLOT( redo() ) );

    QObject::connect( &m_undoStack, SIGNAL( cleanChanged(bool) ),
                      m_ui->actionSave_Project, SLOT( setDisabled(bool) ) );


    // Create help form
    m_helpForm = new HelpForm();

    if ( m_helpForm != NULL )
    {
        // Make sure help form isn't larger than desktop
        const int desktopWidth = QApplication::desktop()->availableGeometry().width();
        const int desktopHeight = QApplication::desktop()->availableGeometry().height();

        const int frameWidth = m_helpForm->frameSize().width();
        const int frameHeight = m_helpForm->frameSize().height();

        int formWidth = m_helpForm->size().width();
        int formHeight = m_helpForm->size().height();

        int maxWidth = m_helpForm->maximumWidth();
        int maxHeight = m_helpForm->maximumHeight();

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

        m_helpForm->resize( formWidth, formHeight );
        m_helpForm->setMaximumSize( maxWidth, maxHeight );

        // Centre form in desktop
        m_helpForm->setGeometry
        (
            QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, m_helpForm->size(), QApplication::desktop()->availableGeometry() )
        );

        m_ui->actionHelp->setEnabled( true );
    }


    // Create export dialog
    m_exportDialog = new ExportDialog();

    if ( m_exportDialog != NULL )
    {
        // Centre form in desktop
        m_exportDialog->setGeometry
        (
            QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, m_exportDialog->size(), QApplication::desktop()->availableGeometry() )
        );
    }


    // Create options dialog
    m_optionsDialog = new OptionsDialog( m_deviceManager );

    if ( m_optionsDialog != NULL )
    {
        // Centre form in desktop
        m_optionsDialog->setGeometry
        (
            QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, m_optionsDialog->size(), QApplication::desktop()->availableGeometry() )
        );

        QObject::connect( m_optionsDialog, SIGNAL( realtimeModeToggled(bool) ),
                          this, SLOT( enableRealtimeControls(bool) ) );

        QObject::connect( m_optionsDialog, SIGNAL( jackSyncToggled(bool) ),
                          m_ui->doubleSpinBox_NewBPM, SLOT( setHidden(bool) ) );

        QObject::connect( m_optionsDialog, SIGNAL( jackSyncToggled(bool) ),
                          m_ui->label_JackSync, SLOT( setVisible(bool) ) );

        QObject::connect( m_optionsDialog, SIGNAL( timeStretchOptionsChanged() ),
                          this, SLOT( enableSaveAction() ) );

        m_optionsDialog->disableTab( OptionsDialog::TIME_STRETCH_TAB );
    }
}



void MainWindow::enableUI()
{
    m_ui->pushButton_PlayStop->setEnabled( true );
    m_ui->pushButton_Loop->setEnabled( true );
    m_ui->pushButton_TimestretchOptions->setEnabled( true );

    if ( m_optionsDialog->isRealtimeModeEnabled() )
    {
        m_ui->checkBox_TimeStretch->setVisible( true );
    }
    else
    {
        m_ui->pushButton_Apply->setEnabled( true );
        m_ui->checkBox_TimeStretch->setVisible( false );
    }

    m_ui->doubleSpinBox_OriginalBPM->setEnabled( true );
    m_ui->doubleSpinBox_NewBPM->setEnabled( true );
    m_ui->pushButton_CalcBPM->setEnabled( true );
    m_ui->checkBox_TimeStretch->setEnabled( true );
    m_ui->checkBox_PitchCorrection->setEnabled( true );
    m_ui->comboBox_DetectMethod->setEnabled( true );
    m_ui->comboBox_WindowSize->setEnabled( true );
    m_ui->comboBox_HopSize->setEnabled( true );
    m_ui->lcdNumber_Threshold->setEnabled( true );
    m_ui->horizontalSlider_Threshold->setEnabled( true );
    m_ui->pushButton_FindOnsets->setEnabled( true );
    m_ui->pushButton_FindBeats->setEnabled( true );
    m_ui->comboBox_TimeSigNumerator->setEnabled( true );
    m_ui->comboBox_TimeSigDenominator->setEnabled( true );
    m_ui->spinBox_Length->setEnabled( true );
    m_ui->comboBox_Units->setEnabled( true );

    m_ui->actionSave_As->setEnabled( true );
    m_ui->actionClose_Project->setEnabled( true );
    m_ui->actionExport_As->setEnabled( true );
    m_ui->actionSelect_All->setEnabled( true );
    m_ui->actionSelect_None->setEnabled( true );
    m_ui->actionAdd_Slice_Point->setEnabled( true );
    m_ui->actionZoom_Original->setEnabled( true );
    m_ui->actionZoom_In->setEnabled( true );
    m_ui->actionSelect_Move->setEnabled( true );
    m_ui->actionMulti_Select->setEnabled( true );
    m_ui->actionAudition->setEnabled( true );

    m_ui->actionAudition->trigger();

    m_optionsDialog->enableTab( OptionsDialog::TIME_STRETCH_TAB );
}



void MainWindow::disableUI()
{
    m_ui->pushButton_PlayStop->setEnabled( false );
    m_ui->pushButton_Loop->setEnabled( false );
    m_ui->doubleSpinBox_OriginalBPM->setValue( 0.0 );
    m_ui->doubleSpinBox_OriginalBPM->setEnabled( false );
    m_ui->doubleSpinBox_NewBPM->setValue( 0.0 );
    m_ui->doubleSpinBox_NewBPM->setEnabled( false );
    m_ui->pushButton_CalcBPM->setEnabled( false );
    m_ui->pushButton_Apply->setEnabled( false );
    m_ui->checkBox_TimeStretch->setEnabled( false );
    m_ui->checkBox_PitchCorrection->setEnabled( false );
    m_ui->pushButton_TimestretchOptions->setEnabled( false );
    m_ui->pushButton_Slice->setEnabled( false );
    m_ui->pushButton_Slice->setChecked( false );
    m_ui->comboBox_DetectMethod->setEnabled( false );
    m_ui->comboBox_WindowSize->setEnabled( false );
    m_ui->comboBox_HopSize->setEnabled( false );
    m_ui->lcdNumber_Threshold->setEnabled( false );
    m_ui->horizontalSlider_Threshold->setEnabled( false );
    m_ui->pushButton_FindOnsets->setEnabled( false );
    m_ui->pushButton_FindBeats->setEnabled( false );
    m_ui->comboBox_SnapValues->setEnabled( false );
    m_ui->comboBox_TimeSigNumerator->setEnabled( false );
    m_ui->comboBox_TimeSigDenominator->setEnabled( false );
    m_ui->spinBox_Length->setEnabled( false );
    m_ui->spinBox_Length->setValue( 0 );
    m_ui->comboBox_Units->setEnabled( false );

    m_ui->actionSave_Project->setEnabled( false );
    m_ui->actionSave_As->setEnabled( false );
    m_ui->actionClose_Project->setEnabled( false );
    m_ui->actionExport_As->setEnabled( false );
    m_ui->actionSelect_All->setEnabled( false );
    m_ui->actionSelect_None->setEnabled( false );
    m_ui->actionAdd_Slice_Point->setEnabled( false );
    m_ui->actionAdd_Slice_Point->setVisible( true );
    m_ui->actionDelete->setEnabled( false );
    m_ui->actionReverse->setEnabled( false );
    m_ui->actionZoom_Original->setEnabled( false );
    m_ui->actionZoom_Out->setEnabled( false );
    m_ui->actionZoom_In->setEnabled( false );
    m_ui->actionSelect_Move->setEnabled( false );
    m_ui->actionMulti_Select->setEnabled( false );
    m_ui->actionAudition->setEnabled( false );

    if ( m_ui->actionSelective_Time_Stretch->isChecked() )
    {
        m_ui->actionSelective_Time_Stretch->trigger();
    }
    m_ui->actionSelective_Time_Stretch->setEnabled( false );

    m_optionsDialog->disableTab( OptionsDialog::TIME_STRETCH_TAB );
}



void MainWindow::connectWaveformToMainWindow( const SharedWaveformItem item )
{
    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( recordWaveformItemMove(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
                      this, SLOT( stopPlayback() ) );

    QObject::connect( item.data(), SIGNAL( clicked(const WaveformItem*,QPointF) ),
                      this, SLOT( playSampleRange(const WaveformItem*,QPointF) ) );
}



void MainWindow::getDetectionSettings( AudioAnalyser::DetectionSettings& settings )
{
    int currentIndex;

    // From aubio website: "Typical threshold values are within 0.001 and 0.900." Default is 0.3 in aubio-0.4.0
    currentIndex = m_ui->comboBox_DetectMethod->currentIndex();
    settings.detectionMethod = m_ui->comboBox_DetectMethod->itemData( currentIndex ).toString().toLocal8Bit();

    settings.threshold = qreal( m_ui->horizontalSlider_Threshold->value() ) / 1000.0;

    currentIndex = m_ui->comboBox_WindowSize->currentIndex();
    settings.windowSize = (uint_t) m_ui->comboBox_WindowSize->itemData( currentIndex ).toInt();

    currentIndex = m_ui->comboBox_HopSize->currentIndex();
    const qreal percentage = m_ui->comboBox_HopSize->itemData( currentIndex ).toReal();
    settings.hopSize = (uint_t) ( settings.windowSize * ( percentage / 100.0 ) );

    settings.sampleRate = (uint_t) m_sampleHeader->sampleRate;
}



void MainWindow::closeProject()
{
    m_sampleHeader.clear();
    m_sampleBufferList.clear();
    tearDownSampler();

    m_scene->clearAll();
    on_actionZoom_Original_triggered();
    disableUI();
    m_ui->statusBar->clearMessage();

    m_undoStack.clear();

    m_appliedBPM = 0.0;

    m_currentProjectFilePath.clear();
    m_isProjectOpen = false;
}



bool MainWindow::isSelectiveTimeStretchInUse() const
{
    bool isSelectiveTimeStretchInUse = false;

    if ( m_samplerAudioSource != NULL && m_rubberbandAudioSource != NULL )
    {
        const int lowestAssignedMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

        for ( int i = 0; i < m_sampleBufferList.size(); i++ )
        {
            const int midiNote = lowestAssignedMidiNote + i;
            const qreal timeRatio = m_rubberbandAudioSource->getNoteTimeRatio( midiNote );

            if ( timeRatio != 1.0 )
            {
                isSelectiveTimeStretchInUse = true;
                break;
            }
        }
    }

    return isSelectiveTimeStretchInUse;
}



void MainWindow::renderTimeStretch()
{
    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        if ( m_samplerAudioSource != NULL && m_rubberbandAudioSource != NULL )
        {
            const QString fileBaseName = QString::number( m_undoStack.index() );

            QUndoCommand* command = new RenderTimeStretchCommand( this,
                                                                  m_scene,
                                                                  m_ui->waveGraphicsView,
                                                                  tempDirPath,
                                                                  fileBaseName );

            m_undoStack.push( command );
        }
    }
    else
    {
        MessageBoxes::showWarningDialog( tr("Temp dir invalid!"),
                                         tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
    }
}



//==================================================================================================
// Private Slots:

void MainWindow::recordWaveformItemMove( QList<int> oldOrderPositions, const int numPlacesMoved )
{
    QUndoCommand* command = new MoveWaveformItemCommand( oldOrderPositions, numPlacesMoved, m_scene, this );
    m_undoStack.push( command );
}



void MainWindow::recordSlicePointItemMove( const SharedSlicePointItem slicePoint,
                                           const int orderPos,
                                           const int numFramesFromPrevSlicePoint,
                                           const int numFramesToNextSlicePoint,
                                           const int oldFrameNum )
{
    if ( m_ui->actionSelective_Time_Stretch->isChecked() )
    {
        if ( m_samplerAudioSource != NULL && m_rubberbandAudioSource != NULL )
        {
            QUndoCommand* parentCommand = new QUndoCommand();

            new MoveSlicePointItemCommand( slicePoint, oldFrameNum, m_scene, parentCommand );

            QList<int> orderPositions;
            QList<qreal> timeRatios;
            QList<int> midiNotes;

            orderPositions << orderPos << orderPos + 1;

            timeRatios << (qreal) numFramesFromPrevSlicePoint / m_sampleBufferList.at( orderPos )->getNumFrames();
            timeRatios << (qreal) numFramesToNextSlicePoint / m_sampleBufferList.at( orderPos + 1 )->getNumFrames();

            midiNotes << m_samplerAudioSource->getLowestAssignedMidiNote() + orderPos;
            midiNotes << m_samplerAudioSource->getLowestAssignedMidiNote() + orderPos + 1;

            new SelectiveTimeStretchCommand( this,
                                             m_scene,
                                             orderPositions,
                                             timeRatios,
                                             midiNotes,
                                             parentCommand );

            m_undoStack.push( parentCommand );
        }
    }
    else
    {
        QUndoCommand* command = new MoveSlicePointItemCommand( slicePoint, oldFrameNum, m_scene );
        m_undoStack.push( command );
    }
}



void MainWindow::playSampleRange( const WaveformItem* waveformItem, const QPointF mouseScenePos )
{
    SharedSampleRange sampleRange( new SampleRange );
    sampleRange->startFrame = 0;
    sampleRange->numFrames = waveformItem->getSampleBuffer()->getNumFrames();

    qreal startPosX = waveformItem->scenePos().x();
    qreal endPosX = startPosX + waveformItem->rect().width();

    const QList<int> slicePointFrameNums = m_scene->getSlicePointFrameNums();

    // If slice points are present and the waveform has not yet been sliced...
    if ( slicePointFrameNums.size() > 0 && m_sampleBufferList.size() == 1 )
    {
        const int mousePosFrameNum = m_scene->getFrameNum( mouseScenePos.x() );
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

        startPosX = m_scene->getScenePosX( sampleRange->startFrame );
        endPosX = m_scene->getScenePosX( endFrame );
    }

    // Play sample range and start playhead scrolling
    m_samplerAudioSource->playSample( waveformItem->getOrderPos(), sampleRange );
    m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-stop.png" ) );

    if ( m_optionsDialog != NULL && m_optionsDialog->isRealtimeModeEnabled() &&
         m_ui->checkBox_TimeStretch->isChecked() &&
         m_ui->doubleSpinBox_OriginalBPM->value() > 0.0 &&
         m_ui->doubleSpinBox_NewBPM->value() > 0.0 )
    {
        qreal stretchRatio = m_ui->doubleSpinBox_OriginalBPM->value() / m_ui->doubleSpinBox_NewBPM->value();
        m_scene->startPlayhead( startPosX, endPosX, sampleRange->numFrames, stretchRatio );
    }
    else
    {
        m_scene->startPlayhead( startPosX, endPosX, sampleRange->numFrames );
    }
}



void MainWindow::stopPlayback()
{
    if ( m_samplerAudioSource != NULL )
    {
        m_samplerAudioSource->stop();
    }
    m_scene->stopPlayhead();
    m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
}



void MainWindow::resetPlayStopButtonIcon()
{
    m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
}



void MainWindow::disableZoomIn()
{
    m_ui->actionZoom_In->setEnabled( false );
}



void MainWindow::disableZoomOut()
{
    m_ui->actionZoom_Out->setEnabled( false );
}



void MainWindow::enableRealtimeControls( const bool isEnabled )
{
    if ( isEnabled ) // Realtime mode
    {
        m_ui->checkBox_TimeStretch->setVisible( true );
        m_ui->pushButton_Apply->setVisible( false );

        if ( m_sampleBufferList.size() > 1 )
        {
            m_ui->actionSelective_Time_Stretch->setEnabled( true );
        }

        QObject::connect( m_optionsDialog, SIGNAL( windowOptionChanged() ),
                          this, SLOT( resetSampler() ) );
    }
    else // Offline mode
    {
        m_ui->checkBox_TimeStretch->setVisible( false );
        m_ui->pushButton_Apply->setVisible( true );

        m_ui->actionSelective_Time_Stretch->setEnabled( false );

        QObject::disconnect( m_optionsDialog, SIGNAL( windowOptionChanged() ),
                             this, SLOT( resetSampler() ) );

        if ( isSelectiveTimeStretchInUse() )
        {
            renderTimeStretch();
        }
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
    const SharedSlicePointItem slicePoint = m_scene->getSelectedSlicePoint();
    const QList<int> orderPositions = m_scene->getSelectedWaveformsOrderPositions();

    m_ui->actionDelete->setEnabled( false );

    if ( ! slicePoint.isNull() || ! orderPositions.isEmpty() )
    {
        if ( orderPositions.size() < m_scene->getNumWaveforms() )
        {
            m_ui->actionDelete->setEnabled( true );
        }
    }

    if ( ! orderPositions.isEmpty() )
    {
        m_ui->actionApply_Gain->setEnabled( true );
        m_ui->actionApply_Gain_Ramp->setEnabled( true );
        m_ui->actionNormalise->setEnabled( true );
        m_ui->actionReverse->setEnabled( true );
    }
    else
    {
        m_ui->actionApply_Gain->setEnabled( false );
        m_ui->actionApply_Gain_Ramp->setEnabled( false );
        m_ui->actionNormalise->setEnabled( false );
        m_ui->actionReverse->setEnabled( false );
    }
}



void MainWindow::enableSaveAction()
{
    if ( m_isProjectOpen )
    {
        m_ui->actionSave_Project->setEnabled( true );
    }
}



//====================
// "File" menu:

void MainWindow::on_actionOpen_Project_triggered()
{
    // Check for unsaved changes before continuing
    if ( m_undoStack.isClean() )
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
    if ( ! m_sampleBufferList.isEmpty() )
    {
        if ( m_currentProjectFilePath.isEmpty() )
        {
            saveProjectDialog();
        }
        else
        {
            saveProject( m_currentProjectFilePath );
        }
    }
}



void MainWindow::on_actionSave_As_triggered()
{
    if ( ! m_sampleBufferList.isEmpty() )
    {
        saveProjectDialog();
    }
}



void MainWindow::on_actionClose_Project_triggered()
{
    // Check for unsaved changes before continuing
    if ( m_undoStack.isClean() )
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
    if ( m_undoStack.isClean() )
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
    if ( m_undoStack.isClean() )
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
    m_scene->selectAll();
}



void MainWindow::on_actionSelect_None_triggered()
{
    m_scene->selectNone();
}



void MainWindow::on_actionDelete_triggered()
{    
    const SharedSlicePointItem selectedSlicePoint = m_scene->getSelectedSlicePoint();

    if ( ! selectedSlicePoint.isNull() )
    {
        selectedSlicePoint->setSelected( false );

        QUndoCommand* command = new DeleteSlicePointItemCommand( selectedSlicePoint,
                                                                 m_scene,
                                                                 m_ui->pushButton_Slice );
        m_undoStack.push( command );
    }
    else
    {
        const QList<int> orderPositions = m_scene->getSelectedWaveformsOrderPositions();

        if ( ! orderPositions.isEmpty() )
        {
            foreach ( int orderPos, orderPositions )
            {
                m_scene->getWaveformAt( orderPos )->setSelected( false );
            }

            QUndoCommand* command = new DeleteWaveformItemCommand( orderPositions,
                                                                   m_scene,
                                                                   this,
                                                                   m_ui->pushButton_Slice,
                                                                   m_ui->pushButton_FindOnsets,
                                                                   m_ui->pushButton_FindBeats,
                                                                   m_ui->actionAdd_Slice_Point );
            m_undoStack.push( command );
        }
    }
}



void MainWindow::on_actionAdd_Slice_Point_triggered()
{
    const QPoint mousePos = m_ui->waveGraphicsView->mapFromGlobal( QCursor::pos() );
    const QPointF mouseScenePos = m_ui->waveGraphicsView->mapToScene( mousePos );
    const int frameNum = m_scene->getFrameNum( mouseScenePos.x() );

    QUndoCommand* command = new AddSlicePointItemCommand( frameNum, true, m_scene, m_ui->pushButton_Slice );
    m_undoStack.push( command );
}



void MainWindow::on_actionApply_Gain_triggered()
{
    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        ApplyGainDialog dialog;

        const int result = dialog.exec();

        if ( result == QDialog::Accepted )
        {
            const QList<int> orderPositions = m_scene->getSelectedWaveformsOrderPositions();

            QUndoCommand* parentCommand = new QUndoCommand();

            const QString stackIndex = QString::number( m_undoStack.index() );
            int i = 0;

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName = stackIndex + "_" + QString::number( i++ );

                new ApplyGainCommand( dialog.getGainValue(),
                                      orderPos,
                                      m_scene,
                                      m_ui->waveGraphicsView,
                                      m_sampleHeader->sampleRate,
                                      m_fileHandler,
                                      tempDirPath,
                                      fileBaseName,
                                      parentCommand );
            }

            m_undoStack.push( parentCommand );
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
    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        ApplyGainRampDialog dialog;

        const int result = dialog.exec();

        if ( result == QDialog::Accepted )
        {
            const QList<int> orderPositions = m_scene->getSelectedWaveformsOrderPositions();

            QUndoCommand* parentCommand = new QUndoCommand();

            const QString stackIndex = QString::number( m_undoStack.index() );
            int i = 0;

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName = stackIndex + "_" + QString::number( i++ );

                new ApplyGainRampCommand( dialog.getStartGainValue(),
                                          dialog.getEndGainValue(),
                                          orderPos,
                                          m_scene,
                                          m_ui->waveGraphicsView,
                                          m_sampleHeader->sampleRate,
                                          m_fileHandler,
                                          tempDirPath,
                                          fileBaseName,
                                          parentCommand );
            }

            m_undoStack.push( parentCommand );
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
    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    if ( ! tempDirPath.isEmpty() )
    {
        const QList<int> orderPositions = m_scene->getSelectedWaveformsOrderPositions();

        QUndoCommand* parentCommand = new QUndoCommand();

        const QString stackIndex = QString::number( m_undoStack.index() );
        int i = 0;

        foreach ( int orderPos, orderPositions )
        {
            QString fileBaseName = stackIndex + "_" + QString::number( i++ );

            new NormaliseCommand( orderPos,
                                  m_scene,
                                  m_ui->waveGraphicsView,
                                  m_sampleHeader->sampleRate,
                                  m_fileHandler,
                                  tempDirPath,
                                  fileBaseName,
                                  parentCommand );
        }

        m_undoStack.push( parentCommand );
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



void MainWindow::on_actionReverse_triggered()
{
    const QList<int> orderPositions = m_scene->getSelectedWaveformsOrderPositions();

    QUndoCommand* parentCommand = new QUndoCommand();

    foreach ( int orderPos, orderPositions )
    {
        new ReverseCommand( orderPos, m_scene, m_ui->waveGraphicsView, parentCommand );
    }

    m_undoStack.push( parentCommand );
}



//====================
// "Options" menu:

void MainWindow::on_actionOptions_triggered()
{
    QPoint pos = m_optionsDialog->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    m_optionsDialog->move( pos );
    m_optionsDialog->setCurrentTab( OptionsDialog::AUDIO_SETUP_TAB );
    m_optionsDialog->show();
}



//====================
// "Help" menu:

void MainWindow::on_actionHelp_triggered()
{
    QPoint pos = m_helpForm->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    m_helpForm->move( pos );
    m_helpForm->show();
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
    int numFrames = SampleUtils::getTotalNumFrames( m_sampleBufferList );

    const qreal numSeconds = numFrames / m_sampleHeader->sampleRate;
    const int numerator = m_ui->comboBox_TimeSigNumerator->currentText().toInt();

    int numBeats = 0;

    if ( m_ui->comboBox_Units->currentIndex() == UNITS_BARS )
    {
        numBeats = m_ui->spinBox_Length->value() * numerator;
    }
    else // UNITS_BEATS
    {
        numBeats = m_ui->spinBox_Length->value();
    }

    bpm = numBeats / ( numSeconds / 60 );

    m_ui->doubleSpinBox_OriginalBPM->setValue( bpm );
    m_ui->doubleSpinBox_NewBPM->setValue( bpm );

    m_scene->setBpmRulerMarks( bpm, numerator );

    if ( m_rubberbandAudioSource != NULL && bpm > 0.0 )
    {
        m_rubberbandAudioSource->setOriginalBPM( bpm );
    }
}



void MainWindow::on_pushButton_Slice_clicked( const bool isChecked )
{
    if ( isChecked ) // Slice
    {
        QUndoCommand* parentCommand = new QUndoCommand();

        new SliceCommand( this,
                          m_scene,
                          m_ui->pushButton_Slice,
                          m_ui->pushButton_FindOnsets,
                          m_ui->pushButton_FindBeats,
                          m_ui->actionAdd_Slice_Point,
                          m_ui->actionSelect_Move,
                          m_ui->actionAudition,
                          m_ui->actionSelective_Time_Stretch,
                          parentCommand );

        QList<SharedSlicePointItem> slicePoints = m_scene->getSlicePointList();

        foreach ( SharedSlicePointItem slicePoint, slicePoints )
        {
            new DeleteSlicePointItemCommand( slicePoint, m_scene, NULL, parentCommand );
        }

        m_undoStack.push( parentCommand );
    }
    else // Unslice
    {
        if ( isSelectiveTimeStretchInUse() )
        {
            renderTimeStretch();
        }

        QUndoCommand* parentCommand = new QUndoCommand();

        int frameNum = 0;

        for ( int i = 0; i < m_sampleBufferList.size() - 1; i++ )
        {
            frameNum += m_sampleBufferList.at( i )->getNumFrames();

            new AddSlicePointItemCommand( frameNum, true, m_scene, NULL, parentCommand );
        }

        new UnsliceCommand( this,
                            m_scene,
                            m_ui->pushButton_Slice,
                            m_ui->pushButton_FindOnsets,
                            m_ui->pushButton_FindBeats,
                            m_ui->actionAdd_Slice_Point,
                            m_ui->actionSelect_Move,
                            m_ui->actionAudition,
                            m_ui->actionSelective_Time_Stretch,
                            parentCommand );

        m_undoStack.push( parentCommand );
    }
}



void MainWindow::on_horizontalSlider_Threshold_valueChanged( const int value )
{
    m_ui->lcdNumber_Threshold->display( qreal( value ) / 1000.0 );
}



void MainWindow::on_pushButton_FindOnsets_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    QUndoCommand* parentCommand = new QUndoCommand();

    // Remove current slice points if present
    {
        const QList<SharedSlicePointItem> slicePointItemList = m_scene->getSlicePointList();

        foreach ( SharedSlicePointItem item, slicePointItemList )
        {
            new DeleteSlicePointItemCommand( item, m_scene, m_ui->pushButton_Slice, parentCommand );
        }
    }

    // Add new slice points
    {
        AudioAnalyser::DetectionSettings settings;
        getDetectionSettings( settings );

        const QList<int> slicePointFrameNumList = AudioAnalyser::findOnsetFrameNums( m_sampleBufferList.first(), settings );

        foreach ( int frameNum, slicePointFrameNumList )
        {
            new AddSlicePointItemCommand( frameNum, true, m_scene, m_ui->pushButton_Slice, parentCommand );
        }
    }

    m_undoStack.push( parentCommand );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_pushButton_FindBeats_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    AudioAnalyser::DetectionSettings settings;
    getDetectionSettings( settings );

    // Get list of frame numbers for slice points to be added
    const QList<int> slicePointFrameNumList = AudioAnalyser::findBeatFrameNums( m_sampleBufferList.first(), settings );

    // Get list of slice point items to be removed
    const QList<SharedSlicePointItem> slicePointItemList = m_scene->getSlicePointList();

    QUndoCommand* parentCommand = new QUndoCommand();

    foreach ( SharedSlicePointItem item, slicePointItemList )
    {
        new DeleteSlicePointItemCommand( item, m_scene, m_ui->pushButton_Slice, parentCommand );
    }

    foreach ( int frameNum, slicePointFrameNumList )
    {
        new AddSlicePointItemCommand( frameNum, true, m_scene, m_ui->pushButton_Slice, parentCommand );
    }

    m_undoStack.push( parentCommand );

    QApplication::restoreOverrideCursor();
}



void MainWindow::on_doubleSpinBox_OriginalBPM_valueChanged( const double originalBPM )
{
    const qreal newBPM = m_ui->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = m_ui->checkBox_TimeStretch->isChecked();

    if ( isTimeStretchEnabled && m_rubberbandAudioSource != NULL )
    {
        m_rubberbandAudioSource->setOriginalBPM( originalBPM );

        if ( ! m_optionsDialog->isJackSyncEnabled() && newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal timeRatio = originalBPM / newBPM;

            m_rubberbandAudioSource->setGlobalTimeRatio( timeRatio );

            m_scene->updatePlayheadSpeed( timeRatio );
        }
    }

    const int timeSigNumerator = m_ui->comboBox_TimeSigNumerator->currentText().toInt();

    m_scene->setBpmRulerMarks( originalBPM, timeSigNumerator );
}



void MainWindow::on_doubleSpinBox_NewBPM_valueChanged( const double newBPM )
{
    const qreal originalBPM = m_ui->doubleSpinBox_OriginalBPM->value();
    const bool isTimeStretchEnabled = m_ui->checkBox_TimeStretch->isChecked();

    if ( isTimeStretchEnabled && m_rubberbandAudioSource != NULL )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const qreal timeRatio = originalBPM / newBPM;

            m_rubberbandAudioSource->setGlobalTimeRatio( timeRatio );

            m_scene->updatePlayheadSpeed( timeRatio );
        }
    }
}



void MainWindow::on_checkBox_TimeStretch_toggled( const bool isChecked )
{
    const qreal originalBPM = m_ui->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = m_ui->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = isChecked;

    if ( m_rubberbandAudioSource != NULL )
    {
        qreal timeRatio = 1.0;
        bool isPitchCorrectionEnabled = true;

        if ( isTimeStretchEnabled )
        {
            if ( newBPM > 0.0 && originalBPM > 0.0 )
                timeRatio = originalBPM / newBPM;
            isPitchCorrectionEnabled = m_ui->checkBox_PitchCorrection->isChecked();
        }

        m_rubberbandAudioSource->setGlobalTimeRatio( timeRatio );
        m_rubberbandAudioSource->enablePitchCorrection( isPitchCorrectionEnabled );
    }
}



void MainWindow::on_checkBox_PitchCorrection_toggled( const bool isChecked )
{
    const bool isTimeStretchEnabled = m_ui->checkBox_TimeStretch->isChecked();

    if ( isTimeStretchEnabled && m_rubberbandAudioSource != NULL )
    {
        m_rubberbandAudioSource->enablePitchCorrection( isChecked );
    }
}



void MainWindow::on_pushButton_PlayStop_clicked()
{
    if ( m_scene->isPlayheadScrolling() )
    {
        m_samplerAudioSource->stop();
        m_scene->stopPlayhead();
        m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
    }
    else
    {
        m_samplerAudioSource->playAll();
        
        m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-stop.png" ) );

        if ( m_optionsDialog != NULL && m_optionsDialog->isRealtimeModeEnabled() &&
             m_ui->checkBox_TimeStretch->isChecked() &&
             m_ui->doubleSpinBox_OriginalBPM->value() > 0.0 &&
             m_ui->doubleSpinBox_NewBPM->value() > 0.0 )
        {
            qreal stretchRatio = m_ui->doubleSpinBox_OriginalBPM->value() / m_ui->doubleSpinBox_NewBPM->value();
            m_scene->startPlayhead( m_ui->pushButton_Loop->isChecked(), stretchRatio );
        }
        else
        {
            m_scene->startPlayhead( m_ui->pushButton_Loop->isChecked() );
        }
    }
}



void MainWindow::on_pushButton_Loop_clicked( const bool isChecked )
{
    if ( m_samplerAudioSource != NULL )
    {
        m_samplerAudioSource->setLooping( isChecked );
    }
    m_scene->setPlayheadLooping( isChecked );
}



void MainWindow::on_actionZoom_In_triggered()
{
    m_ui->waveGraphicsView->zoomIn();
    m_ui->actionZoom_Out->setEnabled( true );
}



void MainWindow::on_actionZoom_Out_triggered()
{
    m_ui->waveGraphicsView->zoomOut();
    m_ui->actionZoom_In->setEnabled( true );
}



void MainWindow::on_actionZoom_Original_triggered()
{
    m_ui->waveGraphicsView->zoomOriginal();
    m_ui->actionZoom_In->setEnabled( true );
    m_ui->actionZoom_Out->setEnabled( false );
}



void MainWindow::on_pushButton_Apply_clicked()
{
    const qreal originalBPM = m_ui->doubleSpinBox_OriginalBPM->value();
    const qreal newBPM = m_ui->doubleSpinBox_NewBPM->value();

    if ( newBPM > 0.0 && originalBPM > 0.0 )
    {
        const QString tempDirPath = m_optionsDialog->getTempDirPath();

        if ( ! tempDirPath.isEmpty() )
        {
            const QString fileBaseName = QString::number( m_undoStack.index() );

            QUndoCommand* command = new GlobalTimeStretchCommand( this,
                                                                  m_scene,
                                                                  m_ui->waveGraphicsView,
                                                                  m_ui->doubleSpinBox_OriginalBPM,
                                                                  m_ui->doubleSpinBox_NewBPM,
                                                                  m_ui->checkBox_PitchCorrection,
                                                                  tempDirPath,
                                                                  fileBaseName );
            m_undoStack.push( command );
        }
        else
        {
            MessageBoxes::showWarningDialog( tr("Temp dir invalid!"),
                                             tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
        }
    }
}



void MainWindow::on_actionSelect_Move_triggered()
{
    m_ui->waveGraphicsView->setInteractionMode( WaveGraphicsView::SELECT_MOVE_ITEMS );
}



void MainWindow::on_actionMulti_Select_triggered()
{
    m_ui->waveGraphicsView->setInteractionMode( WaveGraphicsView::MULTI_SELECT_ITEMS );
}



void MainWindow::on_actionAudition_triggered()
{
    m_ui->waveGraphicsView->setInteractionMode( WaveGraphicsView::AUDITION_ITEMS );
}



void MainWindow::on_actionSelective_Time_Stretch_triggered( const bool isChecked )
{
    if ( isChecked ) // Enable Selective Time Stretching
    {
        QUndoCommand* parentCommand = new QUndoCommand();

        new EnableSelectiveTSCommand( m_optionsDialog,
                                      m_scene,
                                      m_ui->pushButton_Slice,
                                      m_ui->actionAdd_Slice_Point,
                                      m_ui->actionSelect_Move,
                                      m_ui->actionMulti_Select,
                                      m_ui->actionAudition,
                                      m_ui->actionSelective_Time_Stretch,
                                      m_sampleBufferList,
                                      parentCommand );

        const int lowestAssignedMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

        int frameNum = 0;

        for ( int i = 0; i < m_sampleBufferList.size() - 1; i++ )
        {
            const qreal timeRatio = m_rubberbandAudioSource->getNoteTimeRatio( lowestAssignedMidiNote + i );

            const int numFrames = roundToIntAccurate( m_sampleBufferList.at( i )->getNumFrames() * timeRatio );

            frameNum += numFrames;

            new AddSlicePointItemCommand( frameNum, false, m_scene, NULL, parentCommand );
        }

        m_undoStack.push( parentCommand );
    }
    else // Disable Selective Time Stretching
    {
        QUndoCommand* parentCommand = new QUndoCommand();

        QList<SharedSlicePointItem> slicePoints = m_scene->getSlicePointList();

        foreach ( SharedSlicePointItem slicePoint, slicePoints )
        {
            new DeleteSlicePointItemCommand( slicePoint, m_scene, NULL, parentCommand );
        }

        new DisableSelectiveTSCommand( m_optionsDialog,
                                       m_scene,
                                       m_ui->pushButton_Slice,
                                       m_ui->actionAdd_Slice_Point,
                                       m_ui->actionSelect_Move,
                                       m_ui->actionMulti_Select,
                                       m_ui->actionAudition,
                                       m_ui->actionSelective_Time_Stretch,
                                       m_sampleBufferList,
                                       parentCommand );

        m_undoStack.push( parentCommand );
    }
}



void MainWindow::on_pushButton_TimestretchOptions_clicked()
{
    QPoint pos = m_optionsDialog->pos();
    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    m_optionsDialog->move( pos );
    m_optionsDialog->setCurrentTab( OptionsDialog::TIME_STRETCH_TAB );
    m_optionsDialog->show();
}



void MainWindow::on_comboBox_TimeSigNumerator_activated( const QString text )
{
    const qreal bpm = m_ui->doubleSpinBox_OriginalBPM->value();
    const int timeSigNumerator = text.toInt();

    m_scene->setBpmRulerMarks( bpm, timeSigNumerator );
}
