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

#include "messageboxes.h"


//==================================================================================================
// Public Static:

void MessageBoxes::showWarningDialog( const QString text, const QString infoText )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Shuriken Beat Slicer" );
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setText( text );
    msgBox.setInformativeText( infoText );
    msgBox.exec();
}



int MessageBoxes::showUnsavedChangesDialog()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Shuriken Beat Slicer" );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setText( "The project has been modified" );
    msgBox.setInformativeText( "Do you want to save your changes?" );
    msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
    msgBox.setDefaultButton( QMessageBox::Save );
    const int buttonClicked = msgBox.exec();

    return buttonClicked;
}



int MessageBoxes::showQuestionDialog( const QString text, const QString infoText, const QMessageBox::StandardButtons buttons )
{
    QMessageBox msgBox;
    msgBox.setWindowTitle( "Shuriken Beat Slicer" );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setText( text );
    msgBox.setInformativeText( infoText );
    msgBox.setStandardButtons( buttons );
    msgBox.setDefaultButton( QMessageBox::Ok );
    const int buttonClicked = msgBox.exec();

    return buttonClicked;
}
