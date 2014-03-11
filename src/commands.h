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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QPushButton>
#include "wavegraphicsview.h"
#include "slicepointitem.h"


class AddSlicePointItemCommand : public QUndoCommand
{
public:
    AddSlicePointItemCommand( const qreal scenePosX, WaveGraphicsView* const graphicsView, QPushButton* const sliceButton,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const qreal mScenePosX;
    WaveGraphicsView* const mGraphicsView;
    QPushButton* const mSliceButton;
    SharedSlicePointItem mSlicePointItem;
};


class AddSlicePointItemsCommand : public QUndoCommand
{
public:
    AddSlicePointItemsCommand( QPushButton* const findOnsetsButton, QPushButton* const findBeatsButton,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    QPushButton* const mFindOnsetsButton;
    QPushButton* const mFindBeatsButton;
};

#endif // COMMANDS_H
