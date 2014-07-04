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

AddSlicePointItemsCommand::AddSlicePointItemsCommand( QPushButton* const findOnsetsButton,
                                                      QPushButton* const findBeatsButton,
                                                      QUndoCommand* parent )
    : QUndoCommand( parent ), mFindOnsetsButton( findOnsetsButton ), mFindBeatsButton( findBeatsButton )
{
    setText( "Add Slice Points" );
}



void AddSlicePointItemsCommand::undo()
{
    mFindOnsetsButton->setEnabled( true );
    mFindBeatsButton->setEnabled( true );

    for ( int i = 0; i < childCount(); i++ )
    {
        QUndoCommand* command = const_cast<QUndoCommand*>( child( i ) );
        command->undo();
    }
}



void AddSlicePointItemsCommand::redo()
{
    mFindOnsetsButton->setEnabled( false );
    mFindBeatsButton->setEnabled( false );

    for ( int i = 0; i < childCount(); i++ )
    {
        QUndoCommand* command = const_cast<QUndoCommand*>( child( i ) );
        command->redo();
    }
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
                            QAction* const addSlicePointAction,
                            QAction* const moveItemsAction,
                            QAction* const selectItemsAction,
                            QAction* const auditionItemsAction,
                            QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mMainWindow( mainWindow ),
    mGraphicsView( graphicsView ),
    mSliceButton( sliceButton ),
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
    mAddSlicePointAction->setEnabled( true );
    mMoveItemsAction->setEnabled( false );
    mSelectItemsAction->setEnabled( false );
    mAuditionItemsAction->setEnabled( false );
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
    mAddSlicePointAction->setEnabled( false );
    mMoveItemsAction->setEnabled( true );
    mSelectItemsAction->setEnabled( true );
    mAuditionItemsAction->setEnabled( true );

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

        sampleRange->gain = mPrevGain;

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

    mFilePath = mFileHandler.saveAudioFile( mTempDirPath, mFileBaseName, tempBuffer, mSampleHeader, true );

    if ( ! mFilePath.isEmpty() )
    {
        sampleBuffer->applyGain( sampleRange->startFrame, sampleRange->numFrames, mGain );

        mPrevGain = sampleRange->gain;
        sampleRange->gain = mGain;

        mGraphicsView->forceRedraw();
    }
    else
    {
        MainWindow::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
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
    sampleRange->isReversed = false;

    mGraphicsView->forceRedraw();
}



void ReverseCommand::redo()
{
    const SharedWaveformItem item = mGraphicsView->getWaveformAt( mWaveformItemOrderPos );
    SharedSampleBuffer sampleBuffer = item->getSampleBuffer();
    SharedSampleRange sampleRange = item->getSampleRange();

    sampleBuffer->reverse( sampleRange->startFrame, sampleRange->numFrames );
    sampleRange->isReversed = true;

    mGraphicsView->forceRedraw();
}



//==================================================================================================

ApplyTimeStretchCommand::ApplyTimeStretchCommand( MainWindow* const mainWindow,
                                                  WaveGraphicsView* const graphicsView,
                                                  QDoubleSpinBox* const spinBoxOriginalBPM,
                                                  QDoubleSpinBox* const spinBoxNewBPM,
                                                  QCheckBox* const checkBoxPitchCorrection,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mMainWindow( mainWindow ),
    mGraphicsView( graphicsView ),
    mSpinBoxOriginalBPM( spinBoxOriginalBPM ),
    mSpinBoxNewBPM( spinBoxNewBPM ),
    mCheckBoxPitchCorrection( checkBoxPitchCorrection ),
    mOriginalBPM( mSpinBoxOriginalBPM->value() ),
    mNewBPM( mSpinBoxNewBPM->value() ),
    mIsPitchCorrectionEnabled( mCheckBoxPitchCorrection->isChecked() )
{
    setText( "Apply Timestretch" );

    // Search undo stack for previous timestretch command
    int index = mMainWindow->mUndoStack.index() - 1;
    bool isFound = false;

    while ( index >= 0 && ! isFound )
    {
        const QUndoCommand* command = mMainWindow->mUndoStack.command( index );
        const ApplyTimeStretchCommand* tsCommand = dynamic_cast<const ApplyTimeStretchCommand*>( command );

        if ( tsCommand != NULL )
        {
            mPrevOriginalBPM = tsCommand->getOriginalBPM();
            mPrevNewBPM = tsCommand->getNewBPM();
            mPrevIsPitchCorrectionEnabled = tsCommand->isPitchCorrectionEnabled();
            mPrevTimeRatio = mPrevOriginalBPM / mPrevNewBPM;

            isFound = true;
        }
        --index;
    }

    // If no previous timestretch command was found then set default values
    if ( ! isFound )
    {
        mPrevOriginalBPM = 0.0;
        mPrevNewBPM = 0.0;
        mPrevIsPitchCorrectionEnabled = true;
        mPrevTimeRatio = 1.0;
    }
}



void ApplyTimeStretchCommand::undo()
{
    mPrevTimeRatio = mOriginalBPM / mNewBPM;

    if ( mPrevOriginalBPM > 0.0 && mPrevNewBPM > 0.0 )
    {
        const qreal timeRatio = mPrevOriginalBPM / mPrevNewBPM;
        const qreal pitchRatio = mPrevIsPitchCorrectionEnabled ? 1.0 : mPrevNewBPM / mPrevOriginalBPM;

        stretch( timeRatio, pitchRatio );
    }
    else
    {
        stretch( 1.0, 1.0 );
    }

    if ( mPrevOriginalBPM > 0.0 && mPrevNewBPM > 0.0 )
    {
        mPrevTimeRatio = mPrevOriginalBPM / mPrevNewBPM;
    }
    else
    {
        mPrevTimeRatio = 1.0;
    }

    mSpinBoxOriginalBPM->setValue( mPrevOriginalBPM );
    mSpinBoxNewBPM->setValue( mPrevNewBPM );
    mCheckBoxPitchCorrection->setChecked( mPrevIsPitchCorrectionEnabled );

    mMainWindow->mAppliedOriginalBPM = mPrevOriginalBPM;
    mMainWindow->mAppliedNewBPM = mPrevNewBPM;
}



void ApplyTimeStretchCommand::redo()
{    
    const qreal timeRatio = mOriginalBPM / mNewBPM;
    const qreal pitchScale = mIsPitchCorrectionEnabled ? 1.0 : mNewBPM / mOriginalBPM;

    stretch( timeRatio, pitchScale );

    mSpinBoxOriginalBPM->setValue( mOriginalBPM );
    mSpinBoxNewBPM->setValue( mNewBPM );
    mCheckBoxPitchCorrection->setChecked( mIsPitchCorrectionEnabled );

    mMainWindow->mAppliedOriginalBPM = mOriginalBPM;
    mMainWindow->mAppliedNewBPM = mNewBPM;
}



void ApplyTimeStretchCommand::stretch( const qreal timeRatio, const qreal pitchScale )
{
    // If 'timeRatio' is 1.0 then simply reload the original audio file,
    // otherwise apply the time stretch

    if ( timeRatio == 1.0 )
    {
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        SharedSampleBuffer tempSampleBuffer = mMainWindow->mFileHandler.getSampleData( mMainWindow->mCurrentAudioFilePath );

        if ( ! tempSampleBuffer.isNull() )
        {
            const int numChans = mMainWindow->mCurrentSampleHeader->numChans;
            const int numFrames = tempSampleBuffer->getNumFrames();

            mMainWindow->mCurrentSampleBuffer->setSize( numChans, numFrames );

            for ( int chanNum = 0; chanNum < numChans; chanNum++ )
            {
                mMainWindow->mCurrentSampleBuffer->copyFrom( chanNum, 0, *tempSampleBuffer.data(), chanNum, 0, numFrames );
            }

            updateSampleRanges( timeRatio, numFrames );
            mMainWindow->resetSampler();
            mGraphicsView->forceRedraw();

            QApplication::restoreOverrideCursor();
        }
        else // Failed to read audio file
        {
            QApplication::restoreOverrideCursor();
            MainWindow::showWarningDialog( mMainWindow->mFileHandler.getLastErrorTitle(),
                                           mMainWindow->mFileHandler.getLastErrorInfo() );
        }
    }
    else
    {
        stretchImpl( timeRatio, pitchScale );
    }
}



void ApplyTimeStretchCommand::stretchImpl( const qreal timeRatio, const qreal pitchScale )
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const int sampleRate = mMainWindow->mCurrentSampleHeader->sampleRate;
    const int numChans = mMainWindow->mCurrentSampleHeader->numChans;
    const RubberBandStretcher::Options options = mMainWindow->mOptionsDialog->getStretcherOptions();

    RubberBandStretcher stretcher( sampleRate, numChans, options, timeRatio, pitchScale );

    // Get original, unmodified sample data
    SharedSampleBuffer tempSampleBuffer = mMainWindow->mFileHandler.getSampleData( mMainWindow->mCurrentAudioFilePath );

    if ( ! tempSampleBuffer.isNull() )
    {
        const int origNumFrames = tempSampleBuffer->getNumFrames();
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
            inFloatBuffer[ chanNum ] = tempSampleBuffer->getArrayOfChannels()[ chanNum ];
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

        updateSampleRanges( timeRatio, totalNumFramesRetrieved );
        mMainWindow->resetSampler();
        mGraphicsView->forceRedraw();

        delete[] inFloatBuffer;
        delete[] outFloatBuffer;

        QApplication::restoreOverrideCursor();
    }
    else // Failed to read audio file
    {
        QApplication::restoreOverrideCursor();
        MainWindow::showWarningDialog( mMainWindow->mFileHandler.getLastErrorTitle(),
                                       mMainWindow->mFileHandler.getLastErrorInfo() );
    }
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
            const int origStartFrame = roundToInt( range->startFrame / mPrevTimeRatio );
            const int newStartFrame = roundToInt( origStartFrame * timeRatio );
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

    // Reapply all audio modifications (gain, reverse, etc)
    foreach ( SharedSampleRange range, mMainWindow->mSampleRangeList )
    {
        if ( range->isReversed )
        {
            mMainWindow->mCurrentSampleBuffer->reverse( range->startFrame, range->numFrames );
        }
    }
}
