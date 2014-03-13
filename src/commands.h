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
#include <QAction>
#include "wavegraphicsview.h"
#include "slicepointitem.h"
#include "mainwindow.h"


class AddSlicePointItemCommand : public QUndoCommand
{
public:
    AddSlicePointItemCommand( const qreal scenePosX,
                              WaveGraphicsView* const graphicsView,
                              QPushButton* const sliceButton,
                              MainWindow* const mainWindow,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const qreal mScenePosX;
    WaveGraphicsView* const mGraphicsView;
    QPushButton* const mSliceButton;
    MainWindow* const mMainWindow;
    SharedSlicePointItem mSlicePointItem;
};



class AddSlicePointItemsCommand : public QUndoCommand
{
public:
    AddSlicePointItemsCommand( QPushButton* const findOnsetsButton,
                               QPushButton* const findBeatsButton,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    QPushButton* const mFindOnsetsButton;
    QPushButton* const mFindBeatsButton;
};



class MoveSlicePointItemCommand : public QUndoCommand
{
public:
    MoveSlicePointItemCommand( const qreal oldScenePosX,
                               const qreal newScenePosX,
                               WaveGraphicsView* const graphicsView,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const qreal mOldScenePosX;
    const qreal mNewScenePosX;
    WaveGraphicsView* const mGraphicsView;
    bool mIsFirstRedoCall;
};



class CreateSlicesCommand : public QUndoCommand
{
public:
    CreateSlicesCommand( MainWindow* const mainWindow,
                         WaveGraphicsView* const graphicsView,
                         QPushButton* const sliceButton,
                         QAction* const addSlicePointAction,
                         QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const mMainWindow;
    WaveGraphicsView* const mGraphicsView;
    QPushButton* const mSliceButton;
    QAction* const mAddSlicePointAction;
};

#endif // COMMANDS_H
