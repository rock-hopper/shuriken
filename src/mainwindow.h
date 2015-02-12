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
#include <QActionGroup>
#include "JuceHeader.h"
#include "samplebuffer.h"
#include "optionsdialog.h"
#include "audiofilehandler.h"
#include "sampleraudiosource.h"
#include "rubberbandaudiosource.h"
#include "wavegraphicsscene.h"
#include "slicepointitem.h"
#include "audioanalyser.h"
#include "waveformitem.h"
#include "helpform.h"
#include "exportdialog.h"



namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class DeleteWaveformItemCommand;
    friend class MoveWaveformItemCommand;
    friend class SliceCommand;
    friend class UnsliceCommand;
    friend class GlobalTimeStretchCommand;
    friend class RenderTimeStretchCommand;
    friend class SelectiveTimeStretchCommand;

public:
    MainWindow( QWidget* parent = NULL );
    ~MainWindow();

    void openProject( QString filePath );

protected:
    void changeEvent( QEvent* event );
    void closeEvent( QCloseEvent* event );
    void keyPressEvent( QKeyEvent* event );

private:
    void initialiseAudio();
    void setUpSampler();
    void tearDownSampler();

    void setupUI();
    void enableUI();
    void disableUI();

    void connectWaveformToMainWindow( SharedWaveformItem item );

    void getDetectionSettings( AudioAnalyser::DetectionSettings& settings );

    void closeProject();

    void exportAs( QString tempDirPath,
                   QString outputDirPath,
                   QString samplesDirPath,
                   QString fileName,
                   int exportType,
                   int sndFileFormat,
                   int outputSampleRate,
                   int numSamplesToExport );

    void saveProject( QString filePath );

    void saveProjectDialog();
    void openProjectDialog();
    void importAudioFileDialog();
    void exportAsDialog();

    bool isSelectiveTimeStretchInUse() const;
    void renderTimeStretch();

    Ui::MainWindow* m_ui; // "Go to slot..." in Qt Designer won't work if this is changed to ScopedPointer<Ui::MainWindow>
    WaveGraphicsScene* m_scene;
    QActionGroup* m_interactionGroup;

    enum LengthUnits { UNITS_BARS, UNITS_BEATS };

    ScopedPointer<OptionsDialog> m_optionsDialog;
    ScopedPointer<HelpForm> m_helpForm;
    ScopedPointer<ExportDialog> m_exportDialog;

    AudioDeviceManager m_deviceManager;
    AudioFileHandler m_fileHandler;

    SharedSampleHeader m_sampleHeader;
    QList<SharedSampleBuffer> m_sampleBufferList;

    ScopedPointer<SamplerAudioSource> m_samplerAudioSource;
    ScopedPointer<RubberbandAudioSource> m_rubberbandAudioSource;
    AudioSourcePlayer m_audioSourcePlayer;

    QString m_lastOpenedImportDir;
    QString m_lastOpenedProjDir;
    QString m_currentProjectFilePath;

    QUndoStack m_undoStack;

    qreal m_appliedBPM;

    bool m_isProjectOpen;

private slots:
    void on_comboBox_SnapValues_activated( int index );
    void on_comboBox_TimeSigNumerator_activated( QString text );
    void on_pushButton_TimestretchOptions_clicked();
    void on_actionSelective_Time_Stretch_triggered( bool isChecked );
    void on_actionAudition_triggered();
    void on_actionMulti_Select_triggered();
    void on_actionSelect_Move_triggered();
    void on_pushButton_Apply_clicked();
    void on_actionZoom_Original_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_pushButton_Loop_clicked( bool isChecked );
    void on_pushButton_PlayStop_clicked();
    void on_checkBox_PitchCorrection_toggled( bool isChecked );
    void on_checkBox_TimeStretch_toggled( bool isChecked );
    void on_doubleSpinBox_NewBPM_valueChanged( double newBPM );
    void on_doubleSpinBox_OriginalBPM_valueChanged( double originalBPM );
    void on_pushButton_CalcBPM_clicked();
    void on_pushButton_FindBeats_clicked();
    void on_horizontalSlider_Threshold_valueChanged( int value );
    void on_pushButton_FindOnsets_clicked();
    void on_pushButton_Slice_clicked( bool isChecked );
    void on_actionNormalise_triggered();
    void on_actionApply_Gain_Ramp_triggered();
    void on_actionApply_Gain_triggered();
    void on_actionSelect_None_triggered();
    void on_actionSelect_All_triggered();
    void on_actionAbout_triggered();
    void on_actionHelp_triggered();
    void on_actionOptions_triggered();
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
    void on_actionSave_As_triggered();
    void on_actionSave_Project_triggered();
    void on_actionOpen_Project_triggered();

    void recordWaveformItemMove( QList<int> oldOrderPositions, int numPlacesMoved );

    void recordSlicePointItemMove( SharedSlicePointItem slicePoint,
                                   int orderPos,
                                   int numFramesFromPrevSlicePoint,
                                   int numFramesToNextSlicePoint,
                                   int oldFrameNum );

    void playSampleRange( const WaveformItem* waveformItem, QPointF mouseScenePos );

    void stopPlayback();

    void resetPlayStopButtonIcon();

    void disableZoomIn();
    void disableZoomOut();

    void enableRealtimeControls( bool isEnabled );
    void resetSampler();

    void enableEditActions();
    void enableSaveAction();

    void updateUndoText( QString text );
    void updateRedoText( QString text );

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( MainWindow );
};

#endif // MAINWINDOW_H
