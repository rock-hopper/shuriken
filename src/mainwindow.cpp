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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QScrollBar>
#include "commands.h"
#include "globals.h"
#include "applygaindialog.h"
#include "applygainrampdialog.h"
#include "aboutdialog.h"
#include "messageboxes.h"
#include <rubberband/RubberBandStretcher.h>
#include <QtDebug>


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
    // Check if a file path has been passed on the command line
    if ( QApplication::arguments().size() > 1 )
    {
        m_currentProjectFilePath = QApplication::arguments().at( 1 );
    }

    // Check if non session manager is running
    const char* nsmUrl = getenv( "NSM_URL" );

    if ( nsmUrl != NULL && m_currentProjectFilePath.isEmpty() )
    {
        m_nsmThread = new NsmListenerThread();

        // Set JACK client name
        Jack::g_clientId = m_nsmThread->getJackClientId();

        // Create save dir and store path for later
        const QString savePath = m_nsmThread->getSavePath();
        const QString fileName = QString("project") + FILE_EXTENSION;

        QDir().mkpath( savePath );
        m_currentProjectFilePath = QDir( savePath ).absoluteFilePath( fileName );

        // Connect save signal to slot and start thread
        connect( m_nsmThread, SIGNAL(save()), this, SLOT(on_actionSave_Project_triggered()) );
        m_nsmThread->start();
    }

    initialiseAudio();
    setupUI();

    if ( QFileInfo( m_currentProjectFilePath ).exists() )
    {
        openProject( m_currentProjectFilePath );
    }
}



MainWindow::~MainWindow()
{
    closeProject();

    if ( m_optionsDialog != NULL )
    {
        const QString tempDirPath = m_optionsDialog->getTempDirPath();

        if ( ! tempDirPath.isEmpty() )
        {
            File( tempDirPath.toLocal8Bit().data() ).deleteRecursively();
        }
    }

    if ( m_nsmThread != NULL )
    {
        m_nsmThread->quit();
        m_nsmThread->wait( 2000 );
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
    if ( m_samplerAudioSource != NULL )
    {
        switch ( event->key() )
        {
        case Qt::Key_Space:
            if ( ! event->isAutoRepeat() ) on_pushButton_PlayStop_clicked();
            break;
        case Qt::Key_Return:
            if ( ! event->isAutoRepeat() && m_ui->pushButton_Slice->isEnabled() )
                on_pushButton_Slice_clicked( ! m_ui->pushButton_Slice->isChecked() );
            break;
        case Qt::Key_Z:
            m_graphicsScene->selectNextWaveform();
            break;
        case Qt::Key_Q:
            m_graphicsScene->selectPreviousWaveform();
            break;
        case Qt::Key_A:
            if ( m_graphicsScene->getSelectedWaveforms().size() == 1 )
            {
                playSample( m_graphicsScene->getSelectedWaveforms().first(), QPointF() );
            }
            break;
        default:
            QMainWindow::keyPressEvent( event );
            break;
        }
    }
    else
    {
        QMainWindow::keyPressEvent( event );
    }
}



void MainWindow::wheelEvent( QWheelEvent* const event )
{
    const int numDegrees = event->delta() / 8; // delta() returns how much mouse wheel was rotated in eighths of a degree
    const int numSteps = numDegrees / 15;      // Most mouse wheels work in steps of 15 degrees

    if ( event->orientation() == Qt::Vertical )
    {
        const QPoint mouseViewPos = m_ui->waveGraphicsView->mapFromGlobal( event->globalPos() );
        const QPointF mouseScenePos = m_ui->waveGraphicsView->mapToScene( mouseViewPos );
        const qreal ratio = mouseScenePos.x() / m_graphicsScene->width();

        QScrollBar* const scrollBar = m_ui->waveGraphicsView->horizontalScrollBar();

        if ( numSteps > 0 && m_ui->actionZoom_In->isEnabled() )
        {
            on_actionZoom_In_triggered();

            scrollBar->setValue( scrollBar->maximum() * ratio );

            event->accept();
        }
        else if ( numSteps < 0 && m_ui->actionZoom_Out->isEnabled() )
        {
            on_actionZoom_Out_triggered();

            scrollBar->setValue( scrollBar->maximum() * ratio );

            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->ignore();
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
    const String error = m_deviceManager.initialise( InputChannels::MAX, OutputChannels::MAX, stateXml, false );

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
        // Get the current number of output channels
        AudioDeviceManager::AudioDeviceSetup config;
        m_deviceManager.getAudioDeviceSetup( config );
        const int numOutputChans = config.outputChannels.countNumberOfSetBits();

        const RubberBandStretcher::Options options = m_optionsDialog->getStretcherOptions();
        const bool isJackSyncEnabled = m_optionsDialog->isJackSyncEnabled();

        m_rubberbandAudioSource = new RubberbandAudioSource( m_samplerAudioSource, numOutputChans, options, isJackSyncEnabled );
        m_audioSourcePlayer.setSource( m_rubberbandAudioSource );

        connect( m_optionsDialog, SIGNAL( transientsOptionChanged(RubberBandStretcher::Options) ),
                 m_rubberbandAudioSource, SLOT( setTransientsOption(RubberBandStretcher::Options) ) );

        connect( m_optionsDialog, SIGNAL( phaseOptionChanged(RubberBandStretcher::Options) ),
                 m_rubberbandAudioSource, SLOT( setPhaseOption(RubberBandStretcher::Options) ) );

        connect( m_optionsDialog, SIGNAL( formantOptionChanged(RubberBandStretcher::Options) ),
                 m_rubberbandAudioSource, SLOT( setFormantOption(RubberBandStretcher::Options) ) );

        connect( m_optionsDialog, SIGNAL( pitchOptionChanged(RubberBandStretcher::Options) ),
                 m_rubberbandAudioSource, SLOT( setPitchOption(RubberBandStretcher::Options) ) );

        connect( m_optionsDialog, SIGNAL( jackSyncToggled(bool) ),
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
    m_graphicsScene = m_ui->waveGraphicsView->getScene();

#if QT_VERSION < 0x040700
    // This must be set in Qt 4.6, otherwise the proprietary AMD video driver may cause the CPU to max out when scrolling
    m_ui->waveGraphicsView->setFrameShape( QFrame::Panel );
#endif


    // If NSM is running, change text for "Open Project" and "Save As" to "Import Project" and "Export Project"
    if ( m_nsmThread != NULL )
    {
        m_ui->actionOpen_Project->setText( tr( "Import Project" ) );
        m_ui->actionOpen_Project->setToolTip( tr( "Import Project" ) );
        m_ui->actionSave_As->setText( tr( "Export Project" ) );
        m_ui->actionSave_As->setToolTip( tr( "Export Project" ) );
    }

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
    QList<int> snapValuesDataList;

    snapValuesDataList << 64 << 32 << 28 << 24 << 20 << 16 << 14 << 12 << 10 << 8 << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0;

    for ( int i = 0; i < snapValuesDataList.size() - 2; i++ )
    {
        snapValuesTextList << tr("Beats") + " / " + QString::number( snapValuesDataList.at( i ) );
    }
    snapValuesTextList << tr( "Beats" );
    snapValuesTextList << tr( "Off" );

    for ( int i = 0; i < snapValuesTextList.size(); i++ )
    {
        m_ui->comboBox_SnapValues->addItem( snapValuesTextList[ i ], snapValuesDataList[ i ] );
    }

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


    // Set max window size and centre in desktop
    setMaxWindowSize( this );
    centreWindow( this );


    // Connect signals to slots
    connect( m_graphicsScene, SIGNAL( slicePointPosChanged(SharedSlicePointItem,int,int,int,int) ),
             this, SLOT( recordSlicePointItemMove(SharedSlicePointItem,int,int,int,int) ) );

    connect( m_ui->waveGraphicsView, SIGNAL( minDetailLevelReached() ),
             this, SLOT( disableZoomOut() ) );

    connect( m_ui->waveGraphicsView, SIGNAL( maxDetailLevelReached() ),
             this, SLOT( disableZoomIn() ) );

    connect( m_graphicsScene, SIGNAL( playheadFinishedScrolling() ),
             this, SLOT( resetPlayStopButtonIcon() ) );

    connect( m_graphicsScene, SIGNAL( selectionChanged() ),
             this, SLOT( enableEditActions() ) );

    connect( &m_undoStack, SIGNAL( canUndoChanged(bool) ),
             m_ui->actionUndo, SLOT( setEnabled(bool) ) );

    connect( &m_undoStack, SIGNAL( canRedoChanged(bool) ),
             m_ui->actionRedo, SLOT( setEnabled(bool) ) );

    connect( m_ui->actionUndo, SIGNAL( triggered() ),
             &m_undoStack, SLOT( undo() ) );

    connect( m_ui->actionRedo, SIGNAL( triggered() ),
             &m_undoStack, SLOT( redo() ) );

    connect( &m_undoStack, SIGNAL( cleanChanged(bool) ),
             m_ui->actionSave_Project, SLOT( setDisabled(bool) ) );

    connect( &m_undoStack, SIGNAL( undoTextChanged(QString) ),
             this, SLOT( updateUndoText(QString) ) );

    connect( &m_undoStack, SIGNAL( redoTextChanged(QString) ),
             this, SLOT( updateRedoText(QString) ) );

    if ( m_nsmThread != NULL )
    {
        connect( &m_undoStack, SIGNAL( cleanChanged(bool) ),
                 this, SLOT( notifyNsmOfUnsavedChanges(bool) ) );
    }


    // Create help form
    m_helpForm = new HelpForm();

    if ( m_helpForm != NULL )
    {
        setMaxWindowSize( m_helpForm );
        centreWindow( m_helpForm );
        m_ui->actionHelp->setEnabled( true );
    }


    // Create export dialog
    m_exportDialog = new ExportDialog( this );

    if ( m_exportDialog != NULL )
    {
        centreWindow( m_exportDialog );
    }


    // Create options dialog
    m_optionsDialog = new OptionsDialog( m_deviceManager, this );

    if ( m_optionsDialog != NULL )
    {
        centreWindow( m_optionsDialog );

        connect( m_optionsDialog, SIGNAL( realtimeModeToggled(bool) ),
                 this, SLOT( enableRealtimeControls(bool) ) );

        connect( m_optionsDialog, SIGNAL( jackSyncToggled(bool) ),
                 m_ui->doubleSpinBox_NewBPM, SLOT( setHidden(bool) ) );

        connect( m_optionsDialog, SIGNAL( jackSyncToggled(bool) ),
                 m_ui->label_JackSync, SLOT( setVisible(bool) ) );

        connect( m_optionsDialog, SIGNAL( timeStretchOptionsChanged() ),
                 this, SLOT( enableSaveAction() ) );

        connect( m_optionsDialog, SIGNAL( jackAudioEnabled(bool) ),
                 this, SLOT( enableJackOutputsAction(bool) ) );

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
    m_ui->lineEdit_Threshold->setEnabled( true );
    m_ui->horizontalSlider_Threshold->setEnabled( true );
    m_ui->pushButton_FindOnsets->setEnabled( true );
    m_ui->pushButton_FindBeats->setEnabled( true );
    m_ui->comboBox_TimeSigNumerator->setEnabled( true );
    m_ui->comboBox_TimeSigDenominator->setEnabled( true );
    m_ui->spinBox_Length->setEnabled( true );
    m_ui->comboBox_Units->setEnabled( true );

    m_ui->actionSave_As->setEnabled( true );
    if ( m_nsmThread == NULL )
    {
        m_ui->actionClose_Project->setEnabled( true );
    }
    m_ui->actionExport_As->setEnabled( true );
    m_ui->actionSelect_All->setEnabled( true );
    m_ui->actionSelect_None->setEnabled( true );
    m_ui->actionAdd_Slice_Point->setEnabled( true );
    m_ui->actionZoom_Original->setEnabled( true );
    m_ui->actionZoom_In->setEnabled( true );
    m_ui->actionSelect_Move->setEnabled( true );
    m_ui->actionMulti_Select->setEnabled( true );
    m_ui->actionAudition->setEnabled( true );
    if ( m_optionsDialog->isJackAudioEnabled() )
    {
        m_ui->actionJack_Outputs->setEnabled( true );
    }

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
    m_ui->lineEdit_Threshold->setEnabled( false );
    m_ui->horizontalSlider_Threshold->setEnabled( false );
    m_ui->pushButton_FindOnsets->setEnabled( false );
    m_ui->pushButton_FindBeats->setEnabled( false );
    m_ui->comboBox_SnapValues->setEnabled( false );
    m_ui->comboBox_TimeSigNumerator->setEnabled( false );
    m_ui->comboBox_TimeSigDenominator->setEnabled( false );
    m_ui->spinBox_Length->setEnabled( false );
    m_ui->spinBox_Length->setValue( 0 );
    m_ui->comboBox_Units->setEnabled( false );
    m_ui->doubleSpinBox_Attack->setEnabled( false );
    m_ui->dial_Attack->setEnabled( false );
    m_ui->doubleSpinBox_Release->setEnabled( false );
    m_ui->dial_Release->setEnabled( false );
    m_ui->checkBox_OneShot->setEnabled( false );

    m_ui->actionSave_Project->setEnabled( false );
    m_ui->actionSave_As->setEnabled( false );
    m_ui->actionClose_Project->setEnabled( false );
    m_ui->actionExport_As->setEnabled( false );
    m_ui->actionSelect_All->setEnabled( false );
    m_ui->actionSelect_None->setEnabled( false );
    m_ui->actionAdd_Slice_Point->setEnabled( false );
    m_ui->actionAdd_Slice_Point->setVisible( true );
    m_ui->actionCopy->setEnabled( false );
    m_ui->actionPaste->setEnabled( false );
    m_ui->actionDelete->setEnabled( false );
    m_ui->actionReverse->setEnabled( false );
    m_ui->actionZoom_Original->setEnabled( false );
    m_ui->actionZoom_Out->setEnabled( false );
    m_ui->actionZoom_In->setEnabled( false );
    m_ui->actionSelect_Move->setEnabled( false );
    m_ui->actionMulti_Select->setEnabled( false );
    m_ui->actionAudition->setEnabled( false );
    m_ui->actionJack_Outputs->setEnabled( false );

    if ( m_ui->actionSelective_Time_Stretch->isChecked() )
    {
        m_ui->actionSelective_Time_Stretch->trigger();
    }
    m_ui->actionSelective_Time_Stretch->setEnabled( false );

    m_optionsDialog->disableTab( OptionsDialog::TIME_STRETCH_TAB );
}



void MainWindow::connectWaveformToMainWindow( const SharedWaveformItem item )
{
    connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
             this, SLOT( recordWaveformItemMove(QList<int>,int) ) );

    connect( item.data(), SIGNAL( orderPosHasChanged(QList<int>,int) ),
             this, SLOT( stopPlayback() ) );

    connect( item.data(), SIGNAL( clicked(const WaveformItem*,QPointF) ),
             this, SLOT( playSample(const WaveformItem*,QPointF) ) );
}



void MainWindow::getDetectionSettings( AudioAnalyser::DetectionSettings& settings )
{
    int currentIndex;

    // From aubio website: "Typical threshold values are within 0.001 and 0.900." Default is 0.3 in aubio-0.4.0
    currentIndex = m_ui->comboBox_DetectMethod->currentIndex();
    settings.detectionMethod = m_ui->comboBox_DetectMethod->itemData( currentIndex ).toString().toLocal8Bit();

    settings.threshold = m_ui->horizontalSlider_Threshold->value() / 100.0;

    currentIndex = m_ui->comboBox_WindowSize->currentIndex();
    settings.windowSize = (uint_t) m_ui->comboBox_WindowSize->itemData( currentIndex ).toInt();

    currentIndex = m_ui->comboBox_HopSize->currentIndex();
    const qreal percentage = m_ui->comboBox_HopSize->itemData( currentIndex ).toReal();
    settings.hopSize = (uint_t) ( settings.windowSize * ( percentage / 100.0 ) );

    settings.sampleRate = (uint_t) m_sampleHeader->sampleRate;
}



void MainWindow::closeProject()
{
    m_copiedSampleBuffers.clear();
    m_copiedEnvelopes.attackValues.clear();
    m_copiedEnvelopes.releaseValues.clear();
    m_copiedEnvelopes.oneShotSettings.clear();
    m_copiedNoteTimeRatios.clear();

    m_sampleHeader.clear();
    m_sampleBufferList.clear();
    tearDownSampler();

    disconnect( m_graphicsScene, SIGNAL( selectionChanged() ),
                this, SLOT( enableEditActions() ) );

    m_graphicsScene->clearAll();
    on_actionZoom_Original_triggered();
    disableUI();
    m_ui->statusBar->clearMessage();

    connect( m_graphicsScene, SIGNAL( selectionChanged() ),
             this, SLOT( enableEditActions() ) );

    m_undoStack.clear();

    m_appliedBPM = 0.0;

    if ( m_nsmThread == NULL )
    {
        m_currentProjectFilePath.clear();
    }
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



QUndoCommand* MainWindow::createRenderCommand( QUndoCommand* parent )
{
    const QString tempDirPath = m_optionsDialog->getTempDirPath();

    QUndoCommand* command = NULL;

    if ( ! tempDirPath.isEmpty() )
    {
        if ( m_samplerAudioSource != NULL && m_rubberbandAudioSource != NULL )
        {
            const QString fileBaseName = QString::number( m_undoStack.index() );

            command = new RenderTimeStretchCommand( this,
                                                    m_graphicsScene,
                                                    tempDirPath,
                                                    fileBaseName,
                                                    parent );
        }
    }
    else
    {
        MessageBoxes::showWarningDialog( tr("Temp dir invalid!"),
                                         tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
    }

    return command;
}



void MainWindow::resetSamples()
{
    if ( m_samplerAudioSource != NULL &&
         ! m_sampleBufferList.isEmpty() && ! m_sampleHeader.isNull() )
    {
        SamplerAudioSource::EnvelopeSettings envelopes;

        m_samplerAudioSource->getEnvelopeSettings( envelopes );

        m_samplerAudioSource->setSamples( m_sampleBufferList, m_sampleHeader->sampleRate );

        m_samplerAudioSource->setEnvelopeSettings( envelopes );
    }
}



void MainWindow::copySelectedSamplesToClipboard()
{
    const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

    // Copy sample buffers
    m_copiedSampleBuffers.clear();

    const int numChans = m_sampleHeader->numChans;
    const int startFrame = 0;

    foreach ( int orderPos, orderPositions )
    {
        SharedSampleBuffer origSampleBuffer = m_sampleBufferList.at( orderPos );

        const int numFrames = origSampleBuffer->getNumFrames();

        SharedSampleBuffer copiedSampleBuffer( new SampleBuffer( numChans, numFrames ) );

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            copiedSampleBuffer->copyFrom( chanNum, startFrame, *origSampleBuffer.data(), chanNum, startFrame, numFrames );
        }

        m_copiedSampleBuffers << copiedSampleBuffer;
    }

    // Copy envelopes
    m_copiedEnvelopes.attackValues.clear();
    m_copiedEnvelopes.releaseValues.clear();
    m_copiedEnvelopes.oneShotSettings.clear();

    SamplerAudioSource::EnvelopeSettings envelopes;

    m_samplerAudioSource->getEnvelopeSettings( envelopes );

    foreach ( int orderPos, orderPositions )
    {
        m_copiedEnvelopes.attackValues << envelopes.attackValues.at( orderPos );
        m_copiedEnvelopes.releaseValues << envelopes.releaseValues.at( orderPos );
        m_copiedEnvelopes.oneShotSettings << envelopes.oneShotSettings.at( orderPos );
    }

    // If real-time time streching is enabled then also copy per-note time stretch ratios
    m_copiedNoteTimeRatios.clear();

    if ( m_rubberbandAudioSource != NULL )
    {
        const int startMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

        foreach ( int orderPos, orderPositions )
        {
            m_copiedNoteTimeRatios << m_rubberbandAudioSource->getNoteTimeRatio( startMidiNote + orderPos );
        }
    }
    else
    {
        for ( int i = 0; i < orderPositions.size(); i++ )
        {
            m_copiedNoteTimeRatios << 1.0;
        }
    }
}



//==================================================================================================
// Private Static:

void MainWindow::setMaxWindowSize( QWidget* const window )
{
    const int desktopWidth = QApplication::desktop()->availableGeometry().width();
    const int desktopHeight = QApplication::desktop()->availableGeometry().height();

    const int frameWidth = window->frameSize().width();
    const int frameHeight = window->frameSize().height();

    int windowWidth = window->size().width();
    int windowHeight = window->size().height();

    int maxWidth = window->maximumWidth();
    int maxHeight = window->maximumHeight();

    if ( frameWidth > desktopWidth )
    {
        windowWidth = desktopWidth - ( frameWidth - windowWidth );
        maxWidth = windowWidth;
    }

    if ( frameHeight > desktopHeight )
    {
        windowHeight = desktopHeight - ( frameHeight - windowHeight );
        maxHeight = windowHeight;
    }

    window->resize( windowWidth, windowHeight );
    window->setMaximumSize( maxWidth, maxHeight );
}



void MainWindow::centreWindow( QWidget* const window )
{
    window->setGeometry
    (
        QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, window->size(), QApplication::desktop()->availableGeometry() )
    );
}



//==================================================================================================
// Private Slots:

void MainWindow::recordWaveformItemMove( QList<int> oldOrderPositions, const int numPlacesMoved )
{
    QUndoCommand* command = new MoveWaveformItemCommand( oldOrderPositions, numPlacesMoved, m_graphicsScene, this );
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
            parentCommand->setText( tr("Selective Time Stretch") );

            new MoveSlicePointItemCommand( slicePoint, oldFrameNum, m_graphicsScene, parentCommand );

            QList<int> orderPositions;
            QList<qreal> timeRatios;
            QList<int> midiNotes;

            orderPositions << orderPos << orderPos + 1;

            timeRatios << (qreal) numFramesFromPrevSlicePoint / m_sampleBufferList.at( orderPos )->getNumFrames();
            timeRatios << (qreal) numFramesToNextSlicePoint / m_sampleBufferList.at( orderPos + 1 )->getNumFrames();

            midiNotes << m_samplerAudioSource->getLowestAssignedMidiNote() + orderPos;
            midiNotes << m_samplerAudioSource->getLowestAssignedMidiNote() + orderPos + 1;

            new SelectiveTimeStretchCommand( this,
                                             m_graphicsScene,
                                             orderPositions,
                                             timeRatios,
                                             midiNotes,
                                             parentCommand );

            m_undoStack.push( parentCommand );
        }
    }
    else
    {
        QUndoCommand* command = new MoveSlicePointItemCommand( slicePoint, oldFrameNum, m_graphicsScene );
        m_undoStack.push( command );
    }
}



void MainWindow::playSample( const WaveformItem* waveformItem, const QPointF mouseScenePos )
{
    SharedSampleRange sampleRange( new SampleRange );
    sampleRange->startFrame = 0;
    sampleRange->numFrames = waveformItem->getSampleBuffer()->getNumFrames();

    qreal startPosX = waveformItem->scenePos().x();
    qreal endPosX = startPosX + waveformItem->rect().width();

    const QList<int> slicePointFrameNums = m_graphicsScene->getSlicePointFrameNums();

    // If slice points are present and the waveform has not yet been sliced...
    if ( slicePointFrameNums.size() > 0 && m_sampleBufferList.size() == 1 )
    {
        const int mousePosFrameNum = m_graphicsScene->getFrameNum( mouseScenePos.x() );
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

        startPosX = m_graphicsScene->getScenePosX( sampleRange->startFrame );
        endPosX = m_graphicsScene->getScenePosX( endFrame );
    }

    // Play sample range and start playhead scrolling
    m_samplerAudioSource->playSample( waveformItem->getOrderPos(), sampleRange );
    m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-stop.png" ) );

    if ( m_rubberbandAudioSource != NULL && m_ui->checkBox_TimeStretch->isChecked() )
    {
        qreal globalStretchRatio = 1.0;

        if ( m_ui->doubleSpinBox_OriginalBPM->value() > 0.0 && m_ui->doubleSpinBox_NewBPM->value() > 0.0 )
        {
            globalStretchRatio = m_ui->doubleSpinBox_OriginalBPM->value() / m_ui->doubleSpinBox_NewBPM->value();
        }

        const int startMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

        const qreal noteTimeRatio = m_rubberbandAudioSource->getNoteTimeRatio( startMidiNote + waveformItem->getOrderPos() );

        const qreal stretchRatio = globalStretchRatio * noteTimeRatio;

        m_graphicsScene->startPlayhead( startPosX, endPosX, sampleRange->numFrames, m_ui->pushButton_Loop->isChecked(), stretchRatio );
    }
    else
    {
        m_graphicsScene->startPlayhead( startPosX, endPosX, sampleRange->numFrames, m_ui->pushButton_Loop->isChecked() );
    }
}



void MainWindow::stopPlayback()
{
    if ( m_samplerAudioSource != NULL )
    {
        m_samplerAudioSource->stop();
    }
    m_graphicsScene->stopPlayhead();
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
                          this, SLOT( recreateSampler() ) );
    }
    else // Offline mode
    {
        m_ui->checkBox_TimeStretch->setVisible( false );
        m_ui->pushButton_Apply->setVisible( true );

        m_ui->actionSelective_Time_Stretch->setEnabled( false );

        QObject::disconnect( m_optionsDialog, SIGNAL( windowOptionChanged() ),
                             this, SLOT( recreateSampler() ) );

        if ( isSelectiveTimeStretchInUse() )
        {
            QUndoCommand* command = createRenderCommand();

            if ( command != NULL )
            {
                m_undoStack.push( command );
            }
        }
    }

    recreateSampler();
}



void MainWindow::recreateSampler()
{
    SamplerAudioSource::EnvelopeSettings envelopes;

    if ( m_samplerAudioSource != NULL )
    {
        m_samplerAudioSource->getEnvelopeSettings( envelopes );
    }

    tearDownSampler();
    setUpSampler();

    m_samplerAudioSource->setEnvelopeSettings( envelopes );
}



void MainWindow::enableEditActions()
{
    const SharedSlicePointItem slicePoint = m_graphicsScene->getSelectedSlicePoint();
    const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

    // Enable/disable delete action
    m_ui->actionDelete->setEnabled( false );

    if ( ! m_ui->actionSelective_Time_Stretch->isChecked() )
    {
        if ( ! slicePoint.isNull() || ! orderPositions.isEmpty() )
        {
            if ( orderPositions.size() < m_graphicsScene->getNumWaveforms() )
            {
                m_ui->actionDelete->setEnabled( true );
            }
        }
    }

    // Enable/disable other edit actions
    if ( ! orderPositions.isEmpty() )
    {
        m_ui->actionCopy->setEnabled( true );
        m_ui->actionApply_Gain->setEnabled( true );
        m_ui->actionApply_Gain_Ramp->setEnabled( true );
        m_ui->actionNormalise->setEnabled( true );
        m_ui->actionReverse->setEnabled( true );
    }
    else
    {
        m_ui->actionCopy->setEnabled( false );
        m_ui->actionApply_Gain->setEnabled( false );
        m_ui->actionApply_Gain_Ramp->setEnabled( false );
        m_ui->actionNormalise->setEnabled( false );
        m_ui->actionReverse->setEnabled( false );
    }

    // Enable/disable envelope widgets
    if ( orderPositions.size() == 1 )
    {
        disconnect( m_ui->doubleSpinBox_Attack, SIGNAL( valueChanged(double) ),
                    this, SLOT( on_doubleSpinBox_Attack_valueChanged(double) ) );

        disconnect( m_ui->dial_Attack, SIGNAL( valueChanged(int) ),
                    this, SLOT( on_dial_Attack_valueChanged(int) ) );

        disconnect( m_ui->doubleSpinBox_Release, SIGNAL( valueChanged(double) ),
                    this, SLOT( on_doubleSpinBox_Release_valueChanged(double) ) );

        disconnect( m_ui->dial_Release, SIGNAL( valueChanged(int) ),
                    this, SLOT( on_dial_Release_valueChanged(int) ) );

        const qreal attackValue = m_samplerAudioSource->getAttack( orderPositions.first() );
        const qreal releaseValue = m_samplerAudioSource->getRelease( orderPositions.first() );

        m_ui->doubleSpinBox_Attack->setValue( attackValue );
        m_ui->dial_Attack->setValue( attackValue * 100 );

        m_ui->doubleSpinBox_Release->setValue( releaseValue );
        m_ui->dial_Release->setValue( releaseValue * 100 );

        connect( m_ui->doubleSpinBox_Attack, SIGNAL( valueChanged(double) ),
                 this, SLOT( on_doubleSpinBox_Attack_valueChanged(double) ) );

        connect( m_ui->dial_Attack, SIGNAL( valueChanged(int) ),
                 this, SLOT( on_dial_Attack_valueChanged(int) ) );

        connect( m_ui->doubleSpinBox_Release, SIGNAL( valueChanged(double) ),
                 this, SLOT( on_doubleSpinBox_Release_valueChanged(double) ) );

        connect( m_ui->dial_Release, SIGNAL( valueChanged(int) ),
                 this, SLOT( on_dial_Release_valueChanged(int) ) );

        m_ui->doubleSpinBox_Attack->setEnabled( true );
        m_ui->dial_Attack->setEnabled( true );

        const bool isOneShotSet = m_samplerAudioSource->isOneShotSet( orderPositions.first() );

        // Setting this will enable/disable the "Release" spin box and dial
        m_ui->checkBox_OneShot->setChecked( isOneShotSet );

        m_ui->checkBox_OneShot->setEnabled( true );
    }
    else
    {
        m_ui->doubleSpinBox_Attack->setEnabled( false );
        m_ui->dial_Attack->setEnabled( false );

        m_ui->doubleSpinBox_Release->setEnabled( false );
        m_ui->dial_Release->setEnabled( false );

        m_ui->doubleSpinBox_Attack->setValue( 0 );
        m_ui->doubleSpinBox_Release->setValue( 0 );

        m_ui->checkBox_OneShot->setEnabled( false );
    }
}



void MainWindow::enableSaveAction()
{
    if ( m_isProjectOpen )
    {
        m_ui->actionSave_Project->setEnabled( true );
    }
}



void MainWindow::updateUndoText( const QString text )
{
    m_ui->actionUndo->setText( tr("Undo ") + text );
}



void MainWindow::updateRedoText( const QString text )
{
    m_ui->actionRedo->setText( tr("Redo ") + text );
}



void MainWindow::notifyNsmOfUnsavedChanges( const bool isClean )
{
    if ( m_nsmThread != NULL )
    {
        if ( isClean )
        {
            m_nsmThread->sendMessage( NsmListenerThread::MSG_IS_CLEAN );
        }
        else
        {
            m_nsmThread->sendMessage( NsmListenerThread::MSG_IS_DIRTY );
        }
    }
}



void MainWindow::enableJackOutputsAction( const bool isJackAudioEnabled )
{
    if ( isJackAudioEnabled && ! m_sampleBufferList.isEmpty() && ! m_sampleHeader.isNull() )
    {
        m_ui->actionJack_Outputs->setEnabled( true );
    }
    else
    {
        m_ui->actionJack_Outputs->setEnabled( false );
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
    m_graphicsScene->selectAll();
}



void MainWindow::on_actionSelect_None_triggered()
{
    m_graphicsScene->selectNone();
}



void MainWindow::on_actionCopy_triggered()
{
    copySelectedSamplesToClipboard();

    m_ui->actionPaste->setEnabled( true );
}



void MainWindow::on_actionPaste_triggered()
{
    if ( m_copiedSampleBuffers.size() > 0 )
    {
        QList<int> selectedOrderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

        int orderPosToInsertAt = m_sampleBufferList.size();

        if ( selectedOrderPositions.size() > 0 )
        {
            orderPosToInsertAt = selectedOrderPositions.last() + 1;
        }

        QUndoCommand* command = new PasteWaveformItemCommand( m_copiedSampleBuffers,
                                                              m_copiedEnvelopes,
                                                              m_copiedNoteTimeRatios,
                                                              orderPosToInsertAt,
                                                              m_graphicsScene,
                                                              this );
        m_undoStack.push( command );
    }
}



void MainWindow::on_actionDelete_triggered()
{    
    const SharedSlicePointItem selectedSlicePoint = m_graphicsScene->getSelectedSlicePoint();

    if ( ! selectedSlicePoint.isNull() )
    {
        selectedSlicePoint->setSelected( false );

        QUndoCommand* command = new DeleteSlicePointItemCommand( selectedSlicePoint,
                                                                 m_graphicsScene,
                                                                 m_ui->pushButton_Slice,
                                                                 m_ui->comboBox_SnapValues );
        m_undoStack.push( command );
    }
    else
    {
        const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

        if ( ! orderPositions.isEmpty() )
        {
            foreach ( int orderPos, orderPositions )
            {
                m_graphicsScene->getWaveformAt( orderPos )->setSelected( false );
            }

            QUndoCommand* command = new DeleteWaveformItemCommand( orderPositions,
                                                                   m_graphicsScene,
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
    const int frameNum = m_graphicsScene->getFrameNum( mouseScenePos.x() );

    QUndoCommand* command = new AddSlicePointItemCommand( frameNum, true, m_graphicsScene, m_ui->pushButton_Slice, m_ui->comboBox_SnapValues );
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
            const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

            QUndoCommand* parentCommand = new QUndoCommand();
            parentCommand->setText( tr("Apply Gain") );

            const QString stackIndex = QString::number( m_undoStack.index() );
            int i = 0;

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName = stackIndex + "_" + QString::number( i++ );

                new ApplyGainCommand( dialog.getGainValue(),
                                      orderPos,
                                      m_graphicsScene,
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
            const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

            QUndoCommand* parentCommand = new QUndoCommand();
            parentCommand->setText( tr("Apply Gain Ramp") );

            const QString stackIndex = QString::number( m_undoStack.index() );
            int i = 0;

            foreach ( int orderPos, orderPositions )
            {
                QString fileBaseName = stackIndex + "_" + QString::number( i++ );

                new ApplyGainRampCommand( dialog.getStartGainValue(),
                                          dialog.getEndGainValue(),
                                          orderPos,
                                          m_graphicsScene,
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
        const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

        QUndoCommand* parentCommand = new QUndoCommand();
        parentCommand->setText( tr("Normalise") );

        const QString stackIndex = QString::number( m_undoStack.index() );
        int i = 0;

        foreach ( int orderPos, orderPositions )
        {
            QString fileBaseName = stackIndex + "_" + QString::number( i++ );

            new NormaliseCommand( orderPos,
                                  m_graphicsScene,
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



void MainWindow::on_actionReverse_triggered()
{
    const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

    QUndoCommand* parentCommand = new QUndoCommand();
    parentCommand->setText( tr("Reverse") );

    foreach ( int orderPos, orderPositions )
    {
        new ReverseCommand( orderPos, m_graphicsScene, parentCommand );
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

    const int numFrames = SampleUtils::getTotalNumFrames( m_sampleBufferList );

    const qreal numSeconds = numFrames / m_sampleHeader->sampleRate;

    const qreal bpm = numBeats / ( numSeconds / 60 );

    m_ui->doubleSpinBox_OriginalBPM->setValue( bpm );
    m_ui->doubleSpinBox_NewBPM->setValue( bpm );

    const int index = m_ui->comboBox_SnapValues->currentIndex();
    const int divisionsPerBeat = m_ui->comboBox_SnapValues->itemData( index ).toInt();

    m_graphicsScene->setBpmRulerMarks( bpm, numerator, divisionsPerBeat  );

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
        parentCommand->setText( tr("Slice") );

        new SliceCommand( this,
                          m_graphicsScene,
                          m_ui->pushButton_Slice,
                          m_ui->pushButton_FindOnsets,
                          m_ui->pushButton_FindBeats,
                          m_ui->actionAdd_Slice_Point,
                          m_ui->actionSelect_Move,
                          m_ui->actionAudition,
                          m_ui->actionSelective_Time_Stretch,
                          parentCommand );

        QList<SharedSlicePointItem> slicePoints = m_graphicsScene->getSlicePointList();

        foreach ( SharedSlicePointItem slicePoint, slicePoints )
        {
            new DeleteSlicePointItemCommand( slicePoint, m_graphicsScene, m_ui->comboBox_SnapValues, parentCommand );
        }

        m_undoStack.push( parentCommand );
    }
    else // Unslice
    {
        QUndoCommand* parentCommand = new QUndoCommand();
        parentCommand->setText( tr("Unslice") );

        if ( isSelectiveTimeStretchInUse() )
        {
            const int startMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();
            int frameNum = 0;

            for ( int i = 0; i < m_sampleBufferList.size() - 1; i++ )
            {
                const int midiNote = startMidiNote + i;
                const qreal timeRatio = m_rubberbandAudioSource->getNoteTimeRatio( midiNote );

                frameNum += roundToIntAccurate( m_sampleBufferList.at( i )->getNumFrames() * timeRatio );

                new AddSlicePointItemCommand( frameNum, true, m_graphicsScene, m_ui->comboBox_SnapValues, parentCommand );
            }

            createRenderCommand( parentCommand );
        }
        else
        {
            int frameNum = 0;

            for ( int i = 0; i < m_sampleBufferList.size() - 1; i++ )
            {
                frameNum += m_sampleBufferList.at( i )->getNumFrames();

                new AddSlicePointItemCommand( frameNum, true, m_graphicsScene, m_ui->comboBox_SnapValues, parentCommand );
            }
        }

        new UnsliceCommand( this,
                            m_graphicsScene,
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
    m_ui->lineEdit_Threshold->setText( QString::number( value / 100.0, 'f', 2 ) );
}



void MainWindow::on_pushButton_FindOnsets_clicked()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    QUndoCommand* parentCommand = new QUndoCommand();
    parentCommand->setText( tr("Find Onsets") );

    // Remove current slice points if present
    {
        const QList<SharedSlicePointItem> slicePointItemList = m_graphicsScene->getSlicePointList();

        foreach ( SharedSlicePointItem item, slicePointItemList )
        {
            new DeleteSlicePointItemCommand( item, m_graphicsScene, m_ui->pushButton_Slice, m_ui->comboBox_SnapValues, parentCommand );
        }
    }

    // Add new slice points
    {
        AudioAnalyser::DetectionSettings settings;
        getDetectionSettings( settings );

        const QList<int> slicePointFrameNumList = AudioAnalyser::findOnsetFrameNums( m_sampleBufferList.first(), settings );

        foreach ( int frameNum, slicePointFrameNumList )
        {
            new AddSlicePointItemCommand( frameNum, true, m_graphicsScene, m_ui->pushButton_Slice, m_ui->comboBox_SnapValues, parentCommand );
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
    const QList<SharedSlicePointItem> slicePointItemList = m_graphicsScene->getSlicePointList();

    QUndoCommand* parentCommand = new QUndoCommand();
    parentCommand->setText( tr("Find Beats") );

    foreach ( SharedSlicePointItem item, slicePointItemList )
    {
        new DeleteSlicePointItemCommand( item, m_graphicsScene, m_ui->pushButton_Slice, m_ui->comboBox_SnapValues, parentCommand );
    }

    foreach ( int frameNum, slicePointFrameNumList )
    {
        new AddSlicePointItemCommand( frameNum, true, m_graphicsScene, m_ui->pushButton_Slice, m_ui->comboBox_SnapValues, parentCommand );
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

            m_graphicsScene->updatePlayheadSpeed( timeRatio );
        }
    }

    const int timeSigNumerator = m_ui->comboBox_TimeSigNumerator->currentText().toInt();

    const int index = m_ui->comboBox_SnapValues->currentIndex();
    const int divisionsPerBeat = m_ui->comboBox_SnapValues->itemData( index ).toInt();

    m_graphicsScene->setBpmRulerMarks( originalBPM, timeSigNumerator, divisionsPerBeat );
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

            m_graphicsScene->updatePlayheadSpeed( timeRatio );
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
    if ( m_graphicsScene->isPlayheadScrolling() )
    {
        m_samplerAudioSource->stop();
        m_graphicsScene->stopPlayhead();
        m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-start.png" ) );
    }
    else
    {
        m_samplerAudioSource->playAll();
        
        m_ui->pushButton_PlayStop->setIcon( QIcon( ":/resources/images/media-playback-stop.png" ) );

        if ( m_rubberbandAudioSource != NULL &&
             m_ui->checkBox_TimeStretch->isChecked() &&
             m_ui->doubleSpinBox_OriginalBPM->value() > 0.0 &&
             m_ui->doubleSpinBox_NewBPM->value() > 0.0 )
        {
            qreal stretchRatio = m_ui->doubleSpinBox_OriginalBPM->value() / m_ui->doubleSpinBox_NewBPM->value();
            m_graphicsScene->startPlayhead( m_ui->pushButton_Loop->isChecked(), stretchRatio );
        }
        else
        {
            m_graphicsScene->startPlayhead( m_ui->pushButton_Loop->isChecked() );
        }
    }
}



void MainWindow::on_pushButton_Loop_clicked( const bool isChecked )
{
    if ( m_samplerAudioSource != NULL )
    {
        m_samplerAudioSource->setLooping( isChecked );
    }
    m_graphicsScene->setPlayheadLooping( isChecked );
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
                                                                  m_graphicsScene,
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
    m_graphicsScene->setInteractionMode( WaveGraphicsScene::SELECT_MOVE_ITEMS );
}



void MainWindow::on_actionMulti_Select_triggered()
{
    m_graphicsScene->setInteractionMode( WaveGraphicsScene::MULTI_SELECT_ITEMS );
}



void MainWindow::on_actionAudition_triggered()
{
    m_graphicsScene->setInteractionMode( WaveGraphicsScene::AUDITION_ITEMS );
}



void MainWindow::on_actionSelective_Time_Stretch_triggered( const bool isChecked )
{
    if ( isChecked ) // Enable Selective Time Stretching
    {
        QUndoCommand* parentCommand = new QUndoCommand();
        parentCommand->setText( tr("Enable Selective Time Stretching") );

        new EnableSelectiveTSCommand( this,
                                      m_optionsDialog,
                                      m_graphicsScene,
                                      m_ui->pushButton_Slice,
                                      m_ui->actionAdd_Slice_Point,
                                      m_ui->actionSelect_Move,
                                      m_ui->actionMulti_Select,
                                      m_ui->actionAudition,
                                      m_ui->actionSelective_Time_Stretch,
                                      m_ui->actionPaste,
                                      m_sampleBufferList,
                                      parentCommand );

        const int lowestAssignedMidiNote = m_samplerAudioSource->getLowestAssignedMidiNote();

        int frameNum = 0;

        for ( int i = 0; i < m_sampleBufferList.size() - 1; i++ )
        {
            const qreal timeRatio = m_rubberbandAudioSource->getNoteTimeRatio( lowestAssignedMidiNote + i );

            const int numFrames = roundToIntAccurate( m_sampleBufferList.at( i )->getNumFrames() * timeRatio );

            frameNum += numFrames;

            new AddSlicePointItemCommand( frameNum, false, m_graphicsScene, m_ui->comboBox_SnapValues, parentCommand );
        }

        m_undoStack.push( parentCommand );
    }
    else // Disable Selective Time Stretching
    {
        QUndoCommand* parentCommand = new QUndoCommand();
        parentCommand->setText( tr("Disable Selective Time Stretching") );

        QList<SharedSlicePointItem> slicePoints = m_graphicsScene->getSlicePointList();

        foreach ( SharedSlicePointItem slicePoint, slicePoints )
        {
            new DeleteSlicePointItemCommand( slicePoint, m_graphicsScene, m_ui->comboBox_SnapValues, parentCommand );
        }

        new DisableSelectiveTSCommand( this,
                                       m_optionsDialog,
                                       m_graphicsScene,
                                       m_ui->pushButton_Slice,
                                       m_ui->actionAdd_Slice_Point,
                                       m_ui->actionSelect_Move,
                                       m_ui->actionMulti_Select,
                                       m_ui->actionAudition,
                                       m_ui->actionSelective_Time_Stretch,
                                       m_ui->actionPaste,
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

    const int index = m_ui->comboBox_SnapValues->currentIndex();
    const int divisionsPerBeat = m_ui->comboBox_SnapValues->itemData( index ).toInt();

    m_graphicsScene->setBpmRulerMarks( bpm, timeSigNumerator, divisionsPerBeat );
}



void MainWindow::on_comboBox_SnapValues_activated( const int index )
{
    const qreal bpm = m_ui->doubleSpinBox_OriginalBPM->value();
    const int numerator = m_ui->comboBox_TimeSigNumerator->currentText().toInt();
    const int divisionsPerBeat = m_ui->comboBox_SnapValues->itemData( index ).toInt();

    m_graphicsScene->setBpmRulerMarks( bpm, numerator, divisionsPerBeat );

    QList<SharedSlicePointItem> slicePointList = m_graphicsScene->getSlicePointList();

    if ( m_ui->comboBox_SnapValues->currentText() == tr( "Off" ) )
    {
        foreach ( SharedSlicePointItem slicePoint, slicePointList )
        {
            slicePoint->setSnap( false );
        }
    }
    else
    {
        foreach ( SharedSlicePointItem slicePoint, slicePointList )
        {
            slicePoint->setSnap( true );
        }
    }
}



void MainWindow::on_toolButton_LeftArrow_clicked()
{
    int index = m_ui->stackedWidget->currentIndex();

    index = index > 0 ? index - 1 : m_ui->stackedWidget->count() - 1;

    m_ui->stackedWidget->setCurrentIndex( index );
}



void MainWindow::on_toolButton_RightArrow_clicked()
{
    int index = m_ui->stackedWidget->currentIndex();

    index = index < m_ui->stackedWidget->count() - 1 ? index + 1 : 0;

    m_ui->stackedWidget->setCurrentIndex( index );
}



void MainWindow::on_doubleSpinBox_Attack_valueChanged( const double value )
{
    const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

    if ( orderPositions.size() == 1 )
    {
        m_samplerAudioSource->setAttack( orderPositions.first(), value );
    }

    disconnect( m_ui->dial_Attack, SIGNAL( valueChanged(int) ),
                this, SLOT( on_dial_Attack_valueChanged(int) ) );

    m_ui->dial_Attack->setValue( value * 100 );

    connect( m_ui->dial_Attack, SIGNAL( valueChanged(int) ),
             this, SLOT( on_dial_Attack_valueChanged(int) ) );
}



void MainWindow::on_dial_Attack_valueChanged( const int value )
{
    m_ui->doubleSpinBox_Attack->setValue( value / 100.0 );
}



void MainWindow::on_doubleSpinBox_Release_valueChanged( const double value )
{
    const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

    if ( orderPositions.size() == 1 )
    {
        m_samplerAudioSource->setRelease( orderPositions.first(), value );
    }

    disconnect( m_ui->dial_Release, SIGNAL( valueChanged(int) ),
                this, SLOT( on_dial_Release_valueChanged(int) ) );

    m_ui->dial_Release->setValue( value * 100 );

    connect( m_ui->dial_Release, SIGNAL( valueChanged(int) ),
             this, SLOT( on_dial_Release_valueChanged(int) ) );
}



void MainWindow::on_dial_Release_valueChanged( const int value )
{
    m_ui->doubleSpinBox_Release->setValue( value / 100.0 );
}



void MainWindow::on_checkBox_OneShot_toggled( const bool isChecked )
{
    const QList<int> orderPositions = m_graphicsScene->getSelectedWaveformsOrderPositions();

    if ( orderPositions.size() == 1 )
    {
        m_samplerAudioSource->setOneShot( orderPositions.first(), isChecked );

        m_ui->doubleSpinBox_Release->setDisabled( isChecked );
        m_ui->dial_Release->setDisabled( isChecked );
    }
}



void MainWindow::on_actionJack_Outputs_triggered()
{
    if ( ! m_sampleBufferList.isEmpty() && ! m_sampleHeader.isNull() )
    {
        QList<int> sampleOutputPairs;

        for ( int i = 0; i < m_sampleBufferList.size(); i++ )
        {
            sampleOutputPairs << m_samplerAudioSource->getOutputPairNum( i );
        }

        ScopedPointer<JackOutputsDialog> dialog( new JackOutputsDialog( m_sampleBufferList.size(),
                                                                        sampleOutputPairs,
                                                                        m_deviceManager ) );
        if ( dialog != NULL )
        {
            setMaxWindowSize( dialog );
            centreWindow( dialog );

            if ( m_rubberbandAudioSource != NULL ) // Real-time time stretch mode
            {
                connect( dialog, SIGNAL( numOutputsChanged(int) ),
                         this, SLOT( recreateSampler() ) );
            }

            connect( dialog, SIGNAL( outputPairChanged(int,int) ),
                     this, SLOT( stopPlayback() ) );

            connect( dialog, SIGNAL( outputPairChanged(int,int) ),
                     m_samplerAudioSource, SLOT( setOutputPair(int,int) ) );

            dialog->exec();
        }
    }
}
