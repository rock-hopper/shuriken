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

#include "confirmbpmdialog.h"
#include "ui_confirmbpmdialog.h"


//==================================================================================================
// Public:

ConfirmBpmDialog::ConfirmBpmDialog( const qreal bpm,
                                    const TimeSigNumerator numerator,
                                    const TimeSigDenominator denominator,
                                    QWidget* parent  ) :
    QDialog( parent ),
    mUI( new Ui::ConfirmBpmDialog )
{
    mUI->setupUi( this );

    if ( bpm > 0.0 )
    {
        mUI->doubleSpinBox_BPM->setValue( bpm );
    }

    // Populate "Time Signature" combo boxes
    QStringList timeSigNumeratorTextList;
    QStringList timeSigDenominatorTextList;

    timeSigNumeratorTextList << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
    timeSigDenominatorTextList << "1" << "2" << "4" << "8" << "16";

    mUI->comboBox_TimeSigNumerator->addItems( timeSigNumeratorTextList );
    mUI->comboBox_TimeSigDenominator->addItems( timeSigDenominatorTextList );

    mUI->comboBox_TimeSigNumerator->setCurrentIndex( numerator );   // 4
    mUI->comboBox_TimeSigDenominator->setCurrentIndex( denominator ); // 4
}



ConfirmBpmDialog::~ConfirmBpmDialog()
{
    delete mUI;
}



qreal ConfirmBpmDialog::getBpm() const
{
    return mUI->doubleSpinBox_BPM->value();
}



int ConfirmBpmDialog::getTimeSigNumerator() const
{
    return mUI->comboBox_TimeSigNumerator->currentText().toInt();
}



int ConfirmBpmDialog::getTimeSigDenominator() const
{
    return mUI->comboBox_TimeSigDenominator->currentText().toInt();
}



//==================================================================================================
// Protected:

void ConfirmBpmDialog::changeEvent( QEvent* event )
{
    QDialog::changeEvent( event );
    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        mUI->retranslateUi( this );
        break;
    default:
        break;
    }
}
