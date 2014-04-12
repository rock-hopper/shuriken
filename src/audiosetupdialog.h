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
#include "JuceHeader.h"


class SynthAudioSource;

namespace Ui
{
    class AudioSetupDialog;
}

class AudioSetupDialog : public QDialog
{
    Q_OBJECT

public:
    AudioSetupDialog( AudioDeviceManager& deviceManager, QWidget* parent = NULL );
    ~AudioSetupDialog();

protected:
    void changeEvent( QEvent* event );
    void showEvent( QShowEvent* event );
    void closeEvent( QCloseEvent* event );

private:
    void updateAudioDeviceComboBox();
    void updateOutputChannelComboBox();
    void updateSampleRateComboBox();
    void updateBufferSizeComboBox();
    void updateMidiInputListWidget();
    void disableAllWidgets();
    void tearDownMidiInputTestSynth();
    void setJackMidiInput( const String deviceName );

    Ui::AudioSetupDialog* mUI;
    AudioDeviceManager& mDeviceManager;
    ScopedPointer<SynthAudioSource> mSynthAudioSource;
    AudioSourcePlayer mAudioSourcePlayer;

private:
    static void showWarningBox( const QString text, const QString infoText );
    static String getNameForChannelPair( const String& name1, const String& name2 );
    static QString getNoDeviceString() { return "<< " + tr("none") + " >>"; }

private slots:
    void on_listWidget_MidiInput_itemClicked( QListWidgetItem* item );
    void on_comboBox_BufferSize_activated( const int index );
    void on_comboBox_SampleRate_activated( const int index );
    void on_comboBox_OutputChannels_activated( const int index );
    void on_pushButton_TestTone_clicked();
    void on_comboBox_AudioDevice_activated( const QString deviceName );
    void on_comboBox_AudioBackend_currentIndexChanged( const int index );

    void accept();
    void reject();
};

#endif // AUDIOSETUPDIALOG_H
