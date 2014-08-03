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
    enum NumberingStyle { PREFIX, SUFFIX };

    ExportDialog( QWidget* parent = NULL );
    ~ExportDialog();

    QString getOutputDirPath() const;
    QString getFileBaseName() const;
    NumberingStyle getNumberingStyle() const;
    bool isOverwritingEnabled() const;

    bool isFormatSFZ() const;
    bool isFormatH2Drumkit() const;

    int getSndFileFormat() const;

protected:
    void changeEvent( QEvent* event );
    void showEvent( QShowEvent* event );

private:
    void updateEncodingComboBox( const QString format );

private:
    Ui::ExportDialog* mUI;

    QString mLastOpenedExportDir;

    ScopedPointer<DirectoryValidator> mDirectoryValidator;

private slots:
    void on_comboBox_AudioFiles_activated( const QString text );
    void on_comboBox_Format_currentIndexChanged( const QString text );
    void on_pushButton_Create_clicked();
    void on_pushButton_Choose_clicked();

    void displayDirValidityText( const bool isValid );
    void enableOkButtonIfInputValid();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( ExportDialog );
};

#endif // EXPORTDIALOG_H
