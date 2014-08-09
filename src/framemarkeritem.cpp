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

#include "framemarkeritem.h"
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
//#include <QDebug>


//==================================================================================================
// Public:

FrameMarkerItem::FrameMarkerItem( const QBrush brush, const QBrush selectedBrush, const qreal height, QGraphicsItem* parent ) :
    QObject(),
    QGraphicsPolygonItem( parent ),
    mSelectedBrush( selectedBrush )
{
    setBrush( brush );
    setHeight( height );
    setZValue( 2 );
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges );
}



void FrameMarkerItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED( widget );

    painter->setPen( pen() );

    if ( option->state & QStyle::State_Selected )
    {
        painter->setBrush( mSelectedBrush );
    }
    else
    {
        painter->setBrush( brush() );
    }

    painter->drawPolygon( polygon(), fillRule() );
}



void FrameMarkerItem::setHeight( const qreal height )
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

QVariant FrameMarkerItem::itemChange( GraphicsItemChange change, const QVariant &value )
{
    // Keep FrameMarkerItem within bounds of scene rect
    if ( change == ItemPositionChange && scene() != NULL )
    {
        QPointF newPos = value.toPointF();
        const QRectF sceneRect = scene()->sceneRect();

        if ( ! sceneRect.contains( newPos ) )
        {
            newPos.setX
            (
                    qMin( sceneRect.right() - 1, qMax( newPos.x(), sceneRect.left() ) )
            );
        }
        newPos.setY( 0.0 );

        return newPos;
    }

    return QGraphicsItem::itemChange( change, value );
}



void FrameMarkerItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    // Always unset the Ctrl-key modifier to prevent multiple slice point items from being selected
    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~Qt::ControlModifier;
    event->setModifiers( modifiers );

    QGraphicsItem::mousePressEvent( event );
}
