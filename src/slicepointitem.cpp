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
#include "wavegraphicsscene.h"
#include <QBrush>
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
    m_isSnapEnabled( false ),
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
    if ( change == ItemPositionChange && scene() != NULL )
    {
        QPointF newPos = value.toPointF();

        if ( m_isSnapEnabled ) // Snap slice point to BPM ruler marks
        {
            WaveGraphicsScene* scene = static_cast<WaveGraphicsScene*>( this->scene() );

            foreach ( SharedGraphicsItem item, scene->getBpmRulerMarks() )
            {
                const qreal itemPosX = item->scenePos().x();

                if ( qAbs( newPos.x() - itemPosX ) < 8.0 ) // Snap!
                {
                    newPos.setX( itemPosX );

                    setRulerMarkColour( item.data(), Qt::lightGray );
                }
                else
                {
                    setRulerMarkColour( item.data(), Qt::white );
                }
            }
        }

        if ( ! m_canBeMovedPastOtherSlicePoints ) // Prevent slice point from being moved past other slice points
        {
            if ( newPos.x() < m_minScenePosX )
            {
                newPos.setX( m_minScenePosX );
            }
            else if ( newPos.x() > m_maxScenePosX )
            {
                newPos.setX( m_maxScenePosX );
            }
        }

        return FrameMarkerItem::itemChange( change, newPos );
    }

    return FrameMarkerItem::itemChange( change, value );
}



void SlicePointItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    FrameMarkerItem::mousePressEvent( event );

    qreal minX = scene()->sceneRect().left();
    qreal maxX = scene()->sceneRect().right() - 1;

    // Calculate how far this slice point can be moved to the left and the right
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



void SlicePointItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    FrameMarkerItem::mouseReleaseEvent( event );

    // Reset colour of BPM ruler marks
    WaveGraphicsScene* scene = static_cast<WaveGraphicsScene*>( this->scene() );

    foreach ( SharedGraphicsItem item, scene->getBpmRulerMarks() )
    {
        setRulerMarkColour( item.data(), Qt::white );
    }
}



//==================================================================================================
// Private Static:

void SlicePointItem::setRulerMarkColour( QGraphicsItem* const item, const QColor colour )
{
    if ( item->type() == QGraphicsLineItem::Type )
    {
        QGraphicsLineItem* const lineItem = qgraphicsitem_cast<QGraphicsLineItem*>( item );
        lineItem->setPen( colour );
    }
    else if ( item->type() == QGraphicsSimpleTextItem::Type )
    {
        QGraphicsSimpleTextItem* const textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem*>( item );
        textItem->setBrush( colour );
    }
}
