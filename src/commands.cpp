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


AddSlicePointItemCommand::AddSlicePointItemCommand( const qreal scenePosX,
                                                    WaveGraphicsView* const graphicsView,
                                                    QPushButton* const sliceButton,
                                                    MainWindow* const mainWindow,
                                                    QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mScenePosX( scenePosX ),
    mGraphicsView( graphicsView ),
    mSliceButton( sliceButton ),
    mMainWindow( mainWindow )
{
    setText( "Add Slice Point" );
}



void AddSlicePointItemCommand::undo()
{
    mGraphicsView->deleteSlicePoint( mSlicePointItem );
    mSlicePointItem.clear();

    if ( mGraphicsView->getSlicePointScenePosList().isEmpty() )
    {
        mSliceButton->setEnabled( false );
    }
}



void AddSlicePointItemCommand::redo()
{
    mSlicePointItem = mGraphicsView->createSlicePoint( mScenePosX );

    QObject::connect( mSlicePointItem.data(), SIGNAL( scenePosChanged(qreal,qreal) ),
                      mMainWindow, SLOT( recordSlicePointScenePos(qreal,qreal) ) );

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

MoveSlicePointItemCommand::MoveSlicePointItemCommand( const qreal oldScenePosX,
                                                      const qreal newScenePosX,
                                                      WaveGraphicsView* const graphicsView,
                                                      QUndoCommand* parent ) :
    QUndoCommand( parent ),
    mOldScenePosX( oldScenePosX ),
    mNewScenePosX( newScenePosX ),
    mGraphicsView( graphicsView )
{
    setText( "Move Slice Point" );
    mIsFirstRedoCall = true;
}



void MoveSlicePointItemCommand::undo()
{
    const QPoint viewCoords = mGraphicsView->mapFromScene( mNewScenePosX, 0.0 );
    QGraphicsItem* const item = mGraphicsView->itemAt( viewCoords );

    dynamic_cast<SlicePointItem*>( item )->setPos( mOldScenePosX, 0.0 );
}



void MoveSlicePointItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        const QPoint viewCoords = mGraphicsView->mapFromScene( mOldScenePosX, 0.0 );
        QGraphicsItem* const item = mGraphicsView->itemAt( viewCoords );

        dynamic_cast<SlicePointItem*>( item )->setPos( mNewScenePosX, 0.0 );
    }
    mIsFirstRedoCall = false;
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

    mMainWindow->mSlicedSampleBuffers.clear();
    mMainWindow->mSamplerAudioSource->clearAllSamples();
    mMainWindow->mSamplerAudioSource->addNewSample( mMainWindow->mCurrentSampleBuffer,
                                                    mMainWindow->mCurrentSampleHeader->sampleRate );
    mGraphicsView->clearWaveform();
    mGraphicsView->createWaveform( mMainWindow->mCurrentSampleBuffer );
    mGraphicsView->showSlicePoints();
    mSliceButton->setEnabled( true );
    mAddSlicePointAction->setEnabled( true );

    QApplication::restoreOverrideCursor();
}



void CreateSlicesCommand::redo()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const QList<int> sampleSlicePoints = mMainWindow->getCurrentSampleSlicePoints();

    Q_ASSERT_X( sampleSlicePoints.size() > 0, "CreateSlicesCommand::redo", "No sample slice points" );

    MainWindow::createSampleSlices( mMainWindow->mCurrentSampleBuffer,
                                    sampleSlicePoints,
                                    mMainWindow->mSlicedSampleBuffers );

    mMainWindow->mSamplerAudioSource->setSamples( mMainWindow->mSlicedSampleBuffers,
                                                  mMainWindow->mCurrentSampleHeader->sampleRate );

    mGraphicsView->hideSlicePoints();
    mGraphicsView->clearWaveform();

    const QList<SharedWaveformItem> waveformItemList =
            mGraphicsView->createWaveformSlices( mMainWindow->mSlicedSampleBuffers );

    foreach ( SharedWaveformItem item, waveformItemList )
    {
        QObject::connect( item.data(), SIGNAL( orderPosHasChanged(int,int) ),
                          mMainWindow, SLOT( recordWaveformItemNewOrderPos(int,int) ) );
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
    mGraphicsView->moveWaveformSlice( mDestOrderPos, mStartOrderPos );
}



void MoveWaveformItemCommand::redo()
{
    if ( ! mIsFirstRedoCall )
    {
        mGraphicsView->moveWaveformSlice( mStartOrderPos, mDestOrderPos );
    }
    mIsFirstRedoCall = false;
}
