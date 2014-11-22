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

#include "loopmarkeritem.h"
#include <QBrush>
#include <QGraphicsScene>
#include <QDebug>


//==================================================================================================
// Public:

LoopMarkerItem::LoopMarkerItem( const MarkerType markerType,
                                const qreal height,
                                QGraphicsItem* parent ) :
    FrameMarkerItem( QColor( Qt::yellow ),
                     QColor( 255, 255, 150, 255 ),
                     height,
                     markerType == LEFT_MARKER ? HANDLE_CENTRE_RIGHT : HANDLE_CENTRE_LEFT,
                     parent ),
    m_markerType( markerType )
{
}



//==================================================================================================
// Protected:

QVariant LoopMarkerItem::itemChange( GraphicsItemChange change, const QVariant &value )
{
    if ( change == ItemPositionChange && scene() != NULL )
    {
        QPointF newPos = value.toPointF();

        // Prevent loop markers from going past each other
        foreach ( QGraphicsItem* item, scene()->items() )
        {
            if ( item != this && item->type() == LoopMarkerItem::Type )
            {
                LoopMarkerItem* const otherLoopMarker = qgraphicsitem_cast<LoopMarkerItem*>( item );

                const QPointF otherItemPos = otherLoopMarker->scenePos();

                if ( otherLoopMarker->getMarkerType() == LEFT_MARKER )
                {
                    if ( newPos.x() - m_handleWidth <= otherItemPos.x() + m_handleWidth )
                    {
                        newPos.setX( otherItemPos.x() + m_handleWidth * 2 );
                    }
                }
                else // otherLoopMarker->getMarkerType() == RIGHT_MARKER
                {
                    if ( newPos.x() + m_handleWidth >= otherItemPos.x() - m_handleWidth )
                    {
                        newPos.setX( otherItemPos.x() - m_handleWidth * 2 );
                    }
                }
            }
        }

        return FrameMarkerItem::itemChange( change, newPos );
    }

    return FrameMarkerItem::itemChange( change, value );
}



void LoopMarkerItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    FrameMarkerItem::mousePressEvent( event );

    m_scenePosBeforeMove = pos().x();
}



void LoopMarkerItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseReleaseEvent( event );

    if ( m_scenePosBeforeMove != pos().x() )
    {
        emit scenePosChanged( this );
    }
}
