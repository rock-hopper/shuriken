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
//#include <QDebug>


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

DeleteSlicePointItemCommand::DeleteSlicePointItemCommand( SharedSlicePointItem slicePoint,
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

CreateSlicesCommand::CreateSlicesCommand( MainWindow* const mainWindow,
                                          WaveGraphicsView* const graphicsView,
                                          QPushButton* const sliceButton,
                                          QAction* const addSlicePointAction,
                                          QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mMainWindow( mainWindow ),
    mGraphicsView( graphicsView ),
    mSliceButton( sliceButton ),
    mAddSlicePointAction( addSlicePointAction )
{
    setText( "Create Slices" );
}



void CreateSlicesCommand::undo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->mSamplerAudioSource->setSample( mMainWindow->mCurrentSampleBuffer,
                                                 mMainWindow->mCurrentSampleHeader->sampleRate );
    mMainWindow->mSampleRangeList.clear();

    mGraphicsView->clearWaveform();

    SharedWaveformItem item = mGraphicsView->createWaveformItem( mMainWindow->mCurrentSampleBuffer );

    QObject::connect( item.data(), SIGNAL( rightMousePressed(int,int,QPointF) ),
                      mMainWindow, SLOT( playSampleRange(int,int,QPointF) ) );

    mGraphicsView->showSlicePoints();

    mSliceButton->setEnabled( true );
    mAddSlicePointAction->setEnabled( true );

    QApplication::restoreOverrideCursor();
}



void CreateSlicesCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    mMainWindow->getSampleRanges( mMainWindow->mSampleRangeList );

    Q_ASSERT( mMainWindow->mSampleRangeList.size() > 0 );

    mMainWindow->mSamplerAudioSource->setSampleRanges( mMainWindow->mSampleRangeList );

    mGraphicsView->hideSlicePoints();
    mGraphicsView->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList =
            mGraphicsView->createWaveformItems( mMainWindow->mCurrentSampleBuffer,
                                                mMainWindow->mSampleRangeList );

    foreach ( SharedWaveformItem item, waveformItemList )
    {
        QObject::connect( item.data(), SIGNAL( orderPosHasChanged(int,int) ),
                          mMainWindow, SLOT( recordWaveformItemMove(int,int) ) );

        QObject::connect( item.data(), SIGNAL( orderPosHasChanged(int,int) ),
                          mMainWindow, SLOT( reorderSampleRangeList(int,int) ) );

        QObject::connect( item.data(), SIGNAL( rightMousePressed(int,int,QPointF) ),
                          mMainWindow, SLOT( playSampleRange(int,int,QPointF) ) );
    }

    mSliceButton->setEnabled( false );
    mAddSlicePointAction->setEnabled( false );

    QApplication::restoreOverrideCursor();
}



//==================================================================================================

MoveWaveformItemCommand::MoveWaveformItemCommand( const int startOrderPos,
                                                  const int destOrderPos,
                                                  WaveGraphicsView* const graphicsView,
                                                  QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mStartOrderPos( startOrderPos ),
    mDestOrderPos( destOrderPos ),
    mGraphicsView( graphicsView )
{
    setText( "Move Waveform Item" );
    mIsFirstRedoCall = true;
}



void MoveWaveformItemCommand::undo()
{
    mGraphicsView->moveWaveformItem( mDestOrderPos, mStartOrderPos );
}



void MoveWaveformItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        mGraphicsView->moveWaveformItem( mStartOrderPos, mDestOrderPos );
    }
    mIsFirstRedoCall = false;
}



//==================================================================================================

ApplyTimeStretchCommand::ApplyTimeStretchCommand( QUndoCommand* parent ) :
    QUndoCommand( parent )
{
    ;
}



void ApplyTimeStretchCommand::undo()
{
    ;
}



void ApplyTimeStretchCommand::redo()
{
    ;
}
