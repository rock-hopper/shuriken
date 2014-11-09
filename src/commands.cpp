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
    mGraphicsView( graphicsView ),
    mSliceButton( sliceButton )
{
    setText( "Add Slice Point" );
    mSlicePointItem = mGraphicsView->createSlicePoint( frameNum );
    mIsFirstRedoCall = true;
}



void AddSlicePointItemCommand::undo()
{
    mGraphicsView->removeSlicePoint( mSlicePointItem );

    if ( mGraphicsView->getSlicePointFrameNums().isEmpty() )
    {
        mSliceButton->setEnabled( false );
    }
}



void AddSlicePointItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        mGraphicsView->addSlicePoint( mSlicePointItem );
    }
    mIsFirstRedoCall = false;

    mSliceButton->setEnabled( true );
}



//==================================================================================================

MoveSlicePointItemCommand::MoveSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                                      const int oldFrameNum,
                                                      const int newFrameNum,
                                                      WaveGraphicsView* const graphicsView,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mSlicePointItem( slicePoint ),
    mOldFrameNum( oldFrameNum ),
    mNewFrameNum( newFrameNum ),
    mGraphicsView( graphicsView )
{
    setText( "Move Slice Point" );
    mIsFirstRedoCall = true;
}



void MoveSlicePointItemCommand::undo()
{
    mGraphicsView->moveSlicePoint( mSlicePointItem, mOldFrameNum );
}



void MoveSlicePointItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        mGraphicsView->moveSlicePoint( mSlicePointItem, mNewFrameNum );
    }
    mIsFirstRedoCall = false;
}



//==================================================================================================

DeleteSlicePointItemCommand::DeleteSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                                          WaveGraphicsView* const graphicsView,
                                                          QPushButton* const sliceButton,
                                                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mSlicePointItem( slicePoint ),
    mGraphicsView( graphicsView ),
    mSliceButton( sliceButton )
{
    setText( "Delete Slice Point" );
}



void DeleteSlicePointItemCommand::undo()
{
    mGraphicsView->addSlicePoint( mSlicePointItem );
    mSliceButton->setEnabled( true );
}



void DeleteSlicePointItemCommand::redo()
{
    mGraphicsView->removeSlicePoint( mSlicePointItem );

    if ( mGraphicsView->getSlicePointFrameNums().isEmpty() )
    {
        mSliceButton->setEnabled( false );
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
    mMainWindow( mainWindow ),
    mGraphicsView( graphicsView ),
    mSliceButton( sliceButton ),
    mFindOnsetsButton( findOnsetsButton ),
    mFindBeatsButton( findBeatsButton ),
    mAddSlicePointAction( addSlicePointAction ),
    mMoveItemsAction( moveItemsAction ),
    mSelectItemsAction( selectItemsAction ),
    mAuditionItemsAction( auditionItemsAction )
{
    setText( "Slice" );
}



void SliceCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->stopPlayback();

    SharedSampleBuffer singleBuffer = SampleUtils::joinSampleBuffers( mMainWindow->mSampleBufferList );

    mMainWindow->mSampleBufferList.clear();
    mMainWindow->mSampleBufferList << singleBuffer;

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );

    mGraphicsView->clearWaveform();

    SharedWaveformItem item = mGraphicsView->createWaveform( mMainWindow->mSampleBufferList.first(),
                                                             mMainWindow->mSampleHeader );
    mMainWindow->connectWaveformToMainWindow( item );

    mGraphicsView->showSlicePoints();

    mSliceButton->setEnabled( true );
    mFindOnsetsButton->setEnabled( true );
    mFindBeatsButton->setEnabled( true );
    mAddSlicePointAction->setEnabled( true );
    mSelectItemsAction->setEnabled( false );
    mAuditionItemsAction->trigger();

    mMainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



void SliceCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->stopPlayback();

    mMainWindow->mSampleBufferList = SampleUtils::splitSampleBuffer( mMainWindow->mSampleBufferList.first(),
                                                                     mGraphicsView->getSlicePointFrameNums() );

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );

    mGraphicsView->hideSlicePoints();
    mGraphicsView->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList = mGraphicsView->createWaveforms( mMainWindow->mSampleBufferList,
                                                                                       mMainWindow->mSampleHeader );
    foreach ( SharedWaveformItem item, waveformItemList )
    {
        mMainWindow->connectWaveformToMainWindow( item );
    }

    mSliceButton->setEnabled( false );
    mFindOnsetsButton->setEnabled( false );
    mFindBeatsButton->setEnabled( false );
    mAddSlicePointAction->setEnabled( false );
    mSelectItemsAction->setEnabled( true );

    mMainWindow->updateSnapLoopMarkersComboBox();

    QApplication::restoreOverrideCursor();
}



//==================================================================================================

MoveWaveformItemCommand::MoveWaveformItemCommand( const QList<int> oldOrderPositions,
                                                  const int numPlacesMoved,
                                                  WaveGraphicsView* const graphicsView,
                                                  MainWindow* const mainWindow,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOldOrderPositions( oldOrderPositions ),
    mNumPlacesMoved( numPlacesMoved ),
    mGraphicsView( graphicsView ),
    mMainWindow( mainWindow )
{
    setText( "Move Waveform Item" );
    mIsFirstRedoCall = true;

    foreach ( int orderPos, mOldOrderPositions )
    {
        mNewOrderPositions << orderPos + mNumPlacesMoved;
    }
}



void MoveWaveformItemCommand::undo()
{
    mMainWindow->reorderSampleBufferList( mNewOrderPositions, -mNumPlacesMoved );
    mGraphicsView->moveWaveforms( mNewOrderPositions, -mNumPlacesMoved );
}



void MoveWaveformItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        mMainWindow->reorderSampleBufferList( mOldOrderPositions, mNumPlacesMoved );
        mGraphicsView->moveWaveforms( mOldOrderPositions, mNumPlacesMoved );
    }
    mIsFirstRedoCall = false;
}



//==================================================================================================

DeleteWaveformItemCommand::DeleteWaveformItemCommand( const QList<int> orderPositions,
                                                      WaveGraphicsView* const graphicsView,
                                                      MainWindow* const mainWindow,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOrderPositions( orderPositions ),
    mGraphicsView( graphicsView ),
    mMainWindow( mainWindow )
{
    setText( "Delete Waveform Item" );
}



void DeleteWaveformItemCommand::undo()
{
    mMainWindow->stopPlayback();

    mGraphicsView->insertWaveforms( mRemovedWaveforms );

    const int firstOrderPos = mOrderPositions.first();

    for ( int i = 0; i < mOrderPositions.size(); i++ )
    {
        mMainWindow->mSampleBufferList.insert( firstOrderPos + i, mRemovedSampleBuffers.at( i ) );
    }

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );
}



void DeleteWaveformItemCommand::redo()
{
    mMainWindow->stopPlayback();

    mRemovedWaveforms = mGraphicsView->removeWaveforms( mOrderPositions );

    mRemovedSampleBuffers.clear();

    const int firstOrderPos = mOrderPositions.first();

    for ( int i = 0; i < mOrderPositions.size(); i++ )
    {
        mRemovedSampleBuffers << mMainWindow->mSampleBufferList.at( firstOrderPos );
        mMainWindow->mSampleBufferList.removeAt( firstOrderPos );
    }

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );
}



//==================================================================================================

JoinCommand::JoinCommand( const QList<int> orderPositions,
                          WaveGraphicsView* const graphicsView,
                          MainWindow* const mainWindow,
                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOrderPositions( orderPositions ),
    mGraphicsView( graphicsView ),
    mMainWindow( mainWindow )
{
    setText( "Join" );
}



void JoinCommand::undo()
{
    mMainWindow->stopPlayback();
    mGraphicsView->selectNone();

    QList<SharedWaveformItem> items = mGraphicsView->splitWaveform( mJoinedItemOrderPos, mSlicePoints );

    mMainWindow->mSampleBufferList.removeAt( mJoinedItemOrderPos );

    foreach ( SharedWaveformItem item, items )
    {
        mMainWindow->connectWaveformToMainWindow( item );
        mMainWindow->mSampleBufferList.insert( item->getOrderPos(), item->getSampleBuffer() );
    }

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );
}



void JoinCommand::redo()
{
    mMainWindow->stopPlayback();
    mGraphicsView->selectNone();

    SharedWaveformItem item = mGraphicsView->joinWaveforms( mOrderPositions );
    mMainWindow->connectWaveformToMainWindow( item );

    mSlicePoints.clear();
    int frameNum = 0;

    for ( int i = 0; i < mOrderPositions.size() - 1; i++ )
    {
        const int orderPos = mOrderPositions.at( i );

        frameNum += mMainWindow->mSampleBufferList.at( orderPos )->getNumFrames();

        mSlicePoints << frameNum;
    }

    mJoinedItemOrderPos = item->getOrderPos();

    for ( int i = 0; i < mOrderPositions.size(); i++ )
    {
        mMainWindow->mSampleBufferList.removeAt( mJoinedItemOrderPos );
    }

    mMainWindow->mSampleBufferList.insert( mJoinedItemOrderPos, item->getSampleBuffer() );

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );
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
    mGain( gain ),
    mOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView ),
    mSampleRate( sampleRate ),
    mFileHandler( fileHandler ),
    mTempDirPath( tempDirPath ),
    mFileBaseName( fileBaseName )
{
    setText( "Apply Gain" );
}



void ApplyGainCommand::undo()
{
    if ( ! mFilePath.isEmpty() )
    {
        const SharedWaveformItem waveformItem = mGraphicsView->getWaveformAt( mOrderPos );

        const SharedSampleBuffer origSampleBuffer = mFileHandler.getSampleData( mFilePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int numFrames = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = waveformItem->getSampleBuffer();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, numFrames );
        }

        mGraphicsView->redrawWaveforms();
    }
}



void ApplyGainCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mOrderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath,
                                            mFileBaseName,
                                            sampleBuffer,
                                            mSampleRate,
                                            mSampleRate,
                                            AudioFileHandler::TEMP_FORMAT );

    if ( ! mFilePath.isEmpty() )
    {
        sampleBuffer->applyGain( 0, sampleBuffer->getNumFrames(), mGain );
        mGraphicsView->redrawWaveforms();
    }
    else
    {
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
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
    mStartGain( startGain ),
    mEndGain( endGain ),
    mOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView ),
    mSampleRate( sampleRate ),
    mFileHandler( fileHandler ),
    mTempDirPath( tempDirPath ),
    mFileBaseName( fileBaseName )
{
    setText( "Apply Gain Ramp" );
}



void ApplyGainRampCommand::undo()
{
    if ( ! mFilePath.isEmpty() )
    {
        const SharedWaveformItem waveformItem = mGraphicsView->getWaveformAt( mOrderPos );

        const SharedSampleBuffer origSampleBuffer = mFileHandler.getSampleData( mFilePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int numFrames = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = waveformItem->getSampleBuffer();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, numFrames );
        }

        mGraphicsView->redrawWaveforms();
    }
}



void ApplyGainRampCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mOrderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath,
                                            mFileBaseName,
                                            sampleBuffer,
                                            mSampleRate,
                                            mSampleRate,
                                            AudioFileHandler::TEMP_FORMAT );

    if ( ! mFilePath.isEmpty() )
    {
        sampleBuffer->applyGainRamp( 0, sampleBuffer->getNumFrames(), mStartGain, mEndGain );
        mGraphicsView->redrawWaveforms();
    }
    else
    {
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
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
    mOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView ),
    mSampleRate( sampleRate ),
    mFileHandler( fileHandler ),
    mTempDirPath( tempDirPath ),
    mFileBaseName( fileBaseName )
{
    setText( "Normalise" );
}



void NormaliseCommand::undo()
{
    if ( ! mFilePath.isEmpty() )
    {
        const SharedWaveformItem waveformItem = mGraphicsView->getWaveformAt( mOrderPos );

        const SharedSampleBuffer origSampleBuffer = mFileHandler.getSampleData( mFilePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int numFrames = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = waveformItem->getSampleBuffer();

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, numFrames );
        }

        mGraphicsView->redrawWaveforms();
    }
}



void NormaliseCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mOrderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath,
                                            mFileBaseName,
                                            sampleBuffer,
                                            mSampleRate,
                                            mSampleRate,
                                            AudioFileHandler::TEMP_FORMAT );

    if ( ! mFilePath.isEmpty() )
    {
        const int numFrames = sampleBuffer->getNumFrames();
        const float magnitude = sampleBuffer->getMagnitude( 0, numFrames );

        if ( magnitude > 0.0 )
        {
            sampleBuffer->applyGain( 0, numFrames, 1.0 / magnitude );
            mGraphicsView->redrawWaveforms();
        }
    }
    else
    {
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



//==================================================================================================

ReverseCommand::ReverseCommand( const int waveformItemOrderPos,
                                WaveGraphicsView* const graphicsView,
                                QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView )
{
    setText( "Reverse" );
}



void ReverseCommand::undo()
{
    redo();
}



void ReverseCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mOrderPos );
    const SharedSampleBuffer sampleBuffer = item->getSampleBuffer();

    sampleBuffer->reverse( 0, sampleBuffer->getNumFrames() );

    mGraphicsView->redrawWaveforms();
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
    mMainWindow( mainWindow ),
    mGraphicsView( graphicsView ),
    mSpinBoxOriginalBPM( spinBoxOriginalBPM ),
    mSpinBoxNewBPM( spinBoxNewBPM ),
    mCheckBoxPitchCorrection( checkBoxPitchCorrection ),
    mOriginalBPM( mSpinBoxOriginalBPM->value() ),
    mNewBPM( mSpinBoxNewBPM->value() ),
    mPrevAppliedBPM( mMainWindow->mAppliedBPM ),
    mIsPitchCorrectionEnabled( mCheckBoxPitchCorrection->isChecked() ),
    mOptions( mMainWindow->mOptionsDialog->getStretcherOptions() ),
    mTempDirPath( tempDirPath ),
    mFileBaseName( fileBaseName )
{
    setText( "Apply Timestretch" );
}



void ApplyTimeStretchCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->stopPlayback();

    for ( int i = 0; i < mTempFilePaths.size(); i++ )
    {
        const QString filePath = mTempFilePaths.at( i );
        const SharedSampleBuffer origSampleBuffer = mMainWindow->mFileHandler.getSampleData( filePath );

        const int numChans = origSampleBuffer->getNumChannels();
        const int origBufferSize = origSampleBuffer->getNumFrames();

        const SharedSampleBuffer sampleBuffer = mMainWindow->mSampleBufferList.at( i );
        sampleBuffer->setSize( numChans, origBufferSize );

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, 0, *origSampleBuffer.data(), chanNum, 0, origBufferSize );
        }
    }

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                  mMainWindow->mSampleHeader->sampleRate );

    const qreal timeRatio = 1.0 / ( mOriginalBPM / mNewBPM );

    updateLoopMarkers( timeRatio );
    updateSlicePoints( timeRatio );
    mGraphicsView->redrawWaveforms();

    mSpinBoxOriginalBPM->setValue( mOriginalBPM );
    mSpinBoxNewBPM->setValue( mOriginalBPM );
    mCheckBoxPitchCorrection->setChecked( mIsPitchCorrectionEnabled );

    mMainWindow->mAppliedBPM = mPrevAppliedBPM;

    QApplication::restoreOverrideCursor();
}



void ApplyTimeStretchCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->stopPlayback();

    mTempFilePaths.clear();

    for ( int i = 0; i < mMainWindow->mSampleBufferList.size(); i++ )
    {
        const QString path = mMainWindow->mFileHandler.saveAudioFile( mTempDirPath,
                                                                      mFileBaseName + "_" + QString::number( i ),
                                                                      mMainWindow->mSampleBufferList.at( i ),
                                                                      mMainWindow->mSampleHeader->sampleRate,
                                                                      mMainWindow->mSampleHeader->sampleRate,
                                                                      AudioFileHandler::TEMP_FORMAT );
        if ( ! path.isEmpty() )
        {
            mTempFilePaths << path;
        }
        else
        {
            mTempFilePaths.clear();
            break;
        }
    }

    if ( ! mTempFilePaths.isEmpty() )
    {
        const qreal timeRatio = mOriginalBPM / mNewBPM;
        const qreal pitchScale = mIsPitchCorrectionEnabled ? 1.0 : mNewBPM / mOriginalBPM;

        int newTotalNumFrames = 0;

        foreach ( SharedSampleBuffer sampleBuffer, mMainWindow->mSampleBufferList )
        {
            newTotalNumFrames += stretch( sampleBuffer, timeRatio, pitchScale );
        }

        mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSampleBufferList,
                                                      mMainWindow->mSampleHeader->sampleRate );

        updateLoopMarkers( timeRatio );
        updateSlicePoints( timeRatio );
        mGraphicsView->redrawWaveforms();

        mSpinBoxOriginalBPM->setValue( mNewBPM );
        mSpinBoxNewBPM->setValue( mNewBPM );
        mCheckBoxPitchCorrection->setChecked( mIsPitchCorrectionEnabled );

        mMainWindow->mAppliedBPM = mNewBPM;

        QApplication::restoreOverrideCursor();
    }
    else
    {
        QApplication::restoreOverrideCursor();

        MessageBoxes::showWarningDialog( mMainWindow->mFileHandler.getLastErrorTitle(),
                                       mMainWindow->mFileHandler.getLastErrorInfo() );
    }
}



int ApplyTimeStretchCommand::stretch( const SharedSampleBuffer sampleBuffer, const qreal timeRatio, const qreal pitchScale )
{
    const int sampleRate = mMainWindow->mSampleHeader->sampleRate;
    const int numChans = mMainWindow->mSampleHeader->numChans;

    RubberBandStretcher stretcher( sampleRate, numChans, mOptions, timeRatio, pitchScale );

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
    QList<SharedSlicePointItem> slicePointList = mGraphicsView->getSlicePointList();

    foreach ( SharedSlicePointItem slicePoint, slicePointList )
    {
        const int newFrameNum = roundToInt( slicePoint->getFrameNum() * timeRatio );
        slicePoint->setFrameNum( newFrameNum );
    }
}



void ApplyTimeStretchCommand::updateLoopMarkers( const qreal timeRatio )
{
    LoopMarkerItem* loopMarkerLeft = mGraphicsView->getLeftLoopMarker();

    if ( loopMarkerLeft != NULL )
    {
        const int newFrameNum = roundToInt( loopMarkerLeft->getFrameNum() * timeRatio );
        loopMarkerLeft->setFrameNum( newFrameNum );
    }

    LoopMarkerItem* loopMarkerRight = mGraphicsView->getRightLoopMarker();

    if ( loopMarkerRight != NULL )
    {
        const int newFrameNum = roundToInt( loopMarkerRight->getFrameNum() * timeRatio );
        loopMarkerRight->setFrameNum( newFrameNum );
    }
}
