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
                  QPushButton* const findOnsetsButton,
                  QPushButton* const findBeatsButton,
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
    QPushButton* const mFindOnsetsButton;
    QPushButton* const mFindBeatsButton;
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



class DeleteWaveformItemCommand : public QUndoCommand
{
public:
    DeleteWaveformItemCommand( const QList<int> orderPositions,
                               WaveGraphicsView* const graphicsView,
                               MainWindow* const mainWindow,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const QList<int> mOrderPositions;
    WaveGraphicsView* const mGraphicsView;
    MainWindow* const mMainWindow;
    QList<SharedSampleBuffer> mRemovedSampleBuffers;
    QList<SharedWaveformItem> mRemovedWaveforms;
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
    QList<int> mSlicePoints;
};



//class SplitCommand : public QUndoCommand
//{
//public:
//    SplitCommand( const int orderPos,
//                  WaveGraphicsView* const graphicsView,
//                  MainWindow* const mainWindow,
//                  QUndoCommand* parent = NULL );
//
//    void undo();
//    void redo();
//
//private:
//    const int mJoinedItemOrderPos;
//    WaveGraphicsView* const mGraphicsView;
//    MainWindow* const mMainWindow;
//    QList<int> mOrderPositions;
//};



class ApplyGainCommand : public QUndoCommand
{
public:
    ApplyGainCommand( const float gain,
                      const int waveformItemOrderPos,
                      WaveGraphicsView* const graphicsView,
                      const int sampleRate,
                      AudioFileHandler& fileHandler,
                      const QString tempDirPath,
                      const QString fileBaseName,
                      QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const float mGain;
    const int mOrderPos;
    WaveGraphicsView* const mGraphicsView;
    const int mSampleRate;
    AudioFileHandler& mFileHandler;
    const QString mTempDirPath;
    const QString mFileBaseName;
    QString mFilePath;
};



class ApplyGainRampCommand : public QUndoCommand
{
public:
    ApplyGainRampCommand( const float startGain,
                          const float endGain,
                          const int waveformItemOrderPos,
                          WaveGraphicsView* const graphicsView,
                          const int sampleRate,
                          AudioFileHandler& fileHandler,
                          const QString tempDirPath,
                          const QString fileBaseName,
                          QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const float mStartGain;
    const float mEndGain;
    const int mOrderPos;
    WaveGraphicsView* const mGraphicsView;
    const int mSampleRate;
    AudioFileHandler& mFileHandler;
    const QString mTempDirPath;
    const QString mFileBaseName;
    QString mFilePath;
};



class NormaliseCommand : public QUndoCommand
{
public:
    NormaliseCommand( const int waveformItemOrderPos,
                      WaveGraphicsView* const graphicsView,
                      const int sampleRate,
                      AudioFileHandler& fileHandler,
                      const QString tempDirPath,
                      const QString fileBaseName,
                      QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int mOrderPos;
    WaveGraphicsView* const mGraphicsView;
    const int mSampleRate;
    AudioFileHandler& mFileHandler;
    const QString mTempDirPath;
    const QString mFileBaseName;
    QString mFilePath;
};



class ReverseCommand : public QUndoCommand
{
public:
    ReverseCommand( const int waveformItemOrderPos,
                    WaveGraphicsView* const graphicsView,
                    QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int mOrderPos;
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
                             const QString tempDirPath,
                             const QString fileBaseName,
                             QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    int stretch( const SharedSampleBuffer sampleBuffer, const qreal timeRatio, const qreal pitchScale );
    void updateSlicePoints( const qreal timeRatio );
    void updateLoopMarkers( const qreal timeRatio );

    MainWindow* const mMainWindow;
    WaveGraphicsView* const mGraphicsView;
    QDoubleSpinBox* const mSpinBoxOriginalBPM;
    QDoubleSpinBox* const mSpinBoxNewBPM;
    QCheckBox* const mCheckBoxPitchCorrection;
    const qreal mOriginalBPM;
    const qreal mNewBPM;
    const qreal mPrevAppliedBPM;
    const bool mIsPitchCorrectionEnabled;
    const RubberBandStretcher::Options mOptions;
    const QString mTempDirPath;
    const QString mFileBaseName;
    QStringList mTempFilePaths;
};


#endif // COMMANDS_H
