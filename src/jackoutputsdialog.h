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
