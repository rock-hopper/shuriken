/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

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
    mUI->textBrowser->setSource( QUrl( "qrc:/resources/docs/index.html" ) );
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
