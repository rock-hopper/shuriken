/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

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
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    enum Tab { AUDIO_SETUP_TAB = 0, TIME_STRETCH_TAB = 1, PATHS_TAB = 2 };
    
    OptionsDialog( AudioDeviceManager& deviceManager, QWidget* parent = NULL );
    ~OptionsDialog();

    void setCurrentTab( Tab tab );
    void enableTab( Tab tab );
    void disableTab( Tab tab );

    void enableOfflineRealtimeButtons();
    void disableOfflineRealtimeButtons();

    RubberBandStretcher::Options getStretcherOptions() const    { return m_stretcherOptions; }
    void setStretcherOptions( RubberBandStretcher::Options options );

    bool isRealtimeModeEnabled() const;
    void enableRealtimeMode();

    bool isJackSyncEnabled() const;
    void enableJackSync();

    // Returns the absolute path of the user-defined temp directory if
    // it is valid and writable, otherwise returns an empty string
    QString getTempDirPath() const                              { return m_tempDirPath; }

protected:
    void changeEvent( QEvent* event );
    void showEvent( QShowEvent* event );
    void closeEvent( QCloseEvent* event );

private:
    void setTempDirPath();

    bool isJackAudioEnabled() const;
    void updateAudioDeviceComboBox();
    void updateOutputChannelComboBox();
    void updateSampleRateComboBox();
    void updateBufferSizeComboBox();
    void updateMidiInputListWidget();
    void disableAllWidgets();
    void setUpMidiInputTestSynth();
    void tearDownMidiInputTestSynth();
    void setJackMidiInput( String audioDeviceName );

    void enableStretcherOptions( RubberBandStretcher::Options options );
    void disableStretcherOptions( RubberBandStretcher::Options options );

    void saveConfig();

    Ui::OptionsDialog* m_ui;

    AudioDeviceManager& m_deviceManager;
    AudioDeviceManager::AudioDeviceSetup m_originalConfig;
    int m_originalBackendIndex;

    ScopedPointer<SynthAudioSource> m_synthAudioSource;
    AudioSourcePlayer m_audioSourcePlayer;

    RubberBandStretcher::Options m_stretcherOptions;

    ScopedPointer<DirectoryValidator> m_directoryValidator;

    QString m_tempDirPath;

private:
    static String getNameForChannelPair( const String& name1, const String& name2 );
    static QString getNoDeviceString() { return "<< " + tr("none") + " >>"; }
    static bool isJackMidiDevice( String midiDeviceName );

signals:
    void timeStretchOptionsChanged();
    void realtimeModeToggled( bool isEnabled );
    void stretchOptionChanged( RubberBandStretcher::Options option );
    void transientsOptionChanged( RubberBandStretcher::Options option );
    void phaseOptionChanged( RubberBandStretcher::Options option );
    void windowOptionChanged();
    void formantOptionChanged( RubberBandStretcher::Options option );
    void pitchOptionChanged( RubberBandStretcher::Options option );
    void jackSyncToggled( bool isEnabled );

private slots:
    void on_pushButton_ChooseTempDir_clicked();
    void on_checkBox_JackSync_toggled( bool isChecked );
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
    void on_checkBox_MidiInputTestTone_clicked( bool isChecked );
    void on_listWidget_MidiInput_itemClicked( QListWidgetItem* item );
    void on_comboBox_BufferSize_activated( int index );
    void on_comboBox_SampleRate_activated( int index );
    void on_comboBox_OutputChannels_activated( int index );
    void on_pushButton_TestTone_clicked();
    void on_comboBox_AudioDevice_activated( QString deviceName );
    void on_comboBox_AudioBackend_activated( int index );

    void accept();
    void reject();

    void displayDirValidityText( bool isValid );

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( OptionsDialog );
};

#endif // OPTIONSDIALOG_H
