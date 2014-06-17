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
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "wavegraphicsview.h"
#include "slicepointitem.h"
#include "mainwindow.h"


class AddSlicePointItemCommand : public QUndoCommand
{
public:
    AddSlicePointItemCommand( const int frameNum,
                              WaveGraphicsView* const graphicsView,
                              QPushButton* const sliceButton,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    WaveGraphicsView* const mGraphicsView;
    QPushButton* const mSliceButton;
    SharedSlicePointItem mSlicePointItem;
    bool mIsFirstRedoCall;
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
    MoveSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                               const int oldFrameNum,
                               const int newFrameNum,
                               WaveGraphicsView* const graphicsView,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSlicePointItem mSlicePointItem;
    const int mOldFrameNum;
    const int mNewFrameNum;
    WaveGraphicsView* const mGraphicsView;
    bool mIsFirstRedoCall;
};



class DeleteSlicePointItemCommand : public QUndoCommand
{
public:
    DeleteSlicePointItemCommand( const SharedSlicePointItem slicePoint,
                                 WaveGraphicsView* const graphicsView,
                                 QPushButton* const sliceButton,
                                 QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSlicePointItem mSlicePointItem;
    WaveGraphicsView* const mGraphicsView;
    QPushButton* const mSliceButton;
};



class SliceCommand : public QUndoCommand
{
public:
    SliceCommand( MainWindow* const mainWindow,
                  WaveGraphicsView* const graphicsView,
                  QPushButton* const sliceButton,
                  QAction* const addSlicePointAction,
                  QAction* const moveItemsAction,
                  QAction* const selectItemsAction,
                  QAction* const auditionItemsAction,
                  QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const mMainWindow;
    WaveGraphicsView* const mGraphicsView;
    QPushButton* const mSliceButton;
    QAction* const mAddSlicePointAction;
    QAction* const mMoveItemsAction;
    QAction* const mSelectItemsAction;
    QAction* const mAuditionItemsAction;
};



class MoveWaveformItemCommand : public QUndoCommand
{
public:
    MoveWaveformItemCommand( const QList<int> oldOrderPositions,
                             const int numPlacesMoved,
                             WaveGraphicsView* const graphicsView,
                             MainWindow* const mainWindow,
                             QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const QList<int> mOldOrderPositions;
    const int mNumPlacesMoved;
    WaveGraphicsView* const mGraphicsView;
    MainWindow* const mMainWindow;
    QList<int> mNewOrderPositions;
    bool mIsFirstRedoCall;
};



class JoinCommand : public QUndoCommand
{
public:
    JoinCommand( const QList<int> orderPositions,
                 WaveGraphicsView* const graphicsView,
                 MainWindow* const mainWindow,
                 QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const QList<int> mOrderPositions;
    WaveGraphicsView* const mGraphicsView;
    MainWindow* const mMainWindow;
    int mJoinedItemOrderPos;
};



class SplitCommand : public QUndoCommand
{
public:
    SplitCommand( const int orderPos,
                  WaveGraphicsView* const graphicsView,
                  MainWindow* const mainWindow,
                  QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int mJoinedItemOrderPos;
    WaveGraphicsView* const mGraphicsView;
    MainWindow* const mMainWindow;
    QList<int> mOrderPositions;
};



class ReverseCommand : public QUndoCommand
{
public:
    ReverseCommand( const SharedSampleBuffer sampleBuffer,
                    const int waveformItemOrderPos,
                    WaveGraphicsView* const graphicsView,
                    QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSampleBuffer mSampleBuffer;
    const int mWaveformItemOrderPos;
    WaveGraphicsView* const mGraphicsView;
};



class ApplyTimeStretchCommand : public QUndoCommand
{
public:
    ApplyTimeStretchCommand( MainWindow* const mainWindow,
                             WaveGraphicsView* const graphicsView,
                             QDoubleSpinBox* const spinBoxOriginalBPM,
                             QDoubleSpinBox* const spinBoxNewBPM,
                             QCheckBox* const checkBoxPitchCorrection,
                             QUndoCommand* parent = NULL );

    void undo();
    void redo();

    qreal getOriginalBPM() const            { return mOriginalBPM; }
    qreal getNewBPM() const                 { return mNewBPM; }
    bool isPitchCorrectionEnabled() const   { return mIsPitchCorrectionEnabled; }

private:
    void stretch( const qreal timeRatio, const qreal pitchScale );
    void applyTimeStretch( const qreal timeRatio, const qreal pitchScale );
    void updateAll( const qreal timeRatio, const int newTotalNumFrames );

    MainWindow* const mMainWindow;
    WaveGraphicsView* const mGraphicsView;
    QDoubleSpinBox* const mSpinBoxOriginalBPM;
    QDoubleSpinBox* const mSpinBoxNewBPM;
    QCheckBox* const mCheckBoxPitchCorrection;
    const qreal mOriginalBPM;
    const qreal mNewBPM;
    const bool mIsPitchCorrectionEnabled;
    qreal mPrevOriginalBPM;
    qreal mPrevNewBPM;
    bool mPrevIsPitchCorrectionEnabled;
    qreal mPrevTimeRatio;
};


#endif // COMMANDS_H
