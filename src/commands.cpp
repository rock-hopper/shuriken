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
//#include <QDebug>


AddSlicePointItemCommand::AddSlicePointItemCommand( const qreal scenePosX,
                                                    WaveGraphicsView* const graphicsView,
                                                    QPushButton* const sliceButton,
                                                    QUndoCommand* parent )
    : QUndoCommand( parent ), mScenePosX( scenePosX ), mGraphicsView( graphicsView ), mSliceButton( sliceButton )
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
