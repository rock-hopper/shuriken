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
#include <aubio/aubio.h>
#include "commands.h"
//#include <QDebug>


//==================================================================================================
// Public:

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    mUI( new Ui::MainWindow )
{
    // Set up user interface
    mUI->setupUi( this );

    mUI->actionAdd_Slice_Point->setEnabled( false );
    mUI->actionUndo->setEnabled( false );
    mUI->actionRedo->setEnabled( false );


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
    QObject::connect( mUI->zoomSlider, SIGNAL( valueChanged(int) ),
                      mUI->waveGraphicsView, SLOT( setZoom(int) ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( waveformSliceOrderChanged(int,int) ),
                      this, SLOT( reorderSampleBufferList(int,int) ) );

    QObject::connect( mUI->waveGraphicsView, SIGNAL( slicePointOrderChanged(SharedSlicePointItem,int,int) ),
                      this, SLOT( recordSlicePointItemMove(SharedSlicePointItem,int,int) ) );

    QObject::connect( &mUndoStack, SIGNAL( canUndoChanged(bool) ),
                      mUI->actionUndo, SLOT( setEnabled(bool) ) );

    QObject::connect( &mUndoStack, SIGNAL( canRedoChanged(bool) ),
                      mUI->actionRedo, SLOT( setEnabled(bool) ) );

    QObject::connect( mUI->actionUndo, SIGNAL( triggered() ),
                      &mUndoStack, SLOT( undo() ) );

    QObject::connect( mUI->actionRedo, SIGNAL( triggered() ),
                      &mUndoStack, SLOT( redo() ) );


    // Initialise the audio device manager
    const String error = mDeviceManager.initialise( NUM_INPUT_CHANS, NUM_OUTPUT_CHANS, NULL, true );

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error initialising audio device manager!"), error.toRawUTF8() );
        mUI->actionAudio_Setup->setDisabled( true );
        mIsAudioInitialised = FALSE;
    }
    else
    {
        mAudioSetupDialog = new AudioSetupDialog( mDeviceManager, this );
        mSamplerAudioSource = new SamplerAudioSource();
        mIsAudioInitialised = TRUE;
    }


    // Check there were no errors while initialising the audio file handler
    if ( ! mFileHandler.getLastErrorTitle().isEmpty() )
    {
        showWarningBox( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }


    mLastOpenedDir = QDir::homePath();
}



MainWindow::~MainWindow()
{
    on_actionClose_Project_triggered();
    delete mUI;
}



//==================================================================================================
// Protected:

void MainWindow::changeEvent( QEvent* e )
{
    QMainWindow::changeEvent( e );
    switch ( e->type() )
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

MainWindow::DetectionSettings MainWindow::getDetectionSettings()
{
    DetectionSettings settings;
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



QList<int> MainWindow::getAmendedSlicePointFrameNumList()
{
    const QList<int> slicePointFrameNumList = mUI->waveGraphicsView->getSlicePointFrameNumList();
    const int numFrames = mCurrentSampleBuffer->getNumFrames();
    const qreal sampleRate = mCurrentSampleHeader->sampleRate;
    const int minNumFramesBetweenSlicePoints = (int) floor( sampleRate * MIN_INTER_ONSET_SECS );

    QList<int> amendedSlicePointList;
    int prevSlicePointFrameNum = 0;

    foreach ( int slicePointFrameNum, slicePointFrameNumList )
    {
        if ( slicePointFrameNum > minNumFramesBetweenSlicePoints &&
             slicePointFrameNum < numFrames - minNumFramesBetweenSlicePoints )
        {
            if ( slicePointFrameNum > prevSlicePointFrameNum + minNumFramesBetweenSlicePoints )
            {
                amendedSlicePointList.append( slicePointFrameNum );
                prevSlicePointFrameNum = slicePointFrameNum;
            }
        }
    }

    return amendedSlicePointList;
}



//==================================================================================================
// Private Static:

void MainWindow::showWarningBox( const QString text, const QString infoText )
{
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setText( text );
    msgBox.setInformativeText( infoText );
    msgBox.exec();
}



QList<int> MainWindow::calcSlicePointFrameNums( const SharedSampleBuffer sampleBuffer, const aubioRoutine routine, const DetectionSettings settings )
{
    char_t* detectionMethod = (char_t*) settings.detectionMethod.data();
    smpl_t threshold = settings.threshold;
    uint_t windowSize = settings.windowSize;
    uint_t hopSize = settings.hopSize;
    uint_t sampleRate = settings.sampleRate;
    fvec_t* detectionResultVector;
    fvec_t* inputBuffer;

    QList<int> slicePointFrameNumList;

    const int numFrames = sampleBuffer->getNumFrames();

    if ( routine == ONSET_DETECTION )
    {
        const int vectorSize = 1;
        const int onsetData = 0;
        int slicePointFrameNum = 0;

        // Create onset detector and detection result vector
        aubio_onset_t* onsetDetector = new_aubio_onset( detectionMethod, windowSize, hopSize, sampleRate );
        aubio_onset_set_threshold( onsetDetector, threshold );
        aubio_onset_set_minioi_s( onsetDetector, MIN_INTER_ONSET_SECS );
        detectionResultVector = new_fvec( vectorSize );

        inputBuffer = new_fvec( hopSize );

        // Do onset detection
        for ( int frameNum = 0; frameNum < numFrames; frameNum += hopSize )
        {
            fillAubioInputBuffer( inputBuffer, sampleBuffer, frameNum );

            aubio_onset_do( onsetDetector, inputBuffer, detectionResultVector );

            // If an onset is detected add a new slice point to the list
            if ( detectionResultVector->data[ onsetData ] )
            {
                slicePointFrameNum = aubio_onset_get_last( onsetDetector );
                slicePointFrameNumList.append( slicePointFrameNum );
            }
        }

        // Delete onset detector
        del_aubio_onset( onsetDetector );
    }
    else if ( routine == BEAT_DETECTION )
    {
        const int vectorSize = 2;
        const int beatData = 0;
        const int onsetData = 1;
        int slicePointFrameNum = 0;

        // Create beat detector and detection result vector
        aubio_tempo_t* beatDetector = new_aubio_tempo( detectionMethod, windowSize, hopSize, sampleRate );
        aubio_tempo_set_threshold( beatDetector, threshold );
        detectionResultVector = new_fvec( vectorSize );

        inputBuffer = new_fvec( hopSize );

        // Do beat detection
        for ( int frameNum = 0; frameNum < numFrames; frameNum += hopSize )
        {
            fillAubioInputBuffer( inputBuffer, sampleBuffer, frameNum );

            aubio_tempo_do( beatDetector, inputBuffer, detectionResultVector );

            // If a beat of the bar (tactus) is detected add a new slice point to the list
            if ( detectionResultVector->data[ beatData ] )
            {
                slicePointFrameNum = aubio_tempo_get_last( beatDetector );
                slicePointFrameNumList.append( slicePointFrameNum );
            }
        }

        // Delete beat detector
        del_aubio_tempo( beatDetector );
    }

    // Clean up memory
    del_fvec( detectionResultVector );
    del_fvec( inputBuffer );
    aubio_cleanup();

    return slicePointFrameNumList;
}



qreal MainWindow::calcBPM( const SharedSampleBuffer sampleBuffer, const DetectionSettings settings )
{
    char_t* detectionMethod = (char_t*) settings.detectionMethod.data();
    smpl_t threshold = settings.threshold;
    uint_t windowSize = settings.windowSize;
    uint_t hopSize = settings.hopSize;
    uint_t sampleRate = settings.sampleRate;

    fvec_t* detectionResultVector;
    fvec_t* inputBuffer;

    const int numFrames = sampleBuffer->getNumFrames();

    const int vectorSize = 2;
    const int beatData = 0;
    const int onsetData = 1;

    int numDetections = 0;
    qreal currentBPM = 0.0;
    qreal summedBPMs = 0.0;
    qreal averageBPM = 0.0;
    qreal confidence = 0.0;

    // Create beat detector and detection result vector
    aubio_tempo_t* beatDetector = new_aubio_tempo( detectionMethod, windowSize, hopSize, sampleRate );
    aubio_tempo_set_threshold( beatDetector, threshold );
    detectionResultVector = new_fvec( vectorSize );

    inputBuffer = new_fvec( hopSize );

    // Do bpm detection
    for ( int frameNum = 0; frameNum < numFrames; frameNum += hopSize )
    {
        fillAubioInputBuffer( inputBuffer, sampleBuffer, frameNum );

        aubio_tempo_do( beatDetector, inputBuffer, detectionResultVector );

        // If a beat of the bar (tactus) is detected get the current BPM
        if ( detectionResultVector->data[ beatData ] )
        {
            currentBPM = aubio_tempo_get_bpm( beatDetector );
            confidence = aubio_tempo_get_confidence( beatDetector );

            if ( currentBPM > 0.0 && confidence > 0.1 )
            {
                summedBPMs += currentBPM;
                numDetections++;
            }
        }
    }

    // Delete beat detector
    del_aubio_tempo( beatDetector );

    // Clean up memory
    del_fvec( detectionResultVector );
    del_fvec( inputBuffer );
    aubio_cleanup();

    if ( numDetections > 0 )
    {
         averageBPM = floor( (summedBPMs / numDetections) + 0.5 );
    }

    return averageBPM;
}



void MainWindow::fillAubioInputBuffer( fvec_t* inputBuffer, const SharedSampleBuffer sampleBuffer, const int sampleOffset )
{
    const int numFrames = sampleBuffer->getNumFrames();
    const int numChans = sampleBuffer->getNumChannels();
    const float multiplier = 1.0 / numChans;
    const int hopSize = inputBuffer->length;

    FloatVectorOperations::clear( inputBuffer->data, hopSize );

    // Fill up the input buffer, converting stereo to mono if necessary
    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        const float* sampleData = sampleBuffer->getSampleData( chanNum, sampleOffset );
        const int numFramesToAdd = ( sampleOffset + hopSize <= numFrames ? hopSize : numFrames - sampleOffset - 1 );
        FloatVectorOperations::addWithMultiply( inputBuffer->data, sampleData, multiplier, numFramesToAdd );
    }
}



void MainWindow::createSampleSlices( const SharedSampleBuffer inputSampleBuffer,
                                     const QList<int> slicePointFrameNumList,
                                     QList<SharedSampleBuffer>& outputSampleBufferList )
{
    const int totalNumFrames = inputSampleBuffer->getNumFrames();
    const int numChans = inputSampleBuffer->getNumChannels();
    int prevSlicePointFrameNum = 0;

    foreach ( int slicePointFrameNum, slicePointFrameNumList )
    {
        const int numFrames = slicePointFrameNum - prevSlicePointFrameNum;
        SharedSampleBuffer sampleBuffer( new SampleBuffer( numChans, numFrames ) );

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *inputSampleBuffer.data(), chanNum, prevSlicePointFrameNum, numFrames );
        }

        outputSampleBufferList.append( sampleBuffer );
        prevSlicePointFrameNum = slicePointFrameNum;
    }

    const int numFrames = totalNumFrames - prevSlicePointFrameNum;
    SharedSampleBuffer sampleBuffer( new SampleBuffer( numChans, numFrames ) );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        sampleBuffer->copyFrom( chanNum, 0, *inputSampleBuffer.data(), chanNum, prevSlicePointFrameNum, numFrames );
    }

    outputSampleBufferList.append( sampleBuffer );
}



//==================================================================================================
// Private Slots:

void MainWindow::reorderSampleBufferList( const int oldOrderPos, const int newOrderPos )
{
    mSlicedSampleBuffers.move( oldOrderPos, newOrderPos );
    mSamplerAudioSource->setSamples( mSlicedSampleBuffers, mCurrentSampleHeader->sampleRate );
}



void MainWindow::recordWaveformItemNewOrderPos( const int startOrderPos, const int destOrderPos )
{
    QUndoCommand* command = new MoveWaveformItemCommand( startOrderPos, destOrderPos, mUI->waveGraphicsView );
    mUndoStack.push( command );
}



void MainWindow::recordSlicePointItemMove( const SharedSlicePointItem slicePointItem,
                                           const int oldFrameNum,
                                           const int newFrameNum )
{
    QUndoCommand* command = new MoveSlicePointItemCommand( slicePointItem, oldFrameNum, newFrameNum, mUI->waveGraphicsView );
    mUndoStack.push( command );
}



//====================
// "File" menu:

void MainWindow::on_actionOpen_Project_triggered()
{

}



void MainWindow::on_actionSave_Project_triggered()
{

}



void MainWindow::on_actionClose_Project_triggered()
{
    mSlicedSampleBuffers.clear();
    mCurrentSampleBuffer.clear();
    mCurrentSampleHeader.clear();

    if ( mIsAudioInitialised )
    {
        mAudioSourcePlayer.setSource( NULL );
        mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );
        mDeviceManager.removeMidiInputCallback( String::empty, mSamplerAudioSource->getMidiMessageCollector() );
        mSamplerAudioSource->clearAllSamples();
    }

    mUI->waveGraphicsView->clearAll();
    mUI->zoomSlider->setValue( 1 );
    mUI->doubleSpinBox_OriginalBPM->setValue( 0.0 );
    mUI->doubleSpinBox_NewBPM->setValue( 0.0 );
    mUI->pushButton_CalcBPM->setEnabled( false );
    mUI->pushButton_FindOnsets->setEnabled( false );
    mUI->pushButton_FindBeats->setEnabled( false );
    mUI->actionAdd_Slice_Point->setEnabled( false );

    mUndoStack.clear();
}



void MainWindow::on_actionImport_Audio_File_triggered()
{
    // Open audio file dialog
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Import Audio File"), mLastOpenedDir,
                                                           tr("All Files (*.*)") );

    if ( ! filePath.isEmpty() )
    {
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        const QDir dir( filePath );
        const QString fileName = dir.dirName();
        mLastOpenedDir = dir.absolutePath().remove( fileName );

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

            mUI->waveGraphicsView->createWaveform( sampleBuffer );

            if ( mIsAudioInitialised )
            {
                const bool deleteSourceWhenDeleted = false;
                const int bufferSize = 512;
                const int numChans = mCurrentSampleBuffer->getNumChannels();

                mSamplerAudioSource->addNewSample( sampleBuffer, sampleHeader->sampleRate );
                mSoundTouchAudioSource = new SoundTouchAudioSource( mSamplerAudioSource,
                                                                    deleteSourceWhenDeleted,
                                                                    bufferSize,
                                                                    numChans );
                mAudioSourcePlayer.setSource( mSoundTouchAudioSource );
                mDeviceManager.addAudioCallback( &mAudioSourcePlayer );
                mDeviceManager.addMidiInputCallback( String::empty, mSamplerAudioSource->getMidiMessageCollector() );
                mUI->pushButton_Play->setEnabled( true );
            }

            mUI->pushButton_CalcBPM->setEnabled( true );
            mUI->pushButton_FindOnsets->setEnabled( true );
            mUI->pushButton_FindBeats->setEnabled( true );
            mUI->actionAdd_Slice_Point->setEnabled( true );

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

    const DetectionSettings settings = getDetectionSettings();
    const qreal bpm = calcBPM( mCurrentSampleBuffer, settings );
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

    const DetectionSettings settings = getDetectionSettings();
    const QList<int> slicePointFrameNumList = calcSlicePointFrameNums( mCurrentSampleBuffer, ONSET_DETECTION, settings );

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

    const DetectionSettings settings = getDetectionSettings();
    const QList<int> slicePointFrameNumList = calcSlicePointFrameNums( mCurrentSampleBuffer, BEAT_DETECTION, settings );

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



void MainWindow::on_doubleSpinBox_NewBPM_valueChanged( const double newBPM )
{
    const double originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();
    const bool isPitchCorrectionEnabled = mUI->checkBox_PitchCorrection->isChecked();

    if ( isTimeStretchEnabled )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const float rate = 1.0f;
            const float tempo = newBPM / originalBPM;
            const float pitch = isPitchCorrectionEnabled ? 1.0f : newBPM / originalBPM;

            SoundTouchProcessor::PlaybackSettings settings( rate, tempo, pitch );
            mSoundTouchAudioSource->setPlaybackSettings( settings );
        }
    }
}



void MainWindow::on_checkBox_TimeStretch_toggled( const bool isChecked )
{
    const double originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const double newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = isChecked;
    const bool isPitchCorrectionEnabled = mUI->checkBox_PitchCorrection->isChecked();

    if ( newBPM > 0.0 && originalBPM > 0.0 )
    {
        const float rate = 1.0f;
        float tempo = 1.0f;
        float pitch = 1.0f;

        if ( isTimeStretchEnabled )
        {
            tempo = newBPM / originalBPM;
            pitch = isPitchCorrectionEnabled ? 1.0f : newBPM / originalBPM;
        }

        SoundTouchProcessor::PlaybackSettings settings( rate, tempo, pitch );
        mSoundTouchAudioSource->setPlaybackSettings( settings );
    }
}



void MainWindow::on_checkBox_PitchCorrection_toggled( const bool isChecked )
{
    const double originalBPM = mUI->doubleSpinBox_OriginalBPM->value();
    const double newBPM = mUI->doubleSpinBox_NewBPM->value();
    const bool isTimeStretchEnabled = mUI->checkBox_TimeStretch->isChecked();
    const bool isPitchCorrectionEnabled = isChecked;

    if ( isTimeStretchEnabled )
    {
        if ( newBPM > 0.0 && originalBPM > 0.0 )
        {
            const float rate = 1.0f;
            const float tempo = newBPM / originalBPM;
            const float pitch = isPitchCorrectionEnabled ? 1.0f : newBPM / originalBPM;

            SoundTouchProcessor::PlaybackSettings settings( rate, tempo, pitch );
            mSoundTouchAudioSource->setPlaybackSettings( settings );
        }
    }
}

void MainWindow::on_pushButton_Play_clicked()
{
    mSamplerAudioSource->play();
}

void MainWindow::on_pushButton_Stop_clicked()
{
    mSamplerAudioSource->stop();
}
