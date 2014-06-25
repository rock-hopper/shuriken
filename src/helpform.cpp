#include "helpform.h"
#include "ui_helpform.h"


//==================================================================================================
// Public:

HelpForm::HelpForm( QWidget* parent ) :
    QWidget( parent ),
    mUI( new Ui::HelpForm )
{
    mUI->setupUi( this );

    QObject::connect( mUI->textBrowser, SIGNAL( backwardAvailable(bool) ),
                      mUI->pushButton_Back, SLOT( setEnabled(bool) ) );

    QObject::connect( mUI->textBrowser, SIGNAL( forwardAvailable(bool) ),
                      mUI->pushButton_Forward, SLOT( setEnabled(bool) ) );

    mUI->textBrowser->setOpenExternalLinks( true );
    mUI->textBrowser->setSource( QUrl( "qrc:/docs/index.html" ) );
}



HelpForm::~HelpForm()
{
    delete mUI;
}



//==================================================================================================
// Protected:

void HelpForm::changeEvent( QEvent* event )
{
    QWidget::changeEvent( event );

    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        mUI->retranslateUi( this );
        break;
    default:
        break;
    }
}



//==================================================================================================
// Private Slots:

void HelpForm::on_pushButton_Home_clicked()
{
    mUI->textBrowser->home();
}



void HelpForm::on_pushButton_Back_clicked()
{
    mUI->textBrowser->backward();
}



void HelpForm::on_pushButton_Forward_clicked()
{
    mUI->textBrowser->forward();
}
