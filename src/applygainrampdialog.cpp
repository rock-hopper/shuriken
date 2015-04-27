/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#include "applygainrampdialog.h"
#include "ui_applygainrampdialog.h"


//==================================================================================================
// Public:

ApplyGainRampDialog::ApplyGainRampDialog( QWidget* parent ) :
    QDialog( parent ),
    m_ui( new Ui::ApplyGainRampDialog )
{
    m_ui->setupUi( this );
}



ApplyGainRampDialog::~ApplyGainRampDialog()
{
    delete m_ui;
}



qreal ApplyGainRampDialog::getStartGainValue() const
{
    return m_ui->doubleSpinBox_StartGain->value();
}



qreal ApplyGainRampDialog::getEndGainValue() const
{
    return m_ui->doubleSpinBox_EndGain->value();
}



//==================================================================================================
// Protected:

void ApplyGainRampDialog::changeEvent( QEvent* event )
{
    QDialog::changeEvent( event );

    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



//==================================================================================================
// Private Slots:

void ApplyGainRampDialog::on_dial_StartGain_valueChanged( const int value )
{
    m_ui->doubleSpinBox_StartGain->setValue( value / 10.0 );
}



void ApplyGainRampDialog::on_doubleSpinBox_StartGain_valueChanged( const double value )
{
    disconnect( m_ui->dial_StartGain, SIGNAL( valueChanged(int) ),
                this, SLOT( on_dial_StartGain_valueChanged(int) ) );

    m_ui->dial_StartGain->setValue( value * 10 );

    connect( m_ui->dial_StartGain, SIGNAL( valueChanged(int) ),
             this, SLOT( on_dial_StartGain_valueChanged(int) ) );
}



void ApplyGainRampDialog::on_dial_EndGain_valueChanged( const int value )
{
    m_ui->doubleSpinBox_EndGain->setValue( value / 10.0 );
}



void ApplyGainRampDialog::on_doubleSpinBox_EndGain_valueChanged( const double value )
{
    disconnect( m_ui->dial_EndGain, SIGNAL( valueChanged(int) ),
                this, SLOT( on_dial_EndGain_valueChanged(int) ) );

    m_ui->dial_EndGain->setValue( value * 10 );

    connect( m_ui->dial_EndGain, SIGNAL( valueChanged(int) ),
             this, SLOT( on_dial_EndGain_valueChanged(int) ) );
}
