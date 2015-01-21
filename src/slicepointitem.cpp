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
#include <QGraphicsScene>
//#include <QDebug>


//==================================================================================================
// Public:

SlicePointItem::SlicePointItem( const qreal height,
                                const bool canBeMovedPastOtherSlicePoints,
                                const qreal minDistFromOtherSlicePoints,
                                QGraphicsItem* parent ) :
    FrameMarkerItem( canBeMovedPastOtherSlicePoints ? QColor( Qt::red ) : QColor( 95, 207, 0, 255 ),
                     QColor( 255, 192, 0, 255 ),
                     height,
                     HANDLE_TOP_BOTTOM,
                     parent ),
    m_canBeMovedPastOtherSlicePoints( canBeMovedPastOtherSlicePoints ),
    m_minDistFromOtherItems( minDistFromOtherSlicePoints )
{
}



//==================================================================================================
// Public Static:

bool SlicePointItem::isLessThanFrameNum( const SharedSlicePointItem item1, const SharedSlicePointItem item2 )
{
    return item1->getFrameNum() < item2->getFrameNum();
}



//==================================================================================================
// Protected:

QVariant SlicePointItem::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if ( ! m_canBeMovedPastOtherSlicePoints )
    {
        if ( change == ItemPositionChange && scene() != NULL )
        {
            QPointF newPos = value.toPointF();

            if ( newPos.x() < m_minScenePosX )
            {
                newPos.setX( m_minScenePosX );
            }
            else if ( newPos.x() > m_maxScenePosX )
            {
                newPos.setX( m_maxScenePosX );
            }

            return FrameMarkerItem::itemChange( change, newPos );
        }
    }

    return FrameMarkerItem::itemChange( change, value );
}



void SlicePointItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    FrameMarkerItem::mousePressEvent( event );

    qreal minX = scene()->sceneRect().left();
    qreal maxX = scene()->sceneRect().right() - 1;

    foreach ( QGraphicsItem* item, scene()->items() )
    {
        if ( item != this && item->type() == SlicePointItem::Type )
        {
            SlicePointItem* const otherSlicePoint = qgraphicsitem_cast<SlicePointItem*>( item );

            const qreal otherItemPosX = otherSlicePoint->scenePos().x();

            if ( otherItemPosX > minX && otherItemPosX < pos().x() )
            {
                minX = otherItemPosX;
            }

            if ( otherItemPosX < maxX && otherItemPosX > pos().x() )
            {
                maxX = otherItemPosX;
            }
        }
    }

    m_minScenePosX = minX + m_minDistFromOtherItems;
    m_maxScenePosX = maxX - m_minDistFromOtherItems;
}
