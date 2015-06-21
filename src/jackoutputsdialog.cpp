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

JackOutputsDialog::JackOutputsDialog( const int numSampleBuffers,
                                      QList<int> sampleOutputPairs,
                                      AudioDeviceManager& deviceManager,
                                      QWidget* parent ) :
    QDialog( parent ),
    m_numSampleBuffers( numSampleBuffers ),
    m_ui( new Ui::JackOutputsDialog ),
    m_sampleOutputPairs( sampleOutputPairs ),
    m_deviceManager( deviceManager )
{
    Q_ASSERT( m_numSampleBuffers > 0 );

    m_ui->setupUi( this );

    // Get the current number of output channels
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    const int numOutputChans = config.outputChannels.countNumberOfSetBits();

    // Set up table widget
    updateTableWidget( numOutputChans / 2 );

    // Set up "No. of Outputs" spinbox
    m_ui->spinBox_NumOutputs->setMinimum( OutputChannels::MIN / 2 );
    m_ui->spinBox_NumOutputs->setMaximum( OutputChannels::MAX / 2 );
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

void JackOutputsDialog::updateTableWidget( const int numOutputPairs )
{
    // Set number of rows and columns
    m_ui->tableWidget->setRowCount( numOutputPairs );
    m_ui->tableWidget->setColumnCount( m_numSampleBuffers );

    // Set row headers: "Out L + R", "Out 2 L + R", etc...
    QStringList verticalHeaders;

    for ( int row = 0; row < m_ui->tableWidget->rowCount(); row++ )
    {
        verticalHeaders << tr("Out") + (row > 0 ? " " + QString::number(row + 1) : "") + " L + R";
    }

    m_ui->tableWidget->setVerticalHeaderLabels( verticalHeaders );

    // Populate the table with check box items
    for ( int column = 0; column < m_ui->tableWidget->columnCount(); column++ )
    {
        for ( int row = 0; row < m_ui->tableWidget->rowCount(); row++ )
        {
            QTableWidgetItem* newItem = new QTableWidgetItem();
            newItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );

            if ( m_sampleOutputPairs.at( column ) == row )
            {
                newItem->setCheckState( Qt::Checked );
            }
            else
            {
                newItem->setCheckState( Qt::Unchecked );
            }

            m_ui->tableWidget->setItem( row, column, newItem );
        }
    }
}



//==================================================================================================
// Private Slots:

void JackOutputsDialog::on_tableWidget_itemClicked( QTableWidgetItem* const item )
{
    if ( item->checkState() == Qt::Checked )
    {
        const int sampleNum = item->column();
        const int outputPairNum = item->row();

        m_sampleOutputPairs.replace( sampleNum, outputPairNum );
        emit outputPairChanged( sampleNum, outputPairNum );

        // Only one output pair can be selected per audio slice so uncheck all other items in this column
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



void JackOutputsDialog::on_spinBox_NumOutputs_valueChanged( const int numOutputPairs )
{
    // Get current number of outputs
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    const int currentNumOutputChans = config.outputChannels.countNumberOfSetBits();

    // Set new number of outputs
    if ( currentNumOutputChans != numOutputPairs * 2 )
    {
        config.outputChannels.clear();
        config.outputChannels.setRange( 0, numOutputPairs * 2, true );
        config.useDefaultOutputChannels = false;

        String error = m_deviceManager.setAudioDeviceSetup( config, true );

        if ( error.isEmpty() )
        {
            emit numOutputsChanged( numOutputPairs * 2 );
        }
        else
        {
            MessageBoxes::showWarningDialog( tr("Error while trying to set output channels!"), error.toRawUTF8() );
        }
    }

    // Update the table widget
    if ( m_ui->tableWidget->rowCount() != numOutputPairs )
    {
        // Make sure the output number for each sample doesn't exceed the new number of outputs
        if ( numOutputPairs < m_ui->tableWidget->rowCount() )
        {
            const int defaultOutputPair = 0;

            for ( int sampleNum = 0; sampleNum < m_sampleOutputPairs.size(); sampleNum++ )
            {
                if ( m_sampleOutputPairs.at( sampleNum ) >= numOutputPairs )
                {
                    m_sampleOutputPairs.replace( sampleNum, defaultOutputPair );
                    emit outputPairChanged( sampleNum, defaultOutputPair );
                }
            }
        }

        updateTableWidget( numOutputPairs );
    }
}
