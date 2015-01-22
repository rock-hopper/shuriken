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
#include <QApplication>
#include <QDir>
#include <QDebug>
#include "messageboxes.h"
#include "offlinetimestretcher.h"


AddSlicePointItemCommand::AddSlicePointItemCommand( const int frameNum,
                                                    const bool canBeMovedPastOtherSlicePoints,
                                                    WaveGraphicsScene* const graphicsScene,
                                                    QPushButton* const sliceButton,
                                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_graphicsScene( graphicsScene ),
    m_sliceButton( sliceButton )
{
    setText( "Add Slice Point" );
    m_slicePointItem = m_graphicsScene->createSlicePoint( frameNum, canBeMovedPastOtherSlicePoints );
    m_isFirstRedoCall = true;
}



void AddSlicePointItemCommand::undo()
{
    m_graphicsScene->removeSlicePoint( m_slicePointItem );

    if ( m_graphicsScene->getSlicePointFrameNums().isEmpty() )
    {
        if ( m_sliceButton != NULL )
            m_sliceButton->setEnabled( false );
    }
}



void AddSlicePointItemCommand::redo()
{
    if ( ! m_isFirstRedoCall )
    {
        m_graphicsScene->addSlicePoint( m_slicePointItem );
    }
    m_isFirstRedoCall = false;

    if ( m_sliceButton != NULL )
    {
        m_sliceButton->setEnabled( true );
    }
}



//==================================================================================================

MoveSlicePointItemCommand::MoveSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                                      const int oldFrameNum,
                                                      WaveGraphicsScene* const graphicsScene,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_slicePointItem( slicePoint ),
    m_oldFrameNum( oldFrameNum ),
    m_newFrameNum( slicePoint->getFrameNum() ),
    m_graphicsScene( graphicsScene )
{
    setText( "Move Slice Point" );
    m_isFirstRedoCall = true;
}



void MoveSlicePointItemCommand::undo()
{
    m_graphicsScene->moveSlicePoint( m_slicePointItem, m_oldFrameNum );
}



void MoveSlicePointItemCommand::redo()
{
    if ( ! m_isFirstRedoCall )
    {
        m_graphicsScene->moveSlicePoint( m_slicePointItem, m_newFrameNum );
    }
    m_isFirstRedoCall = false;
}



//==================================================================================================

DeleteSlicePointItemCommand::DeleteSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                                          WaveGraphicsScene* const graphicsScene,
                                                          QPushButton* const sliceButton,
                                                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_slicePointItem( slicePoint ),
    m_graphicsScene( graphicsScene ),
    m_sliceButton( sliceButton )
{
    setText( "Delete Slice Point" );
}



void DeleteSlicePointItemCommand::undo()
{
    m_graphicsScene->addSlicePoint( m_slicePointItem );

    if ( m_sliceButton != NULL )
    {
        m_sliceButton->setEnabled( true );
    }
}



void DeleteSlicePointItemCommand::redo()
{
    m_graphicsScene->removeSlicePoint( m_slicePointItem );

    if ( m_graphicsScene->getSlicePointFrameNums().isEmpty() )
    {
        if ( m_sliceButton != NULL )
            m_sliceButton->setEnabled( false );
    }
}



//==================================================================================================

SliceCommand::SliceCommand( MainWindow* const mainWindow,
                            WaveGraphicsScene* const graphicsScene,
                            QPushButton* const sliceButton,
                            QPushButton* const findOnsetsButton,
                            QPushButton* const findBeatsButton,
                            QAction* const addSlicePointAction,
                            QAction* const selectMoveItemsAction,
                            QAction* const auditionItemsAction,
                            QAction* const selectiveTimeStretchAction,
                            QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsScene( graphicsScene ),
    m_sliceButton( sliceButton ),
    m_findOnsetsButton( findOnsetsButton ),
    m_findBeatsButton( findBeatsButton ),
    m_addSlicePointAction( addSlicePointAction ),
    m_selectMoveItemsAction( selectMoveItemsAction ),
    m_auditionItemsAction( auditionItemsAction ),
    m_selectiveTimeStretchAction( selectiveTimeStretchAction )
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

    m_graphicsScene->clearWaveform();

    SharedWaveformItem item = m_graphicsScene->createWaveform( m_mainWindow->m_sampleBufferList.first(),
                                                              m_mainWindow->m_sampleHeader );
    m_mainWindow->connectWaveformToMainWindow( item );

    m_sliceButton->setChecked( false );
    m_findOnsetsButton->setEnabled( true );
    m_findBeatsButton->setEnabled( true );
    m_addSlicePointAction->setEnabled( true );
    m_selectiveTimeStretchAction->setEnabled( false );
    m_auditionItemsAction->trigger();

    m_mainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



void SliceCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    m_mainWindow->m_sampleBufferList = SampleUtils::splitSampleBuffer( m_mainWindow->m_sampleBufferList.first(),
                                                                       m_graphicsScene->getSlicePointFrameNums() );

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                    m_mainWindow->m_sampleHeader->sampleRate );

    m_graphicsScene->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList = m_graphicsScene->createWaveforms( m_mainWindow->m_sampleBufferList,
                                                                                        m_mainWindow->m_sampleHeader );
    foreach ( SharedWaveformItem item, waveformItemList )
    {
        m_mainWindow->connectWaveformToMainWindow( item );
    }

    m_sliceButton->setChecked( true );
    m_findOnsetsButton->setEnabled( false );
    m_findBeatsButton->setEnabled( false );
    m_addSlicePointAction->setEnabled( false );

    if ( m_mainWindow->m_rubberbandAudioSource != NULL )
    {
        m_selectiveTimeStretchAction->setEnabled( true );
    }

    m_selectMoveItemsAction->trigger();

    m_mainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



//==================================================================================================

UnsliceCommand::UnsliceCommand( MainWindow* const mainWindow,
                                WaveGraphicsScene* const graphicsScene,
                                QPushButton* const sliceButton,
                                QPushButton* const findOnsetsButton,
                                QPushButton* const findBeatsButton,
                                QAction* const addSlicePointAction,
                                QAction* const selectMoveItemsAction,
                                QAction* const auditionItemsAction,
                                QAction* const selectiveTimeStretchAction,
                                QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsScene( graphicsScene ),
    m_sliceButton( sliceButton ),
    m_findOnsetsButton( findOnsetsButton ),
    m_findBeatsButton( findBeatsButton ),
    m_addSlicePointAction( addSlicePointAction ),
    m_selectMoveItemsAction( selectMoveItemsAction ),
    m_auditionItemsAction( auditionItemsAction ),
    m_selectiveTimeStretchAction( selectiveTimeStretchAction )
{
    setText( "Unslice" );
}



void UnsliceCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    m_mainWindow->m_sampleBufferList = SampleUtils::splitSampleBuffer( m_mainWindow->m_sampleBufferList.first(),
                                                                       m_graphicsScene->getSlicePointFrameNums() );

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                    m_mainWindow->m_sampleHeader->sampleRate );

    m_graphicsScene->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList = m_graphicsScene->createWaveforms( m_mainWindow->m_sampleBufferList,
                                                                                        m_mainWindow->m_sampleHeader );
    foreach ( SharedWaveformItem item, waveformItemList )
    {
        m_mainWindow->connectWaveformToMainWindow( item );
    }

    m_sliceButton->setChecked( true );
    m_findOnsetsButton->setEnabled( false );
    m_findBeatsButton->setEnabled( false );
    m_addSlicePointAction->setEnabled( false );
    m_selectiveTimeStretchAction->setEnabled( true );
    m_selectMoveItemsAction->trigger();

    m_mainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



void UnsliceCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    SharedSampleBuffer singleBuffer = SampleUtils::joinSampleBuffers( m_mainWindow->m_sampleBufferList );

    m_mainWindow->m_sampleBufferList.clear();
    m_mainWindow->m_sampleBufferList << singleBuffer;

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                    m_mainWindow->m_sampleHeader->sampleRate );

    m_graphicsScene->clearWaveform();

    SharedWaveformItem item = m_graphicsScene->createWaveform( m_mainWindow->m_sampleBufferList.first(),
                                                              m_mainWindow->m_sampleHeader );
    m_mainWindow->connectWaveformToMainWindow( item );

    m_sliceButton->setChecked( false );
    m_findOnsetsButton->setEnabled( true );
    m_findBeatsButton->setEnabled( true );
    m_addSlicePointAction->setEnabled( true );
    m_selectiveTimeStretchAction->setEnabled( false );
    m_auditionItemsAction->trigger();

    m_mainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



//==================================================================================================

EnableSelectiveTSCommand::EnableSelectiveTSCommand( OptionsDialog* const optionsDialog,
                                                    WaveGraphicsScene* const graphicsScene,
                                                    QPushButton* const sliceButton,
                                                    QAction* const addSlicePointAction,
                                                    QAction* const selectMoveItemsAction,
                                                    QAction* const multiSelectItemsAction,
                                                    QAction* const auditionItemsAction,
                                                    QAction* const selectiveTimeStretchAction,
                                                    const QList<SharedSampleBuffer> sampleBufferList,
                                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_optionsDialog( optionsDialog ),
    m_graphicsScene( graphicsScene ),
    m_sliceButton( sliceButton ),
    m_addSlicePointAction( addSlicePointAction ),
    m_selectMoveItemsAction( selectMoveItemsAction ),
    m_multiSelectItemsAction( multiSelectItemsAction ),
    m_auditionItemsAction( auditionItemsAction ),
    m_selectiveTimeStretchAction( selectiveTimeStretchAction ),
    m_sampleBufferList( sampleBufferList )
{
    setText( "Enable Selective Time Stretching" );
}



void EnableSelectiveTSCommand::undo()
{
    m_selectMoveItemsAction->setEnabled( true );
    m_multiSelectItemsAction->setEnabled( true );
    m_auditionItemsAction->setEnabled( true );
    m_addSlicePointAction->setEnabled( true );

    if ( m_graphicsScene->getSlicePointList().size() > 0 || m_sampleBufferList.size() > 1 )
    {
        m_sliceButton->setEnabled( true );
    }

    m_selectiveTimeStretchAction->setChecked( false );
    m_optionsDialog->enableOfflineRealtimeButtons();
}



void EnableSelectiveTSCommand::redo()
{
    m_optionsDialog->disableOfflineRealtimeButtons();
    m_auditionItemsAction->trigger();
    m_selectMoveItemsAction->setEnabled( false );
    m_multiSelectItemsAction->setEnabled( false );
    m_auditionItemsAction->setEnabled( false );
    m_addSlicePointAction->setEnabled( false );
    m_sliceButton->setEnabled( false );
    m_selectiveTimeStretchAction->setChecked( true );
}



//==================================================================================================

DisableSelectiveTSCommand::DisableSelectiveTSCommand( OptionsDialog* const optionsDialog,
                                                      WaveGraphicsScene* const graphicsScene,
                                                      QPushButton* const sliceButton,
                                                      QAction* const addSlicePointAction,
                                                      QAction* const selectMoveItemsAction,
                                                      QAction* const multiSelectItemsAction,
                                                      QAction* const auditionItemsAction,
                                                      QAction* const selectiveTimeStretchAction,
                                                      const QList<SharedSampleBuffer> sampleBufferList,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_optionsDialog( optionsDialog ),
    m_graphicsScene( graphicsScene ),
    m_sliceButton( sliceButton ),
    m_addSlicePointAction( addSlicePointAction ),
    m_selectMoveItemsAction( selectMoveItemsAction ),
    m_multiSelectItemsAction( multiSelectItemsAction ),
    m_auditionItemsAction( auditionItemsAction ),
    m_selectiveTimeStretchAction( selectiveTimeStretchAction ),
    m_sampleBufferList( sampleBufferList )
{
    setText( "Disable Selective Time Stretching" );
}



void DisableSelectiveTSCommand::undo()
{
    m_optionsDialog->disableOfflineRealtimeButtons();
    m_auditionItemsAction->trigger();
    m_selectMoveItemsAction->setEnabled( false );
    m_multiSelectItemsAction->setEnabled( false );
    m_auditionItemsAction->setEnabled( false );
    m_addSlicePointAction->setEnabled( false );
    m_sliceButton->setEnabled( false );
    m_selectiveTimeStretchAction->setChecked( true );
}



void DisableSelectiveTSCommand::redo()
{
    m_selectMoveItemsAction->setEnabled( true );
    m_multiSelectItemsAction->setEnabled( true );
    m_auditionItemsAction->setEnabled( true );
    m_addSlicePointAction->setEnabled( true );

    if ( m_graphicsScene->getSlicePointList().size() > 0 || m_sampleBufferList.size() > 1 )
    {
        m_sliceButton->setEnabled( true );
    }

    m_selectiveTimeStretchAction->setChecked( false );
    m_optionsDialog->enableOfflineRealtimeButtons();
}



//==================================================================================================

MoveWaveformItemCommand::MoveWaveformItemCommand( const QList<int> oldOrderPositions,
                                                  const int numPlacesMoved,
                                                  WaveGraphicsScene* const graphicsScene,
                                                  MainWindow* const mainWindow,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_oldOrderPositions( oldOrderPositions ),
    m_numPlacesMoved( numPlacesMoved ),
    m_graphicsScene( graphicsScene ),
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
    m_graphicsScene->moveWaveforms( m_newOrderPositions, -m_numPlacesMoved );
}



void MoveWaveformItemCommand::redo()
{
    if ( ! m_isFirstRedoCall )
    {
        m_mainWindow->reorderSampleBufferList( m_oldOrderPositions, m_numPlacesMoved );
        m_graphicsScene->moveWaveforms( m_oldOrderPositions, m_numPlacesMoved );
    }
    m_isFirstRedoCall = false;
}



//==================================================================================================

DeleteWaveformItemCommand::DeleteWaveformItemCommand( const QList<int> orderPositions,
                                                      WaveGraphicsScene* const graphicsScene,
                                                      MainWindow* const mainWindow,
                                                      QPushButton* const sliceButton,
                                                      QPushButton* const findOnsetsButton,
                                                      QPushButton* const findBeatsButton,
                                                      QAction* const addSlicePointAction,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_orderPositions( orderPositions ),
    m_graphicsScene( graphicsScene ),
    m_mainWindow( mainWindow ),
    m_sliceButton( sliceButton ),
    m_findOnsetsButton( findOnsetsButton ),
    m_findBeatsButton( findBeatsButton ),
    m_addSlicePointAction( addSlicePointAction )
{
    setText( "Delete Waveform Item" );
}



void DeleteWaveformItemCommand::undo()
{
    m_mainWindow->stopPlayback();

    m_graphicsScene->insertWaveforms( m_removedWaveforms );

    const int firstOrderPos = m_orderPositions.first();

    for ( int i = 0; i < m_orderPositions.size(); i++ )
    {
        m_mainWindow->m_sampleBufferList.insert( firstOrderPos + i, m_removedSampleBuffers.at( i ) );
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                    m_mainWindow->m_sampleHeader->sampleRate );

    m_sliceButton->setEnabled( true );
    m_sliceButton->setChecked( true );
    m_findOnsetsButton->setEnabled( false );
    m_findBeatsButton->setEnabled( false );
    m_addSlicePointAction->setEnabled( false );
}



void DeleteWaveformItemCommand::redo()
{
    m_mainWindow->stopPlayback();

    m_removedWaveforms = m_graphicsScene->removeWaveforms( m_orderPositions );

    m_removedSampleBuffers.clear();

    const int firstOrderPos = m_orderPositions.first();

    for ( int i = 0; i < m_orderPositions.size(); i++ )
    {
        m_removedSampleBuffers << m_mainWindow->m_sampleBufferList.at( firstOrderPos );
        m_mainWindow->m_sampleBufferList.removeAt( firstOrderPos );
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                    m_mainWindow->m_sampleHeader->sampleRate );

    if ( m_mainWindow->m_sampleBufferList.size() == 1 )
    {
        m_sliceButton->setEnabled( false );
        m_sliceButton->setChecked( false );
        m_findOnsetsButton->setEnabled( true );
        m_findBeatsButton->setEnabled( true );
        m_addSlicePointAction->setEnabled( true );
    }
}



//==================================================================================================

ApplyGainCommand::ApplyGainCommand( const float gain,
                                    const int waveformItemOrderPos,
                                    WaveGraphicsScene* const graphicsScene,
                                    WaveGraphicsView* const graphicsView,
                                    const int sampleRate,
                                    AudioFileHandler& fileHandler,
                                    const QString tempDirPath,
                                    const QString fileBaseName,
                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_gain( gain ),
    m_orderPos( waveformItemOrderPos ),
    m_graphicsScene( graphicsScene ),
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
        const SharedWaveformItem waveformItem = m_graphicsScene->getWaveformAt( m_orderPos );

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
    const SharedWaveformItem item = m_graphicsScene->getWaveformAt( m_orderPos );
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
                                            WaveGraphicsScene* const graphicsScene,
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
    m_graphicsScene( graphicsScene ),
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
        const SharedWaveformItem waveformItem = m_graphicsScene->getWaveformAt( m_orderPos );

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
    const SharedWaveformItem item = m_graphicsScene->getWaveformAt( m_orderPos );
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
                                    WaveGraphicsScene* const graphicsScene,
                                    WaveGraphicsView* const graphicsView,
                                    const int sampleRate,
                                    AudioFileHandler& fileHandler,
                                    const QString tempDirPath,
                                    const QString fileBaseName,
                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_orderPos( waveformItemOrderPos ),
    m_graphicsScene( graphicsScene ),
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
        const SharedWaveformItem waveformItem = m_graphicsScene->getWaveformAt( m_orderPos );

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
    const SharedWaveformItem item = m_graphicsScene->getWaveformAt( m_orderPos );
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
                                WaveGraphicsScene* const graphicsScene,
                                WaveGraphicsView* const graphicsView,
                                QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOrderPos( waveformItemOrderPos ),
    m_graphicsScene( graphicsScene ),
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
    const SharedWaveformItem item = m_graphicsScene->getWaveformAt( mOrderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    sampleBuffer->reverse( 0, sampleBuffer->getNumFrames() );

    m_graphicsView->redrawWaveforms();
}



//==================================================================================================

GlobalTimeStretchCommand::GlobalTimeStretchCommand( MainWindow* const mainWindow,
                                                  WaveGraphicsScene* const graphicsScene,
                                                  WaveGraphicsView* const graphicsView,
                                                  QDoubleSpinBox* const spinBoxOriginalBPM,
                                                  QDoubleSpinBox* const spinBoxNewBPM,
                                                  QCheckBox* const checkBoxPitchCorrection,
                                                  const QString tempDirPath,
                                                  const QString fileBaseName,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsScene( graphicsScene ),
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
    setText( "Global Time Stretch" );
}



void GlobalTimeStretchCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    const int numChans = m_mainWindow->m_sampleHeader->numChans;

    for ( int i = 0; i < m_tempFilePaths.size(); i++ )
    {
        const QString filePath = m_tempFilePaths.at( i );
        const SharedSampleBuffer origSampleBuffer = m_mainWindow->m_fileHandler.getSampleData( filePath );

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



void GlobalTimeStretchCommand::redo()
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

        const int sampleRate = m_mainWindow->m_sampleHeader->sampleRate;
        const int numChans = m_mainWindow->m_sampleHeader->numChans;

        foreach ( SharedSampleBuffer sampleBuffer, m_mainWindow->m_sampleBufferList )
        {
            OfflineTimeStretcher::stretch( sampleBuffer, sampleRate, numChans, m_options, timeRatio, pitchScale );
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



void GlobalTimeStretchCommand::updateSlicePoints( const qreal timeRatio )
{
    QList<SharedSlicePointItem> slicePointList = m_graphicsScene->getSlicePointList();

    foreach ( SharedSlicePointItem slicePoint, slicePointList )
    {
        const int newFrameNum = roundToInt( slicePoint->getFrameNum() * timeRatio );
        slicePoint->setFrameNum( newFrameNum );
    }
}



void GlobalTimeStretchCommand::updateLoopMarkers( const qreal timeRatio )
{
    LoopMarkerItem* loopMarkerLeft = m_graphicsScene->getLeftLoopMarker();

    if ( loopMarkerLeft != NULL )
    {
        const int newFrameNum = roundToInt( loopMarkerLeft->getFrameNum() * timeRatio );
        loopMarkerLeft->setFrameNum( newFrameNum );
    }

    LoopMarkerItem* loopMarkerRight = m_graphicsScene->getRightLoopMarker();

    if ( loopMarkerRight != NULL )
    {
        const int newFrameNum = roundToInt( loopMarkerRight->getFrameNum() * timeRatio );
        loopMarkerRight->setFrameNum( newFrameNum );
    }
}



//==================================================================================================

RenderTimeStretchCommand::RenderTimeStretchCommand( MainWindow* const mainWindow,
                                                    WaveGraphicsScene* const graphicsScene,
                                                    WaveGraphicsView* const graphicsView,
                                                    const QString tempDirPath,
                                                    const QString fileBaseName,
                                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsScene( graphicsScene ),
    m_graphicsView( graphicsView ),
    m_options( m_mainWindow->m_optionsDialog->getStretcherOptions() & ~RubberBandStretcher::OptionProcessRealTime ),
    m_tempDirPath( tempDirPath ),
    m_fileBaseName( fileBaseName )
{
    setText( "Render Time Stretch" );
}



void RenderTimeStretchCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    m_mainWindow->stopPlayback();

    if ( m_mainWindow->m_rubberbandAudioSource == NULL )
    {
        m_mainWindow->m_optionsDialog->enableRealtimeMode();
    }

    const int lowestAssignedMidiNote = m_mainWindow->m_samplerAudioSource->getLowestAssignedMidiNote();
    const int numChans = m_mainWindow->m_sampleHeader->numChans;

    QList<int> orderPosList;

    for ( int i = 0; i < m_tempFilePaths.size(); i++ )
    {
        const QString filePath = m_tempFilePaths.at( i );
        const SharedSampleBuffer origSampleBuffer = m_mainWindow->m_fileHandler.getSampleData( filePath );

        const int origBufferSize = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = m_mainWindow->m_sampleBufferList.at( i );
        sampleBuffer->setSize( numChans, origBufferSize );

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, origBufferSize );
        }

        m_mainWindow->m_rubberbandAudioSource->setNoteTimeRatio( lowestAssignedMidiNote + i, m_timeRatioList.at( i ) );

        orderPosList << i;
    }

    m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                    m_mainWindow->m_sampleHeader->sampleRate );

    m_graphicsScene->resizeWaveforms( orderPosList, m_timeRatioList );

    QApplication::restoreOverrideCursor();
}



void RenderTimeStretchCommand::redo()
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
        m_timeRatioList.clear();

        const int lowestAssignedMidiNote = m_mainWindow->m_samplerAudioSource->getLowestAssignedMidiNote();

        const int sampleRate = m_mainWindow->m_sampleHeader->sampleRate;
        const int numChans = m_mainWindow->m_sampleHeader->numChans;

        const qreal pitchScale = 1.0;

        for ( int i = 0; i < m_mainWindow->m_sampleBufferList.size(); i++ )
        {
            const int midiNote = lowestAssignedMidiNote + i;
            const qreal timeRatio = m_mainWindow->m_rubberbandAudioSource->getNoteTimeRatio( midiNote );

            SharedSampleBuffer sampleBuffer = m_mainWindow->m_sampleBufferList.at( i );

            OfflineTimeStretcher::stretch( sampleBuffer, sampleRate, numChans, m_options, timeRatio, pitchScale );

            m_timeRatioList << timeRatio;
            m_mainWindow->m_rubberbandAudioSource->setNoteTimeRatio( midiNote, 1.0 );
        }

        m_mainWindow->m_samplerAudioSource->setSamples( m_mainWindow->m_sampleBufferList,
                                                        m_mainWindow->m_sampleHeader->sampleRate );

        m_graphicsView->redrawWaveforms();

        QApplication::restoreOverrideCursor();
    }
    else
    {
        QApplication::restoreOverrideCursor();

        MessageBoxes::showWarningDialog( m_mainWindow->m_fileHandler.getLastErrorTitle(),
                                         m_mainWindow->m_fileHandler.getLastErrorInfo() );
    }
}



//==================================================================================================

SelectiveTimeStretchCommand::SelectiveTimeStretchCommand( MainWindow* const mainWindow,
                                                          WaveGraphicsScene* const graphicsScene,
                                                          const QList<int> orderPositions,
                                                          const QList<qreal> timeRatios,
                                                          const QList<int> midiNotes,
                                                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    m_mainWindow( mainWindow ),
    m_graphicsScene( graphicsScene ),
    m_orderPositions( orderPositions ),
    m_origTimeRatios( graphicsScene->getWaveformScaleFactors( orderPositions ) ),
    m_timeRatios( timeRatios ),
    m_midiNotes( midiNotes )
{
    setText( "Selective Time Stretch" );
}



void SelectiveTimeStretchCommand::undo()
{
    for ( int i = 0; i < m_midiNotes.size(); i++ )
    {
        m_mainWindow->m_rubberbandAudioSource->setNoteTimeRatio( m_midiNotes.at( i ), m_origTimeRatios.at( i ) );
    }
    m_graphicsScene->resizeWaveforms( m_orderPositions, m_origTimeRatios );
}



void SelectiveTimeStretchCommand::redo()
{
    for ( int i = 0; i < m_midiNotes.size(); i++ )
    {
        m_mainWindow->m_rubberbandAudioSource->setNoteTimeRatio( m_midiNotes.at( i ), m_timeRatios.at( i ) );
    }
    m_graphicsScene->resizeWaveforms( m_orderPositions, m_timeRatios );
}
