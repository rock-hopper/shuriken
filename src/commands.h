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
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    SharedSlicePointItem m_slicePointItem;
    bool m_isFirstRedoCall;
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
    const SharedSlicePointItem m_slicePointItem;
    const int m_oldFrameNum;
    const int m_newFrameNum;
    WaveGraphicsView* const m_graphicsView;
    bool m_isFirstRedoCall;
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
    const SharedSlicePointItem m_slicePointItem;
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
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
    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_moveItemsAction;
    QAction* const m_selectItemsAction;
    QAction* const m_auditionItemsAction;
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
    const QList<int> m_oldOrderPositions;
    const int m_numPlacesMoved;
    WaveGraphicsView* const m_graphicsView;
    MainWindow* const m_mainWindow;
    QList<int> m_newOrderPositions;
    bool m_isFirstRedoCall;
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
    const QList<int> m_orderPositions;
    WaveGraphicsView* const m_graphicsView;
    MainWindow* const m_mainWindow;
    QList<SharedSampleBuffer> m_removedSampleBuffers;
    QList<SharedWaveformItem> m_removedWaveforms;
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
    const QList<int> m_orderPositions;
    WaveGraphicsView* const m_graphicsView;
    MainWindow* const m_mainWindow;
    int m_joinedItemOrderPos;
    QList<int> m_slicePoints;
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
    const float m_gain;
    const int m_orderPos;
    WaveGraphicsView* const m_graphicsView;
    const int m_sampleRate;
    AudioFileHandler& m_fileHandler;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QString m_filePath;
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
    const float m_startGain;
    const float m_endGain;
    const int m_orderPos;
    WaveGraphicsView* const m_graphicsView;
    const int m_sampleRate;
    AudioFileHandler& m_fileHandler;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QString m_filePath;
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
    const int m_orderPos;
    WaveGraphicsView* const m_graphicsView;
    const int m_sampleRate;
    AudioFileHandler& m_fileHandler;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QString m_filePath;
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
    WaveGraphicsView* const m_graphicsView;
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

    MainWindow* const m_mainWindow;
    WaveGraphicsView* const m_graphicsView;
    QDoubleSpinBox* const m_spinBoxOriginalBPM;
    QDoubleSpinBox* const m_spinBoxNewBPM;
    QCheckBox* const m_checkBoxPitchCorrection;
    const qreal m_originalBPM;
    const qreal m_newBPM;
    const qreal m_prevAppliedBPM;
    const bool m_isPitchCorrectionEnabled;
    const RubberBandStretcher::Options m_options;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QStringList m_tempFilePaths;
};


#endif // COMMANDS_H
