#include "jackoutputsdialog.h"
#include "ui_jackoutputsdialog.h"
#include <QtDebug>
#include "globals.h"


JackOutputsDialog::JackOutputsDialog( const int numSampleBuffers, const int numSampleChans, AudioDeviceManager& deviceManager, QWidget* parent ) :
    QDialog( parent ),
    m_numSampleChans( numSampleChans ),
    m_ui( new Ui::JackOutputsDialog ),
    m_deviceManager( deviceManager )
{
    m_ui->setupUi( this );

    // Set up table widget
    m_ui->tableWidget->setRowCount( Jack::g_numOutputChans / m_numSampleChans );
    m_ui->tableWidget->setColumnCount( numSampleBuffers );

    QStringList verticalHeaders;

    for ( int row = 0; row < m_ui->tableWidget->rowCount(); row++ )
    {
        if ( m_numSampleChans == 1)
        {
            verticalHeaders << tr("Channel ") + QString::number( row + 1 );
        }
        else // Stereo
        {
            verticalHeaders << tr("Channel ") + QString::number( row * 2 + 1 ) + " + " + QString::number( row * 2 + 2 );
        }

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
    m_ui->spinBox_NumOutputs->setMinimum( OutputChannels::MIN );
    m_ui->spinBox_NumOutputs->setMaximum( OutputChannels::MAX );
    m_ui->spinBox_NumOutputs->setSingleStep( m_numSampleChans );
    m_ui->spinBox_NumOutputs->setValue( Jack::g_numOutputChans );
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

    if ( m_numSampleChans == 1)
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
    if ( Jack::g_numOutputChans != value )
    {
        Jack::g_numOutputChans = value;

        m_deviceManager.closeAudioDevice();
        m_deviceManager.restartLastAudioDevice();
    }

    if (  m_ui->tableWidget->rowCount() != value / m_numSampleChans )
    {
        ;
    }
}
