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

#include "slicepointitem.h"
#include <QBrush>
//#include <QDebug>


//==================================================================================================
// Public:

SlicePointItem::SlicePointItem( const qreal height, QGraphicsItem* parent ) :
    FrameMarkerItem( QColor( Qt::red ), QColor(255, 192, 0, 255), height, HANDLE_TOP_BOTTOM, parent )
{
}



//==================================================================================================
// Protected:

void SlicePointItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    FrameMarkerItem::mousePressEvent( event );

    m_scenePosBeforeMove = pos().x();
}



void SlicePointItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseReleaseEvent( event );

    if ( m_scenePosBeforeMove != pos().x() )
    {
        emit scenePosChanged( this );
    }
}
