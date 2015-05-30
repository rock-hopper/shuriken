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

#include "jackoutputsdialog.h"
#include "ui_jackoutputsdialog.h"
#include <QtDebug>
#include "globals.h"
#include "messageboxes.h"


JackOutputsDialog::JackOutputsDialog( const int numSampleBuffers, const int numSampleChans, AudioDeviceManager& deviceManager, QWidget* parent ) :
    QDialog( parent ),
    m_numSampleChans( numSampleChans ),
    m_ui( new Ui::JackOutputsDialog ),
    m_deviceManager( deviceManager )
{
    Q_ASSERT( numSampleBuffers > 0 );
    Q_ASSERT( m_numSampleChans > 0 );

    m_ui->setupUi( this );

    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    const int numOutputs = config.outputChannels.countNumberOfSetBits();

    // Set up table widget
    m_ui->tableWidget->setRowCount( numOutputs / 2 );
    m_ui->tableWidget->setColumnCount( numSampleBuffers );

    QStringList verticalHeaders;

    for ( int row = 0; row < m_ui->tableWidget->rowCount(); row++ )
    {
        verticalHeaders << tr("Out") + (row > 0 ? " " + QString::number(row + 1) : "") + " L + R";

        for ( int column = 0; column < m_ui->tableWidget->columnCount(); column++ )
        {
            QTableWidgetItem* newItem = new QTableWidgetItem();
            newItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            newItem->setCheckState( Qt::Unchecked );
            m_ui->tableWidget->setItem( row, column, newItem );
        }
    }

    m_ui->tableWidget->setVerticalHeaderLabels( verticalHeaders );

    // Set up "No. of Outputs" spinbox
    m_ui->spinBox_NumOutputs->setMinimum( OutputChannels::MIN / 2 );
    m_ui->spinBox_NumOutputs->setMaximum( OutputChannels::MAX / 2 );
    m_ui->spinBox_NumOutputs->setValue( numOutputs / 2 );
}



JackOutputsDialog::~JackOutputsDialog()
{
    delete m_ui;
}



void JackOutputsDialog::changeEvent( QEvent* event )
{
    QDialog::changeEvent( event );

    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;
    default:
        break;
    }
}



void JackOutputsDialog::on_tableWidget_itemClicked( QTableWidgetItem* item )
{
    QList<int> outputChans;

    if ( m_numSampleChans == 1 )
    {
        outputChans << item->row();
    }
    else // Stereo
    {
        outputChans << item->row() * 2 << ( item->row() * 2 ) + 1;
    }

    int sampleBufferId = item->column();

    bool isChecked = item->checkState() == Qt::Checked;

    qDebug() << outputChans << sampleBufferId << isChecked;
}



void JackOutputsDialog::on_spinBox_NumOutputs_valueChanged( const int value )
{
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    const int currentNumOutputs = config.outputChannels.countNumberOfSetBits();

    if ( currentNumOutputs != value * 2 )
    {
        config.outputChannels.clear();
        config.outputChannels.setRange( 0, value * 2, true );
        config.useDefaultOutputChannels = false;

        String error = m_deviceManager.setAudioDeviceSetup( config, true );

        if ( error.isNotEmpty() )
        {
            MessageBoxes::showWarningDialog( tr("Error while trying to set output channels!"), error.toRawUTF8() );
        }
    }

    if (  m_ui->tableWidget->rowCount() != value )
    {
        ;
    }
}
