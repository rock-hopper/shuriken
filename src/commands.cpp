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

#include "commands.h"
#include <rubberband/RubberBandStretcher.h>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include "messageboxes.h"

using namespace RubberBand;


AddSlicePointItemCommand::AddSlicePointItemCommand( const int frameNum,
                                                    WaveGraphicsView* const graphicsView,
                                                    QPushButton* const sliceButton,
                                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_graphicsView( graphicsView ),
    m_sliceButton( sliceButton )
{
    setText( "Add Slice Point" );
    m_slicePointItem = m_graphicsView->createSlicePoint( frameNum );
    m_isFirstRedoCall = true;
}



void AddSlicePointItemCommand::undo()
{
    m_graphicsView->removeSlicePoint( m_slicePointItem );

    if ( m_graphicsView->getSlicePointFrameNums().isEmpty() )
    {
        m_sliceButton->setEnabled( false );
    }
}



void AddSlicePointItemCommand::redo()
{
    if ( ! m_isFirstRedoCall )
    {
        m_graphicsView->addSlicePoint( m_slicePointItem );
    }
    m_isFirstRedoCall = false;

    m_sliceButton->setEnabled( true );
}



//==================================================================================================

MoveSlicePointItemCommand::MoveSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                                      const int oldFrameNum,
                                                      const int newFrameNum,
                                                      WaveGraphicsView* const graphicsView,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_slicePointItem( slicePoint ),
    m_oldFrameNum( oldFrameNum ),
    m_newFrameNum( newFrameNum ),
    m_graphicsView( graphicsView )
{
    setText( "Move Slice Point" );
    m_isFirstRedoCall = true;
}



void MoveSlicePointItemCommand::undo()
{
    m_graphicsView->moveSlicePoint( m_slicePointItem, m_oldFrameNum );
}



void MoveSlicePointItemCommand::redo()
{
    if ( ! m_isFirstRedoCall )
    {
        m_graphicsView->moveSlicePoint( m_slicePointItem, m_newFrameNum );
    }
    m_isFirstRedoCall = false;
}



//==================================================================================================

DeleteSlicePointItemCommand::DeleteSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                                          WaveGraphicsView* const graphicsView,
                                                          QPushButton* const sliceButton,
                                                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_slicePointItem( slicePoint ),
    m_graphicsView( graphicsView ),
    m_sliceButton( sliceButton )
{
    setText( "Delete Slice Point" );
}



void DeleteSlicePointItemCommand::undo()
{
    m_graphicsView->addSlicePoint( m_slicePointItem );
    m_sliceButton->setEnabled( true );
}



void DeleteSlicePointItemCommand::redo()
{
    m_graphicsView->removeSlicePoint( m_slicePointItem );

    if ( m_graphicsView->getSlicePointFrameNums().isEmpty() )
    {
        m_sliceButton->setEnabled( false );
    }
}



//==================================================================================================

SliceCommand::SliceCommand( MainWindow* const mainWindow,
                            WaveGraphicsView* const graphicsView,
                            QPushButton* const sliceButton,
                            QPushButton* const findOnsetsButton,
                            QPushButton* const findBeatsButton,
                            QAction* const addSlicePointAction,
                            QAction* const moveItemsAction,
                            QAction* const selectItemsAction,
                            QAction* const auditionItemsAction,
                            QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsView( graphicsView ),
    m_sliceButton( sliceButton ),
    m_findOnsetsButton( findOnsetsButton ),
    m_findBeatsButton( findBeatsButton ),
    m_addSlicePointAction( addSlicePointAction ),
    m_moveItemsAction( moveItemsAction ),
    m_selectItemsAction( selectItemsAction ),
    m_auditionItemsAction( auditionItemsAction )
{
    setText( "Slice" );
}



void SliceCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    SharedSampleBuffer singleBuffer = SampleUtils::joinSampleBuffers( m_mainWindow->m_sampleBufferList );

    m_mainWindow->m_sampleBufferList.clear();
    m_mainWindow->m_sampleBufferList << singleBuffer;

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );

    m_graphicsView->clearWaveform();

    SharedWaveformItem item = m_graphicsView->createWaveform( m_mainWindow->m_sampleBufferList.first(),
                                                             m_mainWindow->m_sampleHeader );
    m_mainWindow->connectWaveformToMainWindow( item );

    m_graphicsView->showSlicePoints();

    m_sliceButton->setEnabled( true );
    m_findOnsetsButton->setEnabled( true );
    m_findBeatsButton->setEnabled( true );
    m_addSlicePointAction->setEnabled( true );
    m_selectItemsAction->setEnabled( false );
    m_auditionItemsAction->trigger();

    m_mainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



void SliceCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    m_mainWindow->m_sampleBufferList = SampleUtils::splitSampleBuffer( m_mainWindow->m_sampleBufferList.first(),
                                                                     m_graphicsView->getSlicePointFrameNums() );

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );

    m_graphicsView->hideSlicePoints();
    m_graphicsView->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList = m_graphicsView->createWaveforms( m_mainWindow->m_sampleBufferList,
                                                                                       m_mainWindow->m_sampleHeader );
    foreach ( SharedWaveformItem item, waveformItemList )
    {
        m_mainWindow->connectWaveformToMainWindow( item );
    }

    m_sliceButton->setEnabled( false );
    m_findOnsetsButton->setEnabled( false );
    m_findBeatsButton->setEnabled( false );
    m_addSlicePointAction->setEnabled( false );
    m_selectItemsAction->setEnabled( true );

    m_mainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



//==================================================================================================

MoveWaveformItemCommand::MoveWaveformItemCommand( const QList<int> oldOrderPositions,
                                                  const int numPlacesMoved,
                                                  WaveGraphicsView* const graphicsView,
                                                  MainWindow* const mainWindow,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_oldOrderPositions( oldOrderPositions ),
    m_numPlacesMoved( numPlacesMoved ),
    m_graphicsView( graphicsView ),
    m_mainWindow( mainWindow )
{
    setText( "Move Waveform Item" );
    m_isFirstRedoCall = true;

    foreach ( int orderPos, m_oldOrderPositions )
    {
        m_newOrderPositions << orderPos + m_numPlacesMoved;
    }
}



void MoveWaveformItemCommand::undo()
{
    m_mainWindow->reorderSampleBufferList( m_newOrderPositions, -m_numPlacesMoved );
    m_graphicsView->moveWaveforms( m_newOrderPositions, -m_numPlacesMoved );
}



void MoveWaveformItemCommand::redo()
{
    if ( ! m_isFirstRedoCall )
    {
        m_mainWindow->reorderSampleBufferList( m_oldOrderPositions, m_numPlacesMoved );
        m_graphicsView->moveWaveforms( m_oldOrderPositions, m_numPlacesMoved );
    }
    m_isFirstRedoCall = false;
}



//==================================================================================================

DeleteWaveformItemCommand::DeleteWaveformItemCommand( const QList<int> orderPositions,
                                                      WaveGraphicsView* const graphicsView,
                                                      MainWindow* const mainWindow,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_orderPositions( orderPositions ),
    m_graphicsView( graphicsView ),
    m_mainWindow( mainWindow )
{
    setText( "Delete Waveform Item" );
}



void DeleteWaveformItemCommand::undo()
{
    m_mainWindow->stopPlayback();

    m_graphicsView->insertWaveforms( m_removedWaveforms );

    const int firstOrderPos = m_orderPositions.first();

    for ( int i = 0; i < m_orderPositions.size(); i++ )
    {
        m_mainWindow->m_sampleBufferList.insert( firstOrderPos + i, m_removedSampleBuffers.at( i ) );
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );
}



void DeleteWaveformItemCommand::redo()
{
    m_mainWindow->stopPlayback();

    m_removedWaveforms = m_graphicsView->removeWaveforms( m_orderPositions );

    m_removedSampleBuffers.clear();

    const int firstOrderPos = m_orderPositions.first();

    for ( int i = 0; i < m_orderPositions.size(); i++ )
    {
        m_removedSampleBuffers << m_mainWindow->m_sampleBufferList.at( firstOrderPos );
        m_mainWindow->m_sampleBufferList.removeAt( firstOrderPos );
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );
}



//==================================================================================================

JoinCommand::JoinCommand( const QList<int> orderPositions,
                          WaveGraphicsView* const graphicsView,
                          MainWindow* const mainWindow,
                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_orderPositions( orderPositions ),
    m_graphicsView( graphicsView ),
    m_mainWindow( mainWindow )
{
    setText( "Join" );
}



void JoinCommand::undo()
{
    m_mainWindow->stopPlayback();
    m_graphicsView->selectNone();

    QList<SharedWaveformItem> items = m_graphicsView->splitWaveform( m_joinedItemOrderPos, m_slicePoints );

    m_mainWindow->m_sampleBufferList.removeAt( m_joinedItemOrderPos );

    foreach ( SharedWaveformItem item, items )
    {
        m_mainWindow->connectWaveformToMainWindow( item );
        m_mainWindow->m_sampleBufferList.insert( item->getOrderPos(), item->getSampleBuffer() );
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );
}



void JoinCommand::redo()
{
    m_mainWindow->stopPlayback();
    m_graphicsView->selectNone();

    SharedWaveformItem item = m_graphicsView->joinWaveforms( m_orderPositions );
    m_mainWindow->connectWaveformToMainWindow( item );

    m_slicePoints.clear();
    int frameNum = 0;

    for ( int i = 0; i < m_orderPositions.size() - 1; i++ )
    {
        const int orderPos = m_orderPositions.at( i );

        frameNum += m_mainWindow->m_sampleBufferList.at( orderPos )->getNumFrames();

        m_slicePoints << frameNum;
    }

    m_joinedItemOrderPos = item->getOrderPos();

    for ( int i = 0; i < m_orderPositions.size(); i++ )
    {
        m_mainWindow->m_sampleBufferList.removeAt( m_joinedItemOrderPos );
    }

    m_mainWindow->m_sampleBufferList.insert( m_joinedItemOrderPos, item->getSampleBuffer() );

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );
}



//==================================================================================================

//SplitCommand::SplitCommand( const int orderPos,
//                           WaveGraphicsView* const graphicsView,
//                           MainWindow* const mainWindow,
//                           QUndoCommand* parent ) :
//    QUndoCommand( parent ),
//    mJoinedItemOrderPos( orderPos ),
//    mGraphicsView( graphicsView ),
//    mMainWindow( mainWindow )
//{
//    setText( "Split" );
//}
//
//
//
//void SplitCommand::undo()
//{
//    mGraphicsView->selectNone();
//
//    SharedWaveformItem item = mGraphicsView->joinWaveforms( mOrderPositions );
//    mMainWindow->connectWaveformToMainWindow( item );
//
//    foreach ( int orderPos, mOrderPositions )
//    {
//        mMainWindow->mSampleRangeList.removeAt( mJoinedItemOrderPos );
//    }
//
//    mMainWindow->mSampleRangeList.insert( mJoinedItemOrderPos, item->getSampleRange() );
//
//    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );
//}
//
//
//
//void SplitCommand::redo()
//{
//    mGraphicsView->selectNone();
//
//    QList<SharedWaveformItem> items = mGraphicsView->splitWaveform( mJoinedItemOrderPos );
//
//    mMainWindow->mSampleRangeList.removeAt( mJoinedItemOrderPos );
//
//    mOrderPositions.clear();
//
//    foreach ( SharedWaveformItem item, items )
//    {
//        mOrderPositions << item->getOrderPos();
//        mMainWindow->mSampleRangeList.insert( item->getOrderPos(), item->getSampleRange() );
//    }
//
//    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );
//}



//==================================================================================================

ApplyGainCommand::ApplyGainCommand( const float gain,
                                    const int waveformItemOrderPos,
                                    WaveGraphicsView* const graphicsView,
                                    const int sampleRate,
                                    AudioFileHandler& fileHandler,
                                    const QString tempDirPath,
                                    const QString fileBaseName,
                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_gain( gain ),
    m_orderPos( waveformItemOrderPos ),
    m_graphicsView( graphicsView ),
    m_sampleRate( sampleRate ),
    m_fileHandler( fileHandler ),
    m_tempDirPath( tempDirPath ),
    m_fileBaseName( fileBaseName )
{
    setText( "Apply Gain" );
}



void ApplyGainCommand::undo()
{
    if ( ! m_filePath.isEmpty() )
    {
        const SharedWaveformItem waveformItem = m_graphicsView->getWaveformAt( m_orderPos );

        const SharedSampleBuffer origSampleBuffer = m_fileHandler.getSampleData( m_filePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int numFrames = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = waveformItem->getSampleBuffer();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, numFrames );
        }

        m_graphicsView->redrawWaveforms();
    }
}



void ApplyGainCommand::redo()
{
    const SharedWaveformItem item = m_graphicsView->getWaveformAt( m_orderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    m_filePath = m_fileHandler.saveAudioFile( m_tempDirPath,
                                            m_fileBaseName,
                                            sampleBuffer,
                                            m_sampleRate,
                                            m_sampleRate,
                                            AudioFileHandler::TEMP_FORMAT );

    if ( ! m_filePath.isEmpty() )
    {
        sampleBuffer->applyGain( 0, sampleBuffer->getNumFrames(), m_gain );
        m_graphicsView->redrawWaveforms();
    }
    else
    {
        MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
    }
}



//==================================================================================================

ApplyGainRampCommand::ApplyGainRampCommand( const float startGain,
                                            const float endGain,
                                            const int waveformItemOrderPos,
                                            WaveGraphicsView* const graphicsView,
                                            const int sampleRate,
                                            AudioFileHandler& fileHandler,
                                            const QString tempDirPath,
                                            const QString fileBaseName,
                                            QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_startGain( startGain ),
    m_endGain( endGain ),
    m_orderPos( waveformItemOrderPos ),
    m_graphicsView( graphicsView ),
    m_sampleRate( sampleRate ),
    m_fileHandler( fileHandler ),
    m_tempDirPath( tempDirPath ),
    m_fileBaseName( fileBaseName )
{
    setText( "Apply Gain Ramp" );
}



void ApplyGainRampCommand::undo()
{
    if ( ! m_filePath.isEmpty() )
    {
        const SharedWaveformItem waveformItem = m_graphicsView->getWaveformAt( m_orderPos );

        const SharedSampleBuffer origSampleBuffer = m_fileHandler.getSampleData( m_filePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int numFrames = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = waveformItem->getSampleBuffer();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, numFrames );
        }

        m_graphicsView->redrawWaveforms();
    }
}



void ApplyGainRampCommand::redo()
{
    const SharedWaveformItem item = m_graphicsView->getWaveformAt( m_orderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    m_filePath = m_fileHandler.saveAudioFile( m_tempDirPath,
                                            m_fileBaseName,
                                            sampleBuffer,
                                            m_sampleRate,
                                            m_sampleRate,
                                            AudioFileHandler::TEMP_FORMAT );

    if ( ! m_filePath.isEmpty() )
    {
        sampleBuffer->applyGainRamp( 0, sampleBuffer->getNumFrames(), m_startGain, m_endGain );
        m_graphicsView->redrawWaveforms();
    }
    else
    {
        MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
    }
}



//==================================================================================================

NormaliseCommand::NormaliseCommand( const int waveformItemOrderPos,
                                    WaveGraphicsView* const graphicsView,
                                    const int sampleRate,
                                    AudioFileHandler& fileHandler,
                                    const QString tempDirPath,
                                    const QString fileBaseName,
                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_orderPos( waveformItemOrderPos ),
    m_graphicsView( graphicsView ),
    m_sampleRate( sampleRate ),
    m_fileHandler( fileHandler ),
    m_tempDirPath( tempDirPath ),
    m_fileBaseName( fileBaseName )
{
    setText( "Normalise" );
}



void NormaliseCommand::undo()
{
    if ( ! m_filePath.isEmpty() )
    {
        const SharedWaveformItem waveformItem = m_graphicsView->getWaveformAt( m_orderPos );

        const SharedSampleBuffer origSampleBuffer = m_fileHandler.getSampleData( m_filePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int numFrames = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = waveformItem->getSampleBuffer();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, numFrames );
        }

        m_graphicsView->redrawWaveforms();
    }
}



void NormaliseCommand::redo()
{
    const SharedWaveformItem item = m_graphicsView->getWaveformAt( m_orderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    m_filePath = m_fileHandler.saveAudioFile( m_tempDirPath,
                                            m_fileBaseName,
                                            sampleBuffer,
                                            m_sampleRate,
                                            m_sampleRate,
                                            AudioFileHandler::TEMP_FORMAT );

    if ( ! m_filePath.isEmpty() )
    {
        const int numFrames = sampleBuffer->getNumFrames();
        const float magnitude = sampleBuffer->getMagnitude( 0, numFrames );

        if ( magnitude > 0.0 )
        {
            sampleBuffer->applyGain( 0, numFrames, 1.0 / magnitude );
            m_graphicsView->redrawWaveforms();
        }
    }
    else
    {
        MessageBoxes::showWarningDialog( m_fileHandler.getLastErrorTitle(), m_fileHandler.getLastErrorInfo() );
    }
}



//==================================================================================================

ReverseCommand::ReverseCommand( const int waveformItemOrderPos,
                                WaveGraphicsView* const graphicsView,
                                QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOrderPos( waveformItemOrderPos ),
    m_graphicsView( graphicsView )
{
    setText( "Reverse" );
}



void ReverseCommand::undo()
{
    redo();
}



void ReverseCommand::redo()
{
    const SharedWaveformItem item = m_graphicsView->getWaveformAt( mOrderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    sampleBuffer->reverse( 0, sampleBuffer->getNumFrames() );

    m_graphicsView->redrawWaveforms();
}



//==================================================================================================

ApplyTimeStretchCommand::ApplyTimeStretchCommand( MainWindow* const mainWindow,
                                                  WaveGraphicsView* const graphicsView,
                                                  QDoubleSpinBox* const spinBoxOriginalBPM,
                                                  QDoubleSpinBox* const spinBoxNewBPM,
                                                  QCheckBox* const checkBoxPitchCorrection,
                                                  const QString tempDirPath,
                                                  const QString fileBaseName,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsView( graphicsView ),
    m_spinBoxOriginalBPM( spinBoxOriginalBPM ),
    m_spinBoxNewBPM( spinBoxNewBPM ),
    m_checkBoxPitchCorrection( checkBoxPitchCorrection ),
    m_originalBPM( m_spinBoxOriginalBPM->value() ),
    m_newBPM( m_spinBoxNewBPM->value() ),
    m_prevAppliedBPM( m_mainWindow->m_appliedBPM ),
    m_isPitchCorrectionEnabled( m_checkBoxPitchCorrection->isChecked() ),
    m_options( m_mainWindow->m_optionsDialog->getStretcherOptions() ),
    m_tempDirPath( tempDirPath ),
    m_fileBaseName( fileBaseName )
{
    setText( "Apply Timestretch" );
}



void ApplyTimeStretchCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    for ( int i = 0; i < m_tempFilePaths.size(); i++ )
    {
        const QString filePath = m_tempFilePaths.at( i );
        const SharedSampleBuffer origSampleBuffer = m_mainWindow->m_fileHandler.getSampleData( filePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int origBufferSize = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = m_mainWindow->m_sampleBufferList.at( i );
        sampleBuffer->setSize( numChans, origBufferSize );

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, origBufferSize );
        }
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                  m_mainWindow->m_sampleHeader->sampleRate );

    const qreal timeRatio = 1.0 / ( m_originalBPM / m_newBPM );

    updateLoopMarkers( timeRatio );
    updateSlicePoints( timeRatio );
    m_graphicsView->redrawWaveforms();

    m_spinBoxOriginalBPM->setValue( m_originalBPM );
    m_spinBoxNewBPM->setValue( m_originalBPM );
    m_checkBoxPitchCorrection->setChecked( m_isPitchCorrectionEnabled );

    m_mainWindow->m_appliedBPM = m_prevAppliedBPM;

    QApplication::restoreOverrideCursor();
}



void ApplyTimeStretchCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    m_tempFilePaths.clear();

    for ( int i = 0; i < m_mainWindow->m_sampleBufferList.size(); i++ )
    {
        const QString path = m_mainWindow->m_fileHandler.saveAudioFile( m_tempDirPath,
                                                                      m_fileBaseName + "_" + QString::number( i ),
                                                                      m_mainWindow->m_sampleBufferList.at( i ),
                                                                      m_mainWindow->m_sampleHeader->sampleRate,
                                                                      m_mainWindow->m_sampleHeader->sampleRate,
                                                                      AudioFileHandler::TEMP_FORMAT );
        if ( ! path.isEmpty() )
        {
            m_tempFilePaths << path;
        }
        else
        {
            m_tempFilePaths.clear();
            break;
        }
    }

    if ( ! m_tempFilePaths.isEmpty() )
    {
        const qreal timeRatio = m_originalBPM / m_newBPM;
        const qreal pitchScale = m_isPitchCorrectionEnabled ? 1.0 : m_newBPM / m_originalBPM;

        int newTotalNumFrames = 0;

        foreach ( SharedSampleBuffer sampleBuffer, m_mainWindow->m_sampleBufferList )
        {
            newTotalNumFrames += stretch( sampleBuffer, timeRatio, pitchScale );
        }

        m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                      m_mainWindow->m_sampleHeader->sampleRate );

        updateLoopMarkers( timeRatio );
        updateSlicePoints( timeRatio );
        m_graphicsView->redrawWaveforms();

        m_spinBoxOriginalBPM->setValue( m_newBPM );
        m_spinBoxNewBPM->setValue( m_newBPM );
        m_checkBoxPitchCorrection->setChecked( m_isPitchCorrectionEnabled );

        m_mainWindow->m_appliedBPM = m_newBPM;

        QApplication::restoreOverrideCursor();
    }
    else
    {
        QApplication::restoreOverrideCursor();

        MessageBoxes::showWarningDialog( m_mainWindow->m_fileHandler.getLastErrorTitle(),
                                       m_mainWindow->m_fileHandler.getLastErrorInfo() );
    }
}



int ApplyTimeStretchCommand::stretch( const SharedSampleBuffer sampleBuffer, const qreal timeRatio, const qreal pitchScale )
{
    const int sampleRate = m_mainWindow->m_sampleHeader->sampleRate;
    const int numChans = m_mainWindow->m_sampleHeader->numChans;

    RubberBandStretcher stretcher( sampleRate, numChans, m_options, timeRatio, pitchScale );

    // Copy sample buffer to a temporary buffer
    SharedSampleBuffer tempBuffer( new SampleBuffer( *sampleBuffer.data() ) );

    const int origNumFrames = tempBuffer->getNumFrames();
    const int newBufferSize = roundToInt( origNumFrames * timeRatio );
    const float** inFloatBuffer = new const float*[ numChans ];
    float** outFloatBuffer = new float*[ numChans ];
    int inFrameNum = 0;
    int totalNumFramesRetrieved = 0;
//            std::map<size_t, size_t> mapping;
//
//            if ( ! mMainWindow->mSampleBufferList.isEmpty() )
//            {
//                foreach ( SharedSampleBuffer sampleBuffer, mMainWindow->mSampleBufferList )
//                {
//                    const int startFrame = sampleBuffer->getStartFrame()
//                    mapping[ startFrame ] = roundToInt( startFrame * timeRatio );
//                }
//            }

    stretcher.setExpectedInputDuration( origNumFrames );

    sampleBuffer->setSize( numChans, newBufferSize );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        inFloatBuffer[ chanNum ] = tempBuffer->getReadPointer( chanNum );
        outFloatBuffer[ chanNum ] = sampleBuffer->getWritePointer( chanNum );
    }

    stretcher.study( inFloatBuffer, origNumFrames, true );

//            if ( ! mapping.empty() )
//            {
//                stretcher.setKeyFrameMap( mapping );
//            }

    while ( inFrameNum < origNumFrames )
    {
        const int numRequired = stretcher.getSamplesRequired();

        const int numFramesToProcess = inFrameNum + numRequired <= origNumFrames ?
                                       numRequired : origNumFrames - inFrameNum;
        const bool isFinal = (inFrameNum + numRequired >= origNumFrames);

        stretcher.process( inFloatBuffer, numFramesToProcess, isFinal );

        const int numAvailable = stretcher.available();

        if ( numAvailable > 0 )
        {
            // Ensure enough space to store output
            if ( sampleBuffer->getNumFrames() < totalNumFramesRetrieved + numAvailable )
            {
                sampleBuffer->setSize( numChans,                                  // No. of channels
                                       totalNumFramesRetrieved + numAvailable,    // New no. of frames
                                       true );                                    // Keep existing content

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    outFloatBuffer[ chanNum ] = sampleBuffer->getWritePointer( chanNum );
                    outFloatBuffer[ chanNum ] += totalNumFramesRetrieved;
                }
            }

            const int numRetrieved = stretcher.retrieve( outFloatBuffer, numAvailable );

            for ( int chanNum = 0; chanNum < numChans; chanNum++ )
            {
                outFloatBuffer[ chanNum ] += numRetrieved;
            }
            totalNumFramesRetrieved += numRetrieved;
        }

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            inFloatBuffer[ chanNum ] += numFramesToProcess;
        }
        inFrameNum += numFramesToProcess;
    }

    int numAvailable;

    while ( (numAvailable = stretcher.available()) >= 0 )
    {
        if ( numAvailable > 0 )
        {
            // Ensure enough space to store output
            if ( sampleBuffer->getNumFrames() < totalNumFramesRetrieved + numAvailable )
            {
                sampleBuffer->setSize( numChans,                                  // No. of channels
                                       totalNumFramesRetrieved + numAvailable,    // New no. of frames
                                       true );                                    // Keep existing content

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    outFloatBuffer[ chanNum ] = sampleBuffer->getWritePointer( chanNum );
                    outFloatBuffer[ chanNum ] += totalNumFramesRetrieved;
                }
            }

            const int numRetrieved = stretcher.retrieve( outFloatBuffer, numAvailable );

            for ( int chanNum = 0; chanNum < numChans; chanNum++ )
            {
                outFloatBuffer[ chanNum ] += numRetrieved;
            }
            totalNumFramesRetrieved += numRetrieved;
        }
        else
        {
            usleep( 10000 );
        }
    }

    if ( sampleBuffer->getNumFrames() != totalNumFramesRetrieved )
    {
        sampleBuffer->setSize( numChans, totalNumFramesRetrieved, true );
    }

    delete[] inFloatBuffer;
    delete[] outFloatBuffer;

    return totalNumFramesRetrieved;
}



void ApplyTimeStretchCommand::updateSlicePoints( const qreal timeRatio )
{
    QList<SharedSlicePointItem> slicePointList = m_graphicsView->getSlicePointList();

    foreach ( SharedSlicePointItem slicePoint, slicePointList )
    {
        const int newFrameNum = roundToInt( slicePoint->getFrameNum() * timeRatio );
        slicePoint->setFrameNum( newFrameNum );
    }
}



void ApplyTimeStretchCommand::updateLoopMarkers( const qreal timeRatio )
{
    LoopMarkerItem* loopMarkerLeft = m_graphicsView->getLeftLoopMarker();

    if ( loopMarkerLeft != NULL )
    {
        const int newFrameNum = roundToInt( loopMarkerLeft->getFrameNum() * timeRatio );
        loopMarkerLeft->setFrameNum( newFrameNum );
    }

    LoopMarkerItem* loopMarkerRight = m_graphicsView->getRightLoopMarker();

    if ( loopMarkerRight != NULL )
    {
        const int newFrameNum = roundToInt( loopMarkerRight->getFrameNum() * timeRatio );
        loopMarkerRight->setFrameNum( newFrameNum );
    }
}
