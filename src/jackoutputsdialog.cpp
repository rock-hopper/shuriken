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


//==================================================================================================
// Public:

JackOutputsDialog::JackOutputsDialog( const int numSampleBuffers, const int numSampleChans, AudioDeviceManager& deviceManager, QWidget* parent ) :
    QDialog( parent ),
    m_numSampleBuffers( numSampleBuffers ),
    m_numSampleChans( numSampleChans ),
    m_ui( new Ui::JackOutputsDialog ),
    m_deviceManager( deviceManager )
{
    Q_ASSERT( m_numSampleBuffers > 0 );
    Q_ASSERT( m_numSampleChans > 0 );

    m_ui->setupUi( this );

    updateTableWidget();

    // Set up "No. of Outputs" spinbox
    m_ui->spinBox_NumOutputs->setMinimum( OutputChannels::MIN / 2 );
    m_ui->spinBox_NumOutputs->setMaximum( OutputChannels::MAX / 2 );

    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    const int numOutputChans = config.outputChannels.countNumberOfSetBits();
    m_ui->spinBox_NumOutputs->setValue( numOutputChans / 2 );
}



JackOutputsDialog::~JackOutputsDialog()
{
    delete m_ui;
}



//==================================================================================================
// Protected:

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



//==================================================================================================
// Private:

void JackOutputsDialog::updateTableWidget()
{
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    const int numOutputs = config.outputChannels.countNumberOfSetBits();

    m_ui->tableWidget->setRowCount( numOutputs / 2 );
    m_ui->tableWidget->setColumnCount( m_numSampleBuffers );

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
}



//==================================================================================================
// Private Slots:

void JackOutputsDialog::on_tableWidget_itemClicked( QTableWidgetItem* const item )
{
    if ( item->checkState() == Qt::Checked )
    {
        const int sampleNum = item->column();
        const int outputPairNum = item->row();

        emit outputPairChanged( sampleNum, outputPairNum );

        // Only one output pair can be selected for each sample so uncheck all other items in this column
        const int selectedCol = item->column();
        const int selectedRow = item->row();

        for ( int row = 0; row < m_ui->tableWidget->rowCount(); row++ )
        {
            if ( row != selectedRow )
            {
                m_ui->tableWidget->item( row, selectedCol )->setCheckState( Qt::Unchecked );
            }
        }
    }
    else // If user clicks on an item that's already checked then make sure it stays checked
    {
        item->setCheckState( Qt::Checked );
    }
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
        updateTableWidget();
    }
}
