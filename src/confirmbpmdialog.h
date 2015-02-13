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

#ifndef CONFIRMBPMDIALOG_H
#define CONFIRMBPMDIALOG_H

#include <QDialog>
#include "JuceHeader.h"


namespace Ui
{
    class ConfirmBpmDialog;
}


class ConfirmBpmDialog : public QDialog
{
    Q_OBJECT
public:
    enum TimeSigNumerator { ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, ELEVEN, TWELVE, THIRTEEN, FOURTEEN, FIFTEEN, SIXTEEN };
    enum TimeSigDenominator { WHOLE_NOTE, HALF_NOTE, QUARTER_NOTE, EIGHTH_NOTE, SIXTEENTH_NOTE };

    ConfirmBpmDialog( qreal bpm,
                      TimeSigNumerator numerator,
                      TimeSigDenominator denominator,
                      QWidget* parent = NULL );
    ~ConfirmBpmDialog();

    qreal getBpm() const;
    int getTimeSigNumerator() const;
    int getTimeSigDenominator() const;

protected:
    void changeEvent( QEvent* event );

private:
    Ui::ConfirmBpmDialog* m_ui;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( ConfirmBpmDialog );
};


#endif // CONFIRMBPMDIALOG_H
