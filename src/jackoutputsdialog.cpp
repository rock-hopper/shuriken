#include "jackoutputsdialog.h"
#include "ui_jackoutputsdialog.h"
#include <QtDebug>


JackOutputsDialog::JackOutputsDialog( QWidget* parent ) :
    QDialog( parent ),
    m_ui( new Ui::JackOutputsDialog )
{
    m_ui->setupUi( this );

    m_ui->tableWidget->setRowCount( NUM_OUTPUT_CHANS );
    m_ui->tableWidget->setColumnCount( NUM_SAMPLE_BUFFERS );

    QStringList verticalHeaders;

    for ( int row = 0; row < NUM_OUTPUT_CHANS; row++ )
    {
        if ( NUM_AUDIO_CHANS == 1)
        {
            verticalHeaders << tr("Channel ") + QString::number( row + 1 );
        }
        else // Stereo
        {
            verticalHeaders << tr("Channel ") + QString::number( row * 2 + 1 ) + " + " + QString::number( row * 2 + 2 );
        }

        for ( int column = 0; column < NUM_SAMPLE_BUFFERS; column++ )
        {
            QTableWidgetItem* newItem = new QTableWidgetItem();
            newItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            newItem->setCheckState( Qt::Unchecked );
            m_ui->tableWidget->setItem( row, column, newItem );
        }
    }

    m_ui->tableWidget->setVerticalHeaderLabels( verticalHeaders );
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

    if ( NUM_AUDIO_CHANS == 1)
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
