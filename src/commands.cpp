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
    mGraphicsView->deleteSlicePoint( mSlicePointItem );

    if ( mGraphicsView->getSlicePointFrameNumList().isEmpty() )
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
    setText( "Add Slice Point" );
}



void DeleteSlicePointItemCommand::undo()
{
    mGraphicsView->addSlicePoint( mSlicePointItem );
    mSliceButton->setEnabled( true );
}



void DeleteSlicePointItemCommand::redo()
{
    mGraphicsView->deleteSlicePoint( mSlicePointItem );

    if ( mGraphicsView->getSlicePointFrameNumList().isEmpty() )
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

    mMainWindow->mSamplerAudioSource->setSample( mMainWindow->mCurrentSampleBuffer,
                                                 mMainWindow->mCurrentSampleHeader->sampleRate );
    mMainWindow->mSampleRangeList.clear();

    SharedSampleRange sampleRange( new SampleRange );
    sampleRange->startFrame = 0;
    sampleRange->numFrames = mMainWindow->mCurrentSampleBuffer->getNumFrames();
    mMainWindow->mSampleRangeList << sampleRange;

    mGraphicsView->clearWaveform();

    SharedWaveformItem item = mGraphicsView->createWaveform( mMainWindow->mCurrentSampleBuffer, sampleRange );
    mMainWindow->connectWaveformToMainWindow( item );

    mGraphicsView->showSlicePoints();

    mSliceButton->setEnabled( true );
    mFindOnsetsButton->setEnabled( true );
    mFindBeatsButton->setEnabled( true );
    mAddSlicePointAction->setEnabled( true );
    mSelectItemsAction->setEnabled( false );
    mAuditionItemsAction->trigger();

    QApplication::restoreOverrideCursor();
}



void SliceCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->getSampleRanges( mMainWindow->mSampleRangeList );

    Q_ASSERT( mMainWindow->mSampleRangeList.size() > 1 );

    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );

    mGraphicsView->hideSlicePoints();
    mGraphicsView->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList =
            mGraphicsView->createWaveforms( mMainWindow->mCurrentSampleBuffer,
                                            mMainWindow->mSampleRangeList );

    foreach ( SharedWaveformItem item, waveformItemList )
    {
        mMainWindow->connectWaveformToMainWindow( item );
    }

    mSliceButton->setEnabled( false );
    mFindOnsetsButton->setEnabled( false );
    mFindBeatsButton->setEnabled( false );
    mAddSlicePointAction->setEnabled( false );
    mSelectItemsAction->setEnabled( true );

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
    mGraphicsView->moveWaveforms( mNewOrderPositions, -mNumPlacesMoved );
    mMainWindow->reorderSampleRangeList( mNewOrderPositions, -mNumPlacesMoved );
}



void MoveWaveformItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        mGraphicsView->moveWaveforms( mOldOrderPositions, mNumPlacesMoved );
        mMainWindow->reorderSampleRangeList( mOldOrderPositions, mNumPlacesMoved );
    }
    mIsFirstRedoCall = false;
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
    mGraphicsView->selectNone();

    QList<SharedWaveformItem> items = mGraphicsView->splitWaveform( mJoinedItemOrderPos );

    mMainWindow->mSampleRangeList.removeAt( mJoinedItemOrderPos );

    foreach ( SharedWaveformItem item, items )
    {
        mMainWindow->mSampleRangeList.insert( item->getOrderPos(), item->getSampleRange() );
    }

    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );
}



void JoinCommand::redo()
{
    mGraphicsView->selectNone();

    SharedWaveformItem item = mGraphicsView->joinWaveforms( mOrderPositions );
    mMainWindow->connectWaveformToMainWindow( item );

    mJoinedItemOrderPos = item->getOrderPos();

    foreach ( int orderPos, mOrderPositions )
    {
        mMainWindow->mSampleRangeList.removeAt( mJoinedItemOrderPos );
    }

    mMainWindow->mSampleRangeList.insert( mJoinedItemOrderPos, item->getSampleRange() );

    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );
}



//==================================================================================================

SplitCommand::SplitCommand( const int orderPos,
                           WaveGraphicsView* const graphicsView,
                           MainWindow* const mainWindow,
                           QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mJoinedItemOrderPos( orderPos ),
    mGraphicsView( graphicsView ),
    mMainWindow( mainWindow )
{
    setText( "Join" );
}



void SplitCommand::undo()
{
    mGraphicsView->selectNone();

    SharedWaveformItem item = mGraphicsView->joinWaveforms( mOrderPositions );
    mMainWindow->connectWaveformToMainWindow( item );

    foreach ( int orderPos, mOrderPositions )
    {
        mMainWindow->mSampleRangeList.removeAt( mJoinedItemOrderPos );
    }

    mMainWindow->mSampleRangeList.insert( mJoinedItemOrderPos, item->getSampleRange() );

    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );
}



void SplitCommand::redo()
{
    mGraphicsView->selectNone();

    QList<SharedWaveformItem> items = mGraphicsView->splitWaveform( mJoinedItemOrderPos );

    mMainWindow->mSampleRangeList.removeAt( mJoinedItemOrderPos );

    mOrderPositions.clear();

    foreach ( SharedWaveformItem item, items )
    {
        mOrderPositions << item->getOrderPos();
        mMainWindow->mSampleRangeList.insert( item->getOrderPos(), item->getSampleRange() );
    }

    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );
}



//==================================================================================================

ApplyGainCommand::ApplyGainCommand( const float gain,
                                    const int waveformItemOrderPos,
                                    WaveGraphicsView* const graphicsView,
                                    const SharedSampleHeader sampleHeader,
                                    AudioFileHandler& fileHandler,
                                    const QString tempDirPath,
                                    const QString fileBaseName,
                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mGain( gain ),
    mWaveformItemOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView ),
    mSampleHeader( sampleHeader ),
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
        const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
        SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
        SharedSampleRange sampleRange = item->getSampleRange();

        SharedSampleBuffer tempBuffer = mFileHandler.getSampleData( mFilePath );

        for ( int chanNum = 0; chanNum < sampleBuffer->getNumChannels(); chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, sampleRange->startFrame, *tempBuffer.data(), chanNum, 0, sampleRange->numFrames );
        }

        mGraphicsView->forceRedraw();
    }
}



void ApplyGainCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
    SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
    SharedSampleRange sampleRange = item->getSampleRange();

    const int numChans = mSampleHeader->numChans;

    SharedSampleBuffer tempBuffer( new SampleBuffer( numChans, sampleRange->numFrames ) );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        tempBuffer->copyFrom( chanNum, 0, *sampleBuffer.data(), chanNum, sampleRange->startFrame, sampleRange->numFrames );
    }

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath, mFileBaseName, tempBuffer, mSampleHeader, AudioFileHandler::TEMP_FORMAT );

    if ( ! mFilePath.isEmpty() )
    {
        sampleBuffer->applyGain( sampleRange->startFrame, sampleRange->numFrames, mGain );

        mGraphicsView->forceRedraw();
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
                                            const SharedSampleHeader sampleHeader,
                                            AudioFileHandler& fileHandler,
                                            const QString tempDirPath,
                                            const QString fileBaseName,
                                            QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mStartGain( startGain ),
    mEndGain( endGain ),
    mWaveformItemOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView ),
    mSampleHeader( sampleHeader ),
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
        const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
        SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
        SharedSampleRange sampleRange = item->getSampleRange();

        SharedSampleBuffer tempBuffer = mFileHandler.getSampleData( mFilePath );

        for ( int chanNum = 0; chanNum < sampleBuffer->getNumChannels(); chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, sampleRange->startFrame, *tempBuffer.data(), chanNum, 0, sampleRange->numFrames );
        }

        mGraphicsView->forceRedraw();
    }
}



void ApplyGainRampCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
    SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
    SharedSampleRange sampleRange = item->getSampleRange();

    const int numChans = mSampleHeader->numChans;

    SharedSampleBuffer tempBuffer( new SampleBuffer( numChans, sampleRange->numFrames ) );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        tempBuffer->copyFrom( chanNum, 0, *sampleBuffer.data(), chanNum, sampleRange->startFrame, sampleRange->numFrames );
    }

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath, mFileBaseName, tempBuffer, mSampleHeader, AudioFileHandler::TEMP_FORMAT );

    if ( ! mFilePath.isEmpty() )
    {
        sampleBuffer->applyGainRamp( sampleRange->startFrame, sampleRange->numFrames, mStartGain, mEndGain );

        mGraphicsView->forceRedraw();
    }
    else
    {
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



//==================================================================================================

NormaliseCommand::NormaliseCommand( const int waveformItemOrderPos,
                                    WaveGraphicsView* const graphicsView,
                                    const SharedSampleHeader sampleHeader,
                                    AudioFileHandler& fileHandler,
                                    const QString tempDirPath,
                                    const QString fileBaseName,
                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mWaveformItemOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView ),
    mSampleHeader( sampleHeader ),
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
        const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
        SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
        SharedSampleRange sampleRange = item->getSampleRange();

        SharedSampleBuffer tempBuffer = mFileHandler.getSampleData( mFilePath );

        for ( int chanNum = 0; chanNum < sampleBuffer->getNumChannels(); chanNum++ )
        {
            sampleBuffer->copyFrom( chanNum, sampleRange->startFrame, *tempBuffer.data(), chanNum, 0, sampleRange->numFrames );
        }

        mGraphicsView->forceRedraw();
    }
}



void NormaliseCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
    SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
    SharedSampleRange sampleRange = item->getSampleRange();

    const int numChans = mSampleHeader->numChans;

    SharedSampleBuffer tempBuffer( new SampleBuffer( numChans, sampleRange->numFrames ) );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        tempBuffer->copyFrom( chanNum, 0, *sampleBuffer.data(), chanNum, sampleRange->startFrame, sampleRange->numFrames );
    }

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath, mFileBaseName, tempBuffer, mSampleHeader, AudioFileHandler::TEMP_FORMAT );

    if ( ! mFilePath.isEmpty() )
    {
        const float magnitude = sampleBuffer->getMagnitude( sampleRange->startFrame, sampleRange->numFrames );

        if ( magnitude > 0.0 )
        {
            sampleBuffer->applyGain( sampleRange->startFrame, sampleRange->numFrames, 1.0 / magnitude );
            mGraphicsView->forceRedraw();
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
    mWaveformItemOrderPos( waveformItemOrderPos ),
    mGraphicsView( graphicsView )
{
    setText( "Reverse" );
}



void ReverseCommand::undo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
    SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
    SharedSampleRange sampleRange = item->getSampleRange();

    sampleBuffer->reverse( sampleRange->startFrame, sampleRange->numFrames );

    mGraphicsView->forceRedraw();
}



void ReverseCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
    SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
    SharedSampleRange sampleRange = item->getSampleRange();

    sampleBuffer->reverse( sampleRange->startFrame, sampleRange->numFrames );

    mGraphicsView->forceRedraw();
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

    SharedSampleBuffer tempBuffer = mMainWindow->mFileHandler.getSampleData( mTempFilePath );

    const int numChans = tempBuffer->getNumChannels();
    const int numFrames = tempBuffer->getNumFrames();

    mMainWindow->mCurrentSampleBuffer->setSize( numChans, numFrames );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        mMainWindow->mCurrentSampleBuffer->copyFrom( chanNum, 0, *tempBuffer.data(), chanNum, 0, numFrames );
    }

    const qreal timeRatio = 1.0 / ( mOriginalBPM / mNewBPM );

    updateSampleRanges( timeRatio, numFrames );
    mMainWindow->resetSampler();

    updateSlicePoints( timeRatio );
    mGraphicsView->forceRedraw();

    mSpinBoxOriginalBPM->setValue( mOriginalBPM );
    mSpinBoxNewBPM->setValue( mOriginalBPM );
    mCheckBoxPitchCorrection->setChecked( mIsPitchCorrectionEnabled );

    mMainWindow->mAppliedBPM = mPrevAppliedBPM;

    QApplication::restoreOverrideCursor();
}



void ApplyTimeStretchCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mTempFilePath = mMainWindow->mFileHandler.saveAudioFile( mTempDirPath,
                                                             mFileBaseName,
                                                             mMainWindow->mCurrentSampleBuffer,
                                                             mMainWindow->mCurrentSampleHeader,
                                                             AudioFileHandler::TEMP_FORMAT );

    if ( ! mTempFilePath.isEmpty() )
    {
        const qreal timeRatio = mOriginalBPM / mNewBPM;
        const qreal pitchScale = mIsPitchCorrectionEnabled ? 1.0 : mNewBPM / mOriginalBPM;

        const int newTotalNumFrames = stretch( timeRatio, pitchScale );

        updateSampleRanges( timeRatio, newTotalNumFrames );
        mMainWindow->resetSampler();

        updateSlicePoints( timeRatio );
        mGraphicsView->forceRedraw();

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



int ApplyTimeStretchCommand::stretch( const qreal timeRatio, const qreal pitchScale )
{
    const int sampleRate = mMainWindow->mCurrentSampleHeader->sampleRate;
    const int numChans = mMainWindow->mCurrentSampleHeader->numChans;

    RubberBandStretcher stretcher( sampleRate, numChans, mOptions, timeRatio, pitchScale );

    // Copy current sample buffer to a temporary buffer
    SharedSampleBuffer tempBuffer( new SampleBuffer( *(mMainWindow->mCurrentSampleBuffer.data()) ) );

    const int origNumFrames = tempBuffer->getNumFrames();
    const int newBufferSize = roundToInt( origNumFrames * timeRatio );
    float** inFloatBuffer = new float*[ numChans ];
    float** outFloatBuffer = new float*[ numChans ];
    int inFrameNum = 0;
    int totalNumFramesRetrieved = 0;
//            std::map<size_t, size_t> mapping;
//
//            if ( ! mMainWindow->mSampleRangeList.isEmpty() )
//            {
//                foreach ( SharedSampleRange range, mMainWindow->mSampleRangeList )
//                {
//                    mapping[ range->startFrame ] = roundToInt( range->startFrame * timeRatio );
//                }
//            }

    stretcher.setExpectedInputDuration( origNumFrames );

    mMainWindow->mCurrentSampleBuffer->setSize( numChans, newBufferSize );

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        inFloatBuffer[ chanNum ] = tempBuffer->getArrayOfChannels()[ chanNum ];
        outFloatBuffer[ chanNum ] = mMainWindow->mCurrentSampleBuffer->getArrayOfChannels()[ chanNum ];
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
            if ( mMainWindow->mCurrentSampleBuffer->getNumFrames() < totalNumFramesRetrieved + numAvailable )
            {
                mMainWindow->mCurrentSampleBuffer->setSize( numChans,                                  // No. of channels
                                                            totalNumFramesRetrieved + numAvailable,    // New no. of frames
                                                            true );                                    // Keep existing content

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    outFloatBuffer[ chanNum ] = mMainWindow->mCurrentSampleBuffer->getArrayOfChannels()[ chanNum ];
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
            if ( mMainWindow->mCurrentSampleBuffer->getNumFrames() < totalNumFramesRetrieved + numAvailable )
            {
                mMainWindow->mCurrentSampleBuffer->setSize( numChans,                                  // No. of channels
                                                            totalNumFramesRetrieved + numAvailable,    // New no. of frames
                                                            true );                                    // Keep existing content

                for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                {
                    outFloatBuffer[ chanNum ] = mMainWindow->mCurrentSampleBuffer->getArrayOfChannels()[ chanNum ];
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

    if ( mMainWindow->mCurrentSampleBuffer->getNumFrames() != totalNumFramesRetrieved )
    {
        mMainWindow->mCurrentSampleBuffer->setSize( numChans, totalNumFramesRetrieved, true );
    }

    delete[] inFloatBuffer;
    delete[] outFloatBuffer;

    return totalNumFramesRetrieved;
}



void ApplyTimeStretchCommand::updateSampleRanges( const qreal timeRatio, const int newTotalNumFrames )
{
    // Update start frame and length of all sample ranges while preserving current ordering of list
    if ( mMainWindow->mSampleRangeList.size() > 1 )
    {
        QList<SharedSampleRange> tempList( mMainWindow->mSampleRangeList );

        qSort( tempList.begin(), tempList.end(), SampleRange::isLessThan );

        foreach ( SharedSampleRange range, tempList )
        {
            const int newStartFrame = roundToInt( range->startFrame * timeRatio );
            range->startFrame = newStartFrame;
        }

        for ( int i = 0; i < tempList.size(); i++ )
        {
            const int newNumFrames = i + 1 < tempList.size() ?
                                     tempList.at( i + 1 )->startFrame - tempList.at( i )->startFrame :
                                     newTotalNumFrames - tempList.at( i )->startFrame;

            tempList.at( i )->numFrames = newNumFrames;
        }
    }
    else
    {
        mMainWindow->mSampleRangeList.first()->numFrames = newTotalNumFrames;
    }
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
