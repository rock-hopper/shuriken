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

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QShowEvent>
#include "JuceHeader.h"
#include "directoryvalidator.h"
#include <sndfile.h>


namespace Ui
{
    class ExportDialog;
}


class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    enum NumberingStyle { NUMBERING_PREFIX, NUMBERING_SUFFIX };

    enum ExportType
    {
        EXPORT_AUDIO_FILES = 0x01,
        EXPORT_H2DRUMKIT   = 0x02,
        EXPORT_SFZ         = 0x04,
        EXPORT_AKAI_PGM    = 0x08,
        EXPORT_MIDI_FILE   = 0x10
    };

    enum SampleRate { SAMPLE_RATE_KEEP_SAME = 0 };

    ExportDialog( QWidget* parent = NULL );
    ~ExportDialog();

    QString getOutputDirPath() const;

    int getExportType() const;
    int getMidiFileType() const;

    QString getFileName() const;
    NumberingStyle getNumberingStyle() const;
    bool isOverwriteEnabled() const;

    int getSndFileFormat() const;
    int getSampleRate() const;
    int getAkaiModelID() const;

protected:
    void changeEvent( QEvent* event );
    void showEvent( QShowEvent* event );

private:
    void setPlatformFileNameValidator();
    void enableMidiFileTypeRadioButtons();
    void disableMidiFileTypeRadioButtons();

    Ui::ExportDialog* m_ui;

    QString m_lastOpenedExportDir;

    ScopedPointer<DirectoryValidator> m_directoryValidator;

private slots:
    void on_comboBox_MidiFile_activated( QString text );
    void on_radioButton_Akai_clicked();
    void on_radioButton_SFZ_clicked();
    void on_radioButton_H2Drumkit_clicked();
    void on_radioButton_AudioFiles_clicked();
    void on_comboBox_Format_currentIndexChanged( QString text );
    void on_pushButton_Create_clicked();
    void on_pushButton_Choose_clicked();

    void displayDirValidityText( bool isValid );
    void enableOkButtonIfInputValid();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( ExportDialog );
};

#endif // EXPORTDIALOG_H
