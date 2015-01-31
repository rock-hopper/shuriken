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
#include "wavegraphicsscene.h"
#include "wavegraphicsview.h"
#include "slicepointitem.h"
#include "mainwindow.h"
#include "optionsdialog.h"


class AddSlicePointItemCommand : public QUndoCommand
{
public:
    AddSlicePointItemCommand( int frameNum,
                              bool canBeMovedPastOtherSlicePoints,
                              WaveGraphicsScene* graphicsScene,
                              QPushButton* sliceButton = NULL,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    WaveGraphicsScene* const m_graphicsScene;
    QPushButton* const m_sliceButton;
    SharedSlicePointItem m_slicePointItem;
    bool m_isFirstRedoCall;
};



class MoveSlicePointItemCommand : public QUndoCommand
{
public:
    MoveSlicePointItemCommand( SharedSlicePointItem slicePoint,
                               int oldFrameNum,
                               WaveGraphicsScene* graphicsScene,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSlicePointItem m_slicePointItem;
    const int m_oldFrameNum;
    const int m_newFrameNum;
    WaveGraphicsScene* const m_graphicsScene;
    bool m_isFirstRedoCall;
};



class DeleteSlicePointItemCommand : public QUndoCommand
{
public:
    DeleteSlicePointItemCommand( SharedSlicePointItem slicePoint,
                                 WaveGraphicsScene* graphicsScene,
                                 QPushButton* sliceButton = NULL,
                                 QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const SharedSlicePointItem m_slicePointItem;
    WaveGraphicsScene* const m_graphicsScene;
    QPushButton* const m_sliceButton;
};



class SliceCommand : public QUndoCommand
{
public:
    SliceCommand( MainWindow* mainWindow,
                  WaveGraphicsScene* graphicsScene,
                  QPushButton* sliceButton,
                  QPushButton* findOnsetsButton,
                  QPushButton* findBeatsButton,
                  QAction* addSlicePointAction,
                  QAction* selectMoveItemsAction,
                  QAction* auditionItemsAction,
                  QAction* selectiveTimeStretchAction,
                  QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsScene* const m_graphicsScene;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
};



class UnsliceCommand : public QUndoCommand
{
public:
    UnsliceCommand( MainWindow* mainWindow,
                    WaveGraphicsScene* graphicsScene,
                    QPushButton* sliceButton,
                    QPushButton* findOnsetsButton,
                    QPushButton* findBeatsButton,
                    QAction* addSlicePointAction,
                    QAction* selectMoveItemsAction,
                    QAction* auditionItemsAction,
                    QAction* selectiveTimeStretchAction,
                    QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsScene* const m_graphicsScene;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
};



class EnableSelectiveTSCommand : public QUndoCommand
{
public:
    EnableSelectiveTSCommand( OptionsDialog* optionsDialog,
                              WaveGraphicsScene* graphicsScene,
                              QPushButton* sliceButton,
                              QAction* addSlicePointAction,
                              QAction* selectMoveItemsAction,
                              QAction* multiSelectItemsAction,
                              QAction* auditionItemsAction,
                              QAction* selectiveTimeStretchAction,
                              const QList<SharedSampleBuffer> sampleBufferList,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    OptionsDialog* const m_optionsDialog;
    WaveGraphicsScene* const m_graphicsScene;
    QPushButton* const m_sliceButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_multiSelectItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
    const QList<SharedSampleBuffer> m_sampleBufferList;
};



class DisableSelectiveTSCommand : public QUndoCommand
{
public:
    DisableSelectiveTSCommand( OptionsDialog* optionsDialog,
                               WaveGraphicsScene* graphicsScene,
                               QPushButton* sliceButton,
                               QAction* addSlicePointAction,
                               QAction* selectMoveItemsAction,
                               QAction* multiSelectItemsAction,
                               QAction* auditionItemsAction,
                               QAction* selectiveTimeStretchAction,
                               const QList<SharedSampleBuffer> sampleBufferList,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    OptionsDialog* const m_optionsDialog;
    WaveGraphicsScene* const m_graphicsScene;
    QPushButton* const m_sliceButton;
    QAction* const m_addSlicePointAction;
    QAction* const m_selectMoveItemsAction;
    QAction* const m_multiSelectItemsAction;
    QAction* const m_auditionItemsAction;
    QAction* const m_selectiveTimeStretchAction;
    const QList<SharedSampleBuffer> m_sampleBufferList;
};



class MoveWaveformItemCommand : public QUndoCommand
{
public:
    MoveWaveformItemCommand( QList<int> oldOrderPositions,
                             int numPlacesMoved,
                             WaveGraphicsScene* graphicsScene,
                             MainWindow* mainWindow,
                             QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    void reorderSampleBufferList( QList<int> oldOrderPositions, int numPlacesMoved );

    const QList<int> m_oldOrderPositions;
    const int m_numPlacesMoved;
    WaveGraphicsScene* const m_graphicsScene;
    MainWindow* const m_mainWindow;
    QList<int> m_newOrderPositions;
    bool m_isFirstRedoCall;
};



class DeleteWaveformItemCommand : public QUndoCommand
{
public:
    DeleteWaveformItemCommand( QList<int> orderPositions,
                               WaveGraphicsScene* graphicsScene,
                               MainWindow* mainWindow,
                               QPushButton* sliceButton,
                               QPushButton* findOnsetsButton,
                               QPushButton* findBeatsButton,
                               QAction* addSlicePointAction,
                               QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const QList<int> m_orderPositions;
    WaveGraphicsScene* const m_graphicsScene;
    MainWindow* const m_mainWindow;
    QPushButton* const m_sliceButton;
    QPushButton* const m_findOnsetsButton;
    QPushButton* const m_findBeatsButton;
    QAction* const m_addSlicePointAction;
    QList<SharedSampleBuffer> m_removedSampleBuffers;
    QList<SharedWaveformItem> m_removedWaveforms;
};



class ApplyGainCommand : public QUndoCommand
{
public:
    ApplyGainCommand( float gain,
                      int waveformItemOrderPos,
                      WaveGraphicsScene* graphicsScene,
                      WaveGraphicsView* graphicsView,
                      int sampleRate,
                      AudioFileHandler& fileHandler,
                      QString tempDirPath,
                      QString fileBaseName,
                      QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const float m_gain;
    const int m_orderPos;
    WaveGraphicsScene* const m_graphicsScene;
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
    ApplyGainRampCommand( float startGain,
                          float endGain,
                          int waveformItemOrderPos,
                          WaveGraphicsScene* graphicsScene,
                          WaveGraphicsView* graphicsView,
                          int sampleRate,
                          AudioFileHandler& fileHandler,
                          QString tempDirPath,
                          QString fileBaseName,
                          QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const float m_startGain;
    const float m_endGain;
    const int m_orderPos;
    WaveGraphicsScene* const m_graphicsScene;
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
    NormaliseCommand( int waveformItemOrderPos,
                      WaveGraphicsScene* graphicsScene,
                      WaveGraphicsView* graphicsView,
                      int sampleRate,
                      AudioFileHandler& fileHandler,
                      QString tempDirPath,
                      QString fileBaseName,
                      QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int m_orderPos;
    WaveGraphicsScene* const m_graphicsScene;
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
    ReverseCommand( int waveformItemOrderPos,
                    WaveGraphicsScene* graphicsScene,
                    WaveGraphicsView* graphicsView,
                    QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    const int mOrderPos;
    WaveGraphicsScene* const m_graphicsScene;
    WaveGraphicsView* const m_graphicsView;
};



class GlobalTimeStretchCommand : public QUndoCommand
{
public:
    GlobalTimeStretchCommand( MainWindow* mainWindow,
                              WaveGraphicsScene* graphicsScene,
                              WaveGraphicsView* graphicsView,
                              QDoubleSpinBox* spinBoxOriginalBPM,
                              QDoubleSpinBox* spinBoxNewBPM,
                              QCheckBox* checkBoxPitchCorrection,
                              QString tempDirPath,
                              QString fileBaseName,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    void updateSlicePoints( qreal timeRatio );
    void updateLoopMarkers( qreal timeRatio );

    MainWindow* const m_mainWindow;
    WaveGraphicsScene* const m_graphicsScene;
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



class RenderTimeStretchCommand : public QUndoCommand
{
public:
    RenderTimeStretchCommand( MainWindow* mainWindow,
                              WaveGraphicsScene* graphicsScene,
                              WaveGraphicsView* graphicsView,
                              QString tempDirPath,
                              QString fileBaseName,
                              QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsScene* const m_graphicsScene;
    WaveGraphicsView* const m_graphicsView;
    const RubberBandStretcher::Options m_options;
    const QString m_tempDirPath;
    const QString m_fileBaseName;
    QStringList m_tempFilePaths;
    QList<qreal> m_timeRatioList;
};



class SelectiveTimeStretchCommand : public QUndoCommand
{
public:
    SelectiveTimeStretchCommand( MainWindow* mainWindow,
                                 WaveGraphicsScene* graphicsScene,
                                 QList<int> orderPositions,
                                 QList<qreal> timeRatios,
                                 QList<int> midiNotes,
                                 QUndoCommand* parent = NULL );

    void undo();
    void redo();

private:
    MainWindow* const m_mainWindow;
    WaveGraphicsScene* const m_graphicsScene;
    const QList<int> m_orderPositions;
    const QList<qreal> m_origTimeRatios;
    const QList<qreal> m_timeRatios;
    const QList<int> m_midiNotes;
};


#endif // COMMANDS_H
