#ifndef JACKOUTPUTSDIALOG_H
#define JACKOUTPUTSDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui
{
    class JackOutputsDialog;
}


class JackOutputsDialog : public QDialog
{
    Q_OBJECT

public:
    JackOutputsDialog( QWidget* parent = NULL );
    ~JackOutputsDialog();

protected:
    void changeEvent( QEvent* event );

private:
    Ui::JackOutputsDialog* m_ui;

    static const int NUM_SAMPLE_BUFFERS = 16;
    static const int NUM_OUTPUT_CHANS = 4;
    static const int NUM_AUDIO_CHANS = 2;

private slots:
    void on_tableWidget_itemClicked( QTableWidgetItem* item );
};


#endif // JACKOUTPUTSDIALOG_H
