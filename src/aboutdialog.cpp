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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"


//==================================================================================================
// Public:

AboutDialog::AboutDialog( QWidget* parent ) :
    QDialog( parent ),
    m_ui( new Ui::AboutDialog )
{
    m_ui->setupUi( this );

    m_ui->textBrowser->setOpenExternalLinks( true );
    m_ui->textBrowser->setSource( QUrl( "qrc:/resources/docs/about.html" ) );
}



AboutDialog::~AboutDialog()
{
    delete m_ui;
}



//==================================================================================================
// Protected:

void AboutDialog::changeEvent( QEvent* event )
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
