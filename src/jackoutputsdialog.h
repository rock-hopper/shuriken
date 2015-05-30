/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#ifndef JACKOUTPUTSDIALOG_H
#define JACKOUTPUTSDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include "JuceHeader.h"


namespace Ui
{
    class JackOutputsDialog;
}


class JackOutputsDialog : public QDialog
{
    Q_OBJECT

public:
    JackOutputsDialog( int numSampleBuffers, int numSampleChans, AudioDeviceManager& deviceManager, QWidget* parent = NULL );
    ~JackOutputsDialog();

protected:
    void changeEvent( QEvent* event );

private:
    const int m_numSampleChans;

    Ui::JackOutputsDialog* m_ui;

    AudioDeviceManager& m_deviceManager;

private slots:
    void on_spinBox_NumOutputs_valueChanged( int value );
    void on_tableWidget_itemClicked( QTableWidgetItem* item );
};


#endif // JACKOUTPUTSDIALOG_H
