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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QUndoStack>
#include <aubio/aubio.h>
#include "JuceHeader.h"
#include "samplebuffer.h"
#include "audiosetupdialog.h"
#include "audiofilehandler.h"
#include "sampleraudiosource.h"
#include "slicepointitem.h"


namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class CreateSlicesCommand;

public:
    MainWindow( QWidget* parent = NULL );
    ~MainWindow();

protected:
    void changeEvent( QEvent* e );

private:
    struct DetectionSettings
    {
        QByteArray detectionMethod;
        smpl_t threshold;
        uint_t windowSize;
        uint_t hopSize;
        uint_t sampleRate;
    };

    DetectionSettings getDetectionSettings();
    QList<int> getAmendedSlicePointFrameNumList();
    void setUpSampler( const int numChans );
    void tearDownSampler();
    void enableUI();
    void disableUI();

    Ui::MainWindow* mUI; // "Go to slot..." in Qt Designer won't work if this is changed to ScopedPointer<Ui::MainWindow>

    ScopedPointer<AudioSetupDialog> mAudioSetupDialog;

    AudioDeviceManager mDeviceManager;
    AudioFileHandler mFileHandler;

    SharedSampleBuffer mCurrentSampleBuffer;
    SharedSampleHeader mCurrentSampleHeader;
    QList<SharedSampleBuffer> mSlicedSampleBuffers;

    ScopedPointer<SamplerAudioSource> mSamplerAudioSource;
    ScopedPointer<SoundTouchAudioSource> mSoundTouchAudioSource;
    AudioSourcePlayer mAudioSourcePlayer;

    bool mIsAudioInitialised;

    QString mLastOpenedImportDir;
    QString mLastOpenedProjDir;

    QUndoStack mUndoStack;

    int mSoundTouchBufferSize;

private:
    enum AubioRoutine { ONSET_DETECTION, BEAT_DETECTION };

    static void showWarningBox( const QString text, const QString infoText );

    static QList<int> calcSlicePointFrameNums( const SharedSampleBuffer sampleBuffer,
                                               const AubioRoutine routine,
                                               const DetectionSettings settings );

    static qreal calcBPM( const SharedSampleBuffer sampleBuffer,
                          const DetectionSettings settings );

    static void fillAubioInputBuffer( fvec_t* pInputBuffer,
                                      const SharedSampleBuffer sampleBuffer,
                                      const int sampleOffset );

    static void createSampleSlices( const SharedSampleBuffer inputBuffer,
                                    const QList<int> slicePointFrameNumList,
                                    QList<SharedSampleBuffer>& outputBufferList );

    static const qreal MIN_INTER_ONSET_SECS = 0.03;

private slots:
    void on_actionZoom_Original_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_pushButton_Stop_clicked();
    void on_pushButton_Play_clicked();
    void on_checkBox_PitchCorrection_toggled( const bool isChecked );
    void on_checkBox_TimeStretch_toggled( const bool isChecked );
    void on_doubleSpinBox_NewBPM_valueChanged( const double newBPM );
    void on_pushButton_CalcBPM_clicked();
    void on_pushButton_FindBeats_clicked();
    void on_checkBox_AdvancedOptions_toggled( const bool isChecked );
    void on_horizontalSlider_Threshold_valueChanged( const int value );
    void on_pushButton_FindOnsets_clicked();
    void on_pushButton_Slice_clicked();
    void on_actionApply_Ramp_triggered();
    void on_actionNormalise_triggered();
    void on_actionApply_Gain_triggered();
    void on_actionClear_Selection_triggered();
    void on_actionSelect_All_triggered();
    void on_actionJoin_triggered();
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
    void on_actionUser_Interface_triggered();
    void on_actionAudio_Setup_triggered();
    void on_actionEnvelope_triggered();
    void on_actionReverse_triggered();
    void on_actionDelete_triggered();
    void on_actionAdd_Slice_Point_triggered();
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
    void on_actionQuit_triggered();
    void on_actionExport_As_triggered();
    void on_actionImport_Audio_File_triggered();
    void on_actionClose_Project_triggered();
    void on_actionSave_Project_triggered();
    void on_actionOpen_Project_triggered();

    void reorderSampleBufferList( const int oldOrderPos, const int newOrderPos );
    void recordWaveformItemMove( const int startOrderPos, const int destOrderPos );
    void recordSlicePointItemMove( const SharedSlicePointItem slicePointItem,
                                   const int oldFrameNum,
                                   const int newFrameNum );
    void playSample( const int sampleNum, const int startFrame, const int endFrame );
    void disableZoomIn();
    void disableZoomOut();
};

#endif // MAINWINDOW_H
