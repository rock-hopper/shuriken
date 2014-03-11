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


//==================================================================================================
// Public:

SlicePointItem::SlicePointItem( const qreal height, QGraphicsItem* parent ) :
        QObject(), QGraphicsPolygonItem( parent )
{
    setHeight( height );
    setBrush( QBrush( QColor( Qt::red ) ) );
    setZValue( 2 );
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges );
}



void SlicePointItem::setHeight( const qreal height )
{
    const qreal triangleWidth = 16.0;
    const qreal triangleHeight = 16.0;

    QPolygonF polygon;
    polygon << QPointF( -triangleWidth * 0.5, 0.0 ) << QPointF( triangleWidth * 0.5, 0.0 ) << QPointF( 0.0, triangleHeight )
            << QPointF( 0.0, height - triangleHeight )
            << QPointF( -triangleWidth * 0.5, height ) << QPointF( triangleWidth * 0.5, height ) << QPointF( 0.0, height - triangleHeight )
            << QPointF( 0.0, triangleHeight );

    setPolygon( polygon );
}



//==================================================================================================
// Protected:

QVariant SlicePointItem::itemChange( GraphicsItemChange change, const QVariant &value )
{
    // Keep SlicePointItem within bounds of scene rect
    if ( change == ItemPositionChange && scene() != NULL )
    {
        QPointF newPosTop = value.toPointF();
        const QPointF newPosBottom( newPosTop.x(), newPosTop.y() + boundingRect().height() );
        const QRectF sceneRect = scene()->sceneRect();

        if ( ! ( sceneRect.contains( newPosTop ) && sceneRect.contains( newPosBottom ) ) )
        {
            newPosTop.setX
            (
                    qMin( sceneRect.right() - 1, qMax( newPosTop.x(), sceneRect.left() ) )
            );
            newPosTop.setY( 0.0 );

            return newPosTop;
        }
    }

    return QGraphicsItem::itemChange( change, value );
}
