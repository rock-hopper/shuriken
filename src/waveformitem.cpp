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

#include "waveformitem.h"
#include "wavegraphicsscene.h"
#include <QDebug>


//==================================================================================================
// Public:

WaveformItem::WaveformItem( const SharedSampleBuffer sampleBuffer,
                            const int orderPos,
                            const qreal width, const qreal height,
                            QGraphicsItem* parent ) :
    QObject(),
    QGraphicsRectItem( 0.0, 0.0, width, height, parent ),
    m_sampleBuffer( sampleBuffer ),
    m_currentOrderPos( orderPos ),
    m_globalScaleFactor( NOT_SET ),
    m_stretchRatio( 1.0 ),
    m_firstCalculatedBin( NOT_SET ),
    m_lastCalculatedBin( NOT_SET )
{
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges | ItemUsesExtendedStyleOption );

    setBackgroundGradient();
    m_wavePen  = QPen( QColor(23, 23, 135, 191) );
    m_centreLinePen = QPen( QColor(127, 127, 127, 191) );

    // Don't draw rect border
    setPen( Qt::NoPen );

    // Set up min/max sample "bins"
    const int numChans = m_sampleBuffer->getNumChannels();
    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        m_minSampleValues.add( new Array<float> );
        m_maxSampleValues.add( new Array<float> );
    }
}



void WaveformItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED( widget );

    const int numChans = m_sampleBuffer->getNumChannels();

    int firstVisibleBin = 0;
    int lastVisibleBin = 0;
    int numVisibleBins = 0;

    qreal distanceBetweenFrames = 0.0;
    int firstVisibleFrame = 0;
    int lastVisibleFrame = 0;
    int numVisibleFrames = 0;

    // If scale factor has changed since the last redraw then reset sample bins and establish new detail level
    if ( m_globalScaleFactor != painter->worldTransform().m11() )
    {
        m_globalScaleFactor = painter->worldTransform().m11(); // m11() returns the current horizontal scale factor
        resetSampleBins();
    }

    if ( m_detailLevel != VERY_HIGH )
    {
        // Reduce no. of samples to draw by finding the min/max values in each consecutive sample "bin"
        firstVisibleBin = qMax( (int) floor( option->exposedRect.left() * m_globalScaleFactor ), 0 );
        lastVisibleBin = qMin( (int) ceil( option->exposedRect.right() * m_globalScaleFactor ), m_numBins - 1 );

        if ( m_firstCalculatedBin == NOT_SET || m_lastCalculatedBin == NOT_SET )
        {
            findMinMaxSamples( firstVisibleBin, lastVisibleBin );
            m_firstCalculatedBin = firstVisibleBin;
            m_lastCalculatedBin = lastVisibleBin;
        }

        if ( firstVisibleBin < m_firstCalculatedBin )
        {
            findMinMaxSamples( firstVisibleBin, m_firstCalculatedBin - 1 );
            m_firstCalculatedBin = firstVisibleBin;
        }

        if ( lastVisibleBin > m_lastCalculatedBin )
        {
            findMinMaxSamples( m_lastCalculatedBin + 1, lastVisibleBin );
            m_lastCalculatedBin = lastVisibleBin;
        }

        numVisibleBins = lastVisibleBin - firstVisibleBin + 1;
    }
    else // mDetailLevel == VERY_HIGH
    {
        distanceBetweenFrames = rect().width() / m_sampleBuffer->getNumFrames();

        firstVisibleFrame = (int) floor( option->exposedRect.left() / distanceBetweenFrames );

        lastVisibleFrame = qMin( (int) ceil( option->exposedRect.right() / distanceBetweenFrames ),
                                 m_sampleBuffer->getNumFrames() - 1 );

        numVisibleFrames = lastVisibleFrame - firstVisibleFrame + 1;
    }

    // Draw rect background
    painter->setPen( pen() );
    painter->setBrush( brush() );
    painter->drawRect( rect() );

    // Scale waveform to fit size of rect
    painter->save();
    painter->scale( 1.0, rect().height() * 0.5 / numChans );

    // Draw centre line/lines
    painter->save();
    painter->translate( 0.0, 1.0 );
    painter->setPen( m_centreLinePen );
    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        painter->drawLine( 0.0, 0.0, rect().width(), 0.0 );
        painter->translate( 0.0, 1.0 * numChans );
    }
    painter->restore();

    // Draw waveform
    painter->translate( 0.0, 1.0 );
    painter->setPen( m_wavePen );

    const qreal lineWidth = 1.0 / m_globalScaleFactor;
    const qreal leftEdge = option->exposedRect.left();

    float min;
    float max;

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        if ( m_detailLevel == LOW )
        {
            painter->setRenderHint( QPainter::Antialiasing, false );

            for ( int binCount = 0; binCount < numVisibleBins; binCount++ )
            {
                min = (*m_minSampleValues[ chanNum ])[ firstVisibleBin + binCount ];
                max = (*m_maxSampleValues[ chanNum ])[ firstVisibleBin + binCount ];

                painter->drawLine
                (
                    QPointF( leftEdge + (binCount * lineWidth), -min ),
                    QPointF( leftEdge + (binCount * lineWidth), -max )
                );
            }
        }
        else if ( m_detailLevel == HIGH )
        {
            painter->setRenderHint( QPainter::Antialiasing, true );

            QPointF points[ numVisibleBins * 2 ];

            for ( int binCount = 0; binCount < numVisibleBins; binCount++ )
            {
                min = (*m_minSampleValues[ chanNum ])[ firstVisibleBin + binCount ];
                max = (*m_maxSampleValues[ chanNum ])[ firstVisibleBin + binCount ];

                points[ binCount * 2 ]       = QPointF( leftEdge + (binCount * lineWidth), -min );
                points[ (binCount * 2) + 1 ] = QPointF( leftEdge + (binCount * lineWidth), -max );
            }
            painter->drawPolyline( points, numVisibleBins * 2 );
        }
        else // mDetailLevel == VERY_HIGH
        {
            painter->setRenderHint( QPainter::Antialiasing, true );

            QPointF points[ numVisibleFrames ];
            const float* sampleData = m_sampleBuffer->getReadPointer( chanNum, firstVisibleFrame );

            for ( int frameCount = 0; frameCount < numVisibleFrames; frameCount++ )
            {
                points[ frameCount ] = QPointF( leftEdge + (frameCount * distanceBetweenFrames),
                                                -(*sampleData) );
                sampleData++;
            }
            painter->drawPolyline( points, numVisibleFrames );
        }

        painter->translate( 0.0, numChans );
    }

    painter->restore();

    // If selected draw highlight
    if ( option->state & QStyle::State_Selected )
    {
        painter->setBrush( QColor(255, 127, 127, 70) );
        painter->drawRect( rect() );
    }
}



void WaveformItem::setRect( const qreal x, const qreal y, const qreal width, const qreal height )
{
    QGraphicsRectItem::setRect( x, y, width, height );
    setBackgroundGradient();
    resetSampleBins();
}



//==================================================================================================
// Public Static:

bool WaveformItem::isLessThanOrderPos( const WaveformItem* const item1, const WaveformItem* const item2 )
{
    return item1->getOrderPos() < item2->getOrderPos();
}



//==================================================================================================
// Protected:

QVariant WaveformItem::itemChange( GraphicsItemChange change, const QVariant &value )
{
    // Keep waveform item within bounds of scene rect
    if ( change == ItemPositionChange && scene() != NULL )
    {
        qreal minDistanceFromSceneLeftEdge = 0.0;
        qreal minDistanceFromSceneRightEdge = 0.0;

        // If this item is part of a group of selected items then calculate the
        // minimum distance it must be from the left and right edges of the scene
        if ( isSelected() )
        {
            WaveGraphicsScene* scene = static_cast<WaveGraphicsScene*>( this->scene() );
            QList<WaveformItem*> selectedItems = scene->getSelectedWaveforms();

            foreach( WaveformItem* item, selectedItems )
            {
                if ( getOrderPos() > item->getOrderPos() )
                {
                    minDistanceFromSceneLeftEdge += item->rect().width();
                }
                else if ( getOrderPos() < item->getOrderPos() )
                {
                    minDistanceFromSceneRightEdge += item->rect().width();
                }
            }
        }

        QPointF newPos = value.toPointF();

        const qreal newPosRightEdge = newPos.x() + rect().width() - 1;
        const QRectF sceneRect = scene()->sceneRect();

        if ( newPos.x() < minDistanceFromSceneLeftEdge )
        {
            newPos.setX( minDistanceFromSceneLeftEdge );
        }
        else if ( newPosRightEdge > sceneRect.width() - minDistanceFromSceneRightEdge )
        {
            newPos.setX( sceneRect.width() - minDistanceFromSceneRightEdge - rect().width() );
        }
        newPos.setY( Ruler::HEIGHT );

        return newPos;
    }

    // If this waveform item is selected then bring it to the front, else send it to the back
    if ( change == ItemSelectedHasChanged )
    {
        if ( isSelected() )
            setZValue( ZValues::SELECTED_WAVEFORM );
        else
            setZValue( ZValues::WAVEFORM );
    }

    return QGraphicsItem::itemChange( change, value );
}



void WaveformItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    // Always unset the Ctrl-key modifier to prevent non-contiguous waveform items from being selected

    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~Qt::ControlModifier;
    event->setModifiers( modifiers );

    // If the Graphics View has set drag mode to RubberBandDrag then it will additionally have unset
    // this item's ItemIsMovable flag; the event must then be ignored for RubberBandDrag to work.

    if ( flags() & ItemIsSelectable )
    {
        if ( flags() & ItemIsMovable )
        {
            if ( event->button() == Qt::RightButton )
            {
                event->ignore();
            }
            else
            {
                QGraphicsItem::mousePressEvent( event );
                m_orderPosBeforeMove = m_currentOrderPos;
            }
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        emit clicked( this, event->scenePos() );
        event->ignore();
    }
}



void WaveformItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseMoveEvent( event );

    WaveGraphicsScene* scene = static_cast<WaveGraphicsScene*>( this->scene() );
    QList<WaveformItem*> selectedItems = scene->getSelectedWaveforms();

    // If this item is being dragged to the left...
    if ( event->screenPos().x() < event->lastScreenPos().x() )
    {
        // Get the order position and scene position of the leftmost selected item
        const QPointF leftmostSelectedItemScenePos = selectedItems.first()->scenePos();
        const int leftmostSelectedItemOrderPos = selectedItems.first()->getOrderPos();

        // Get item under left edge of the leftmost selected item
        QGraphicsItem* const otherItem = scene->items( leftmostSelectedItemScenePos ).last();

        // If the other item is a WaveformItem...
        if ( otherItem != this && otherItem->type() == WaveformItem::Type )
        {
            WaveformItem* const otherWaveformItem = qgraphicsitem_cast<WaveformItem*>( otherItem );
            const int otherItemOrderPos = otherWaveformItem->getOrderPos();

            if ( otherItemOrderPos < leftmostSelectedItemOrderPos )
            {
                // If the left edge of the leftmost selected item is more than halfway across the other item
                // then move the other item out of the way
                if ( leftmostSelectedItemScenePos.x() < otherWaveformItem->scenePos().x() + otherWaveformItem->rect().center().x() )
                {
                    const int numPlacesMoved = otherItemOrderPos - leftmostSelectedItemOrderPos;
                    QList<int> selectedItemsOrderPositions;

                    foreach ( WaveformItem* item, selectedItems )
                    {
                        selectedItemsOrderPositions << item->getOrderPos();
                    }

                    emit orderPosIsChanging( selectedItemsOrderPositions, numPlacesMoved );
                }
            }
        }
    }

    // If this item is being dragged to the right...
    if ( event->screenPos().x() > event->lastScreenPos().x() )
    {
        // Get the order position and scene position of the rightmost selected item
        const int rightmostSelectedItemOrderPos = selectedItems.last()->getOrderPos();
        const qreal rightmostSelectedItemRightEdge = selectedItems.last()->scenePos().x() +
                                                     selectedItems.last()->rect().width() - 1;

        // Get item under right edge of the rightmost selected item
        QGraphicsItem* const otherItem = scene->items( QPointF(rightmostSelectedItemRightEdge, Ruler::HEIGHT) ).last();

        // If the other item is a WaveformItem...
        if ( otherItem != this && otherItem->type() == WaveformItem::Type )
        {
            WaveformItem* const otherWaveformItem = qgraphicsitem_cast<WaveformItem*>( otherItem );
            const int otherItemOrderPos = otherWaveformItem->getOrderPos();

            if ( otherItemOrderPos > rightmostSelectedItemOrderPos )
            {
                // If the right edge of the rightmost selected item is more than halfway across the other item
                // then move the other item out of the way
                if ( rightmostSelectedItemRightEdge > otherWaveformItem->scenePos().x() + otherWaveformItem->rect().center().x() )
                {
                    const int numPlacesMoved = otherItemOrderPos - rightmostSelectedItemOrderPos;
                    QList<int> orderPositions;

                    foreach ( WaveformItem* item, selectedItems )
                    {
                        orderPositions << item->getOrderPos();
                    }

                    emit orderPosIsChanging( orderPositions, numPlacesMoved );
                }
            }
        }
    }
}



void WaveformItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseReleaseEvent( event );

    WaveGraphicsScene* scene = static_cast<WaveGraphicsScene*>( this->scene() );
    QList<WaveformItem*> selectedItems = scene->getSelectedWaveforms();

    if ( m_orderPosBeforeMove != m_currentOrderPos )
    {
        const int numPlacesMoved = m_currentOrderPos - m_orderPosBeforeMove;
        QList<int> oldOrderPositions;

        foreach ( WaveformItem* item, selectedItems )
        {
            oldOrderPositions << item->getOrderPos() - numPlacesMoved;
        }

        emit orderPosHasChanged( oldOrderPositions, numPlacesMoved );
    }

    foreach ( WaveformItem* item, selectedItems )
    {
        emit finishedMoving( item->getOrderPos() );
    }
}



//==================================================================================================
// Private:

void WaveformItem::setBackgroundGradient()
{
    QLinearGradient gradient( 0.0, 0.0, rect().width(), 0.0 );

    gradient.setColorAt( 0,     QColor::fromRgbF(1.0,   1.0,   1.0,   1.0) );
    gradient.setColorAt( 0.125, QColor::fromRgbF(0.925, 0.925, 0.975, 1.0) );
    gradient.setColorAt( 0.875, QColor::fromRgbF(0.925, 0.925, 0.975, 1.0) );
    gradient.setColorAt( 1,     QColor::fromRgbF(0.8,   0.8,   0.9,   1.0) );

    setBrush( QBrush( gradient ) );
}



void WaveformItem::resetSampleBins()
{
    const int numChans = m_sampleBuffer->getNumChannels();

    m_firstCalculatedBin = NOT_SET;
    m_lastCalculatedBin = NOT_SET;

    m_numBins = rect().width() * m_globalScaleFactor;
    m_binSize = (qreal) m_sampleBuffer->getNumFrames() / ( rect().width() * m_globalScaleFactor );

    if ( m_binSize <= DETAIL_LEVEL_VERY_HIGH_CUTOFF )
    {
        m_detailLevel = VERY_HIGH;

        if ( m_binSize <= DETAIL_LEVEL_MAX_CUTOFF )
        {
            emit maxDetailLevelReached();
        }
    }
    else if ( m_binSize <= DETAIL_LEVEL_HIGH_CUTOFF )
    {
        m_detailLevel = HIGH;

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            m_minSampleValues[ chanNum ]->resize( m_numBins );
            m_maxSampleValues[ chanNum ]->resize( m_numBins );
        }
    }
    else
    {
        m_detailLevel = LOW;

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            m_minSampleValues[ chanNum ]->resize( m_numBins );
            m_maxSampleValues[ chanNum ]->resize( m_numBins );
        }
    }
}



void WaveformItem::findMinMaxSamples( const int startBin, const int endBin )
{
    const int numChans = m_sampleBuffer->getNumChannels();

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        for ( int binNum = startBin; binNum <= endBin; binNum++ )
        {
            Range<float> range = m_sampleBuffer->findMinMax( chanNum,
                                                            int( binNum * m_binSize ),
                                                            int( m_binSize ) );

            m_minSampleValues[ chanNum ]->set( binNum, range.getStart() );
            m_maxSampleValues[ chanNum ]->set( binNum, range.getEnd() );
        }
    }
}
