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

#include "slicepointitem.h"
#include "wavegraphicsscene.h"
#include <QBrush>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QtDebug>


//==================================================================================================
// Public:

SlicePointItem::SlicePointItem( const qreal height,
                                const bool canBeMovedPastOtherSlicePoints,
                                const qreal minDistFromOtherSlicePoints,
                                QGraphicsItem* parent ) :
    QObject(),
    QGraphicsPolygonItem( parent ),
    m_selectedBrush( QColor( 255, 192, 0, 255 ) ),
    m_canBeMovedPastOtherSlicePoints( canBeMovedPastOtherSlicePoints ),
    m_isSnapEnabled( false ),
    m_isLeftMousePressed( false ),
    m_frameNum( 0 ),
    m_scenePosX_beforeMove( 0.0 ),
    m_minDistFromOtherItems( minDistFromOtherSlicePoints ),
    m_minScenePosX( 0.0 ),
    m_maxScenePosX( 1.0 )
{
    if ( canBeMovedPastOtherSlicePoints )
    {
        setBrush( QColor( Qt::red ) );
    }
    else
    {
        setBrush( QColor( 95, 207, 0, 255 ) ); // Green
    }

    setHeight( height );

    setZValue( ZValues::SLICE_POINT );
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges );
}



void SlicePointItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED( widget );

    painter->setPen( pen() );

    if ( option->state & QStyle::State_Selected )
    {
        painter->setBrush( m_selectedBrush );
    }
    else
    {
        painter->setBrush( brush() );
    }

    painter->drawPolygon( polygon(), fillRule() );
}



void SlicePointItem::setHeight( qreal height )
{
    QPolygonF polygon;

    --height;
    const qreal handleWidth = 16.0;
    const qreal handleHeight = 16.0;

    polygon << QPointF( -handleWidth * 0.5, 0.0 ) << QPointF( handleWidth * 0.5, 0.0 ) << QPointF( 0.0, handleHeight )
            << QPointF( 0.0, height - handleHeight )
            << QPointF( -handleWidth * 0.5, height ) << QPointF( handleWidth * 0.5, height ) << QPointF( 0.0, height - handleHeight )
            << QPointF( 0.0, handleHeight );

    setPolygon( polygon );
}



void SlicePointItem::setPos( const qreal x, const qreal y )
{
#if QT_VERSION >= 0x040700  // Qt 4.7
    QGraphicsItem::setPos( QPointF(x, y+1) );
#else
    QGraphicsItem::setPos( QPointF(x, y) );
#endif
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

        // Snap slice point to BPM ruler marks
        if ( m_isSnapEnabled )
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

        // If required, prevent slice point from being moved past other slice points
        if ( ! m_canBeMovedPastOtherSlicePoints && m_isLeftMousePressed )
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

        // Keep item within bounds of scene rect
        const QRectF sceneRect = scene()->sceneRect();

        if ( newPos.x() < sceneRect.left() || newPos.x() > sceneRect.right() - 1 )
        {
            newPos.setX
            (
                qMin( sceneRect.right() - 1, qMax( newPos.x(), sceneRect.left() ) )
            );
        }

#if QT_VERSION >= 0x040700  // Qt 4.7
        newPos.setY( BpmRuler::HEIGHT + 1 );
#else
        newPos.setY( BpmRuler::HEIGHT );
#endif

        return newPos;
    }


    // If this item is selected then raise it above other slice point items
    if ( change == ItemSelectedHasChanged )
    {
        if ( isSelected() )
            setZValue( ZValues::SELECTED_SLICE_POINT );
        else
            setZValue( ZValues::SLICE_POINT );
    }


    return QGraphicsItem::itemChange( change, value );
}



void SlicePointItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    // Always unset the Ctrl-key modifier to prevent multiple slice point items from being selected

    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~Qt::ControlModifier;
    event->setModifiers( modifiers );

    QGraphicsItem::mousePressEvent( event );

    if ( event->button() == Qt::LeftButton )
    {
        m_scenePosX_beforeMove = scenePos().x();

        calcMinMaxScenePosX();

        m_isLeftMousePressed = true;
    }
}



void SlicePointItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseReleaseEvent( event );

    if ( event->button() == Qt::LeftButton )
    {
        WaveGraphicsScene* const scene = static_cast<WaveGraphicsScene*>( this->scene() );

        // If slice point has been moved then set new frame number
        if ( m_scenePosX_beforeMove != scenePos().x() )
        {
            const int oldFrameNum = getFrameNum();
            const int newFrameNum = scene->getFrameNum( scenePos().x() );

            setFrameNum( newFrameNum );

            emit scenePosChanged( this, oldFrameNum );
        }

        // Reset colour of BPM ruler marks
        foreach ( SharedGraphicsItem item, scene->getBpmRulerMarks() )
        {
            setRulerMarkColour( item.data(), Qt::white );
        }

        m_isLeftMousePressed = false;
    }
}



void SlicePointItem::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    QMenu menu;
    QAction* nextAction = menu.addAction( tr("Move to next zero-crossing") );
    QAction* prevAction = menu.addAction( tr("Move to previous zero-crossing") );
    QAction* selectedAction = menu.exec(event->screenPos());

    if ( selectedAction == nextAction )
    {
        //setPos( 0.0, 0.0 );
    }
    else if ( selectedAction == prevAction )
    {
        //setPos( 100.0, 0.0 );
    }

    // Prevent wonky behaviour

    while ( QGraphicsItem* item = scene()->mouseGrabberItem() )
    {
        item->ungrabMouse();
    }

    foreach ( QGraphicsItem* item, scene()->items() )
    {
        if ( item->type() == SlicePointItem::Type )
        {
            item->setSelected( false );
        }
    }
}



//==================================================================================================
// Private:

void SlicePointItem::calcMinMaxScenePosX()
{
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
