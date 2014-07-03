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

#ifndef AUDIOSETUPDIALOG_H
#define AUDIOSETUPDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QAbstractButton>
#include <rubberband/RubberBandStretcher.h>
#include "JuceHeader.h"
#include "simplesynth.h"
#include "directoryvalidator.h"

using namespace RubberBand;


namespace Ui
{
    class AudioSetupDialog;
}

class AudioSetupDialog : public QDialog
{
    Q_OBJECT

public:
    enum Tab { AUDIO_SETUP = 0, TIMESTRETCH = 1 };
    
    AudioSetupDialog( AudioDeviceManager& deviceManager, QWidget* parent = NULL );
    ~AudioSetupDialog();

    void setCurrentTab( const Tab tab );

    bool isRealtimeModeEnabled() const;
    void enableRealtimeMode( const bool isEnabled );

    RubberBandStretcher::Options getStretcherOptions() const                    { return mStretcherOptions; }

    bool isJackSyncEnabled() const;

    // Returns the absolute path of the user-defined temp directory
    // if valid, otherwise returns an empty string
    QString getTempDirPath() const;

protected:
    void changeEvent( QEvent* event );
    void showEvent( QShowEvent* event );
    void closeEvent( QCloseEvent* event );

private:
    bool isJackAudioEnabled() const;
    void updateAudioDeviceComboBox();
    void updateOutputChannelComboBox();
    void updateSampleRateComboBox();
    void updateBufferSizeComboBox();
    void updateMidiInputListWidget();
    void disableAllWidgets();
    void setUpMidiInputTestSynth();
    void tearDownMidiInputTestSynth();
    void setJackMidiInput( const String deviceName );

    void enableStretcherOptions( const RubberBandStretcher::Options options );
    void disableStretcherOptions( const RubberBandStretcher::Options options );

    Ui::AudioSetupDialog* mUI;
    AudioDeviceManager& mDeviceManager;
    AudioDeviceManager::AudioDeviceSetup mOriginalConfig;
    ScopedPointer<SynthAudioSource> mSynthAudioSource;
    AudioSourcePlayer mAudioSourcePlayer;
    RubberBandStretcher::Options mStretcherOptions;

private:
    static void showWarningBox( const QString text, const QString infoText );
    static String getNameForChannelPair( const String& name1, const String& name2 );
    static QString getNoDeviceString() { return "<< " + tr("none") + " >>"; }

    ScopedPointer<DirectoryValidator> mDirectoryValidator;

signals:
    void realtimeModeToggled( const bool isEnabled );
    void stretchOptionChanged( const RubberBandStretcher::Options option );
    void transientsOptionChanged( const RubberBandStretcher::Options option );
    void phaseOptionChanged( const RubberBandStretcher::Options option );
    void windowOptionChanged();
    void formantOptionChanged( const RubberBandStretcher::Options option );
    void pitchOptionChanged( const RubberBandStretcher::Options option );
    void jackSyncToggled( const bool isEnabled );

private slots:
    void on_pushButton_ChooseTempDir_clicked();
    void on_checkBox_JackSync_toggled( const bool isChecked );
    void on_radioButton_HighConsistency_clicked();
    void on_radioButton_HighQuality_clicked();
    void on_radioButton_HighSpeed_clicked();
    void on_radioButton_Preserved_clicked();
    void on_radioButton_Shifted_clicked();
    void on_radioButton_Long_clicked();
    void on_radioButton_Short_clicked();
    void on_radioButton_Standard_clicked();
    void on_radioButton_Independent_clicked();
    void on_radioButton_Laminar_clicked();
    void on_radioButton_Smooth_clicked();
    void on_radioButton_Mixed_clicked();
    void on_radioButton_Crisp_clicked();
    void on_radioButton_Precise_clicked();
    void on_radioButton_Elastic_clicked();
    void on_radioButton_Offline_clicked();
    void on_radioButton_RealTime_clicked();
    void on_checkBox_MidiInputTestTone_clicked( const bool isChecked );
    void on_buttonBox_clicked( QAbstractButton* button );
    void on_listWidget_MidiInput_itemClicked( QListWidgetItem* item );
    void on_comboBox_BufferSize_activated( const int index );
    void on_comboBox_SampleRate_activated( const int index );
    void on_comboBox_OutputChannels_activated( const int index );
    void on_pushButton_TestTone_clicked();
    void on_comboBox_AudioDevice_activated( const QString deviceName );
    void on_comboBox_AudioBackend_currentIndexChanged( const int index );

    void accept();
    void reject();

    void displayDirValidityText( const bool isValid );

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioSetupDialog );
};

#endif // AUDIOSETUPDIALOG_H
