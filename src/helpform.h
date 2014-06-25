#ifndef HELPFORM_H
#define HELPFORM_H

#include <QWidget>

namespace Ui
{
    class HelpForm;
}

class HelpForm : public QWidget
{
    Q_OBJECT

public:
    HelpForm( QWidget* parent = NULL );
    ~HelpForm();

protected:
    void changeEvent( QEvent* event );

private:
    Ui::HelpForm* mUI;

private slots:
    void on_pushButton_Forward_clicked();
    void on_pushButton_Back_clicked();
    void on_pushButton_Home_clicked();
};

#endif // HELPFORM_H
