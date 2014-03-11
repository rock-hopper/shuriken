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

    ScopedPointer<Ui::AudioSetupDialog> mUI;
    AudioDeviceManager& mDeviceManager;
    ScopedPointer<SynthAudioSource> mSynthAudioSource;
    AudioSourcePlayer mAudioSourcePlayer;

private:
    static void showWarningBox( const QString text, const QString infoText );
    static String getNameForChannelPair( const String& name1, const String& name2 );
    static QString getNoDeviceName() { return "<< " + tr("none") + " >>"; }

private slots:
    void setAudioBackend( const int currentIndex );
    void setAudioDevice( const int currentIndex );
    void setOutputChannel( const int currentIndex );
    void setSampleRate( const int currentIndex );
    void setBufferSize( const int currentIndex );
    void enableMidiInput( QListWidgetItem* listItem );
    void playTestSound();
    void accept();
    void reject();
};

#endif // AUDIOSETUPDIALOG_H
