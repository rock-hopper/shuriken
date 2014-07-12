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

#include "waveformitem.h"
#include <QDebug>


//==================================================================================================
// Public:

WaveformItem::WaveformItem( const SharedSampleBuffer sampleBuffer,
                            const SharedSampleRange sampleRange,
                            const qreal width, const qreal height,
                            QGraphicsItem* parent ) :
    QObject(),
    QGraphicsRectItem( 0.0, 0.0, width, height, parent ),
    mSampleBuffer( sampleBuffer ),
    mSampleRange( sampleRange ),
    mCurrentOrderPos( 0 ),
    mScaleFactor( NOT_SET ),
    mFirstCalculatedBin( NOT_SET ),
    mLastCalculatedBin( NOT_SET )
{
    init();
}



WaveformItem::WaveformItem( const SharedSampleBuffer sampleBuffer,
                            const SharedSampleRange sampleRange,
                            const int orderPos,
                            const qreal width, const qreal height,
                            QGraphicsItem* parent ) :
    QObject(),
    QGraphicsRectItem( 0.0, 0.0, width, height, parent ),
    mSampleBuffer( sampleBuffer ),
    mSampleRange( sampleRange ),
    mCurrentOrderPos( orderPos ),
    mScaleFactor( NOT_SET ),
    mFirstCalculatedBin( NOT_SET ),
    mLastCalculatedBin( NOT_SET )
{
    init();
}



WaveformItem::WaveformItem( const QList<SharedWaveformItem> items, QGraphicsItem* parent ) :
    QObject(),
    QGraphicsRectItem( 0.0, 0.0, 1.0, 1.0, parent ),
    mSampleBuffer( items.first()->getSampleBuffer() ),
    mSampleRange( new SampleRange ),
    mCurrentOrderPos( items.first()->getOrderPos() ),
    mScaleFactor( NOT_SET ),
    mFirstCalculatedBin( NOT_SET ),
    mLastCalculatedBin( NOT_SET )
{
    mSampleRange->startFrame = items.first()->getSampleRange()->startFrame;

    qreal width = 0.0;
    int numFrames = 0;

    foreach ( SharedWaveformItem item, items )
    {
        width += item->rect().width();
        numFrames += item->getSampleRange()->numFrames;

        mJoinedItems << item;
    }

    mSampleRange->numFrames = numFrames;

    QGraphicsRectItem::setRect( 0.0, 0.0, width, items.first()->rect().height() );
    setPos( items.first()->scenePos() );

    init();
}



void WaveformItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED( widget );

    const int numChans = mSampleBuffer->getNumChannels();

    int firstVisibleBin;
    int lastVisibleBin;
    int numVisibleBins;

    qreal distanceBetweenFrames;
    int firstVisibleFrame;
    int lastVisibleFrame;
    int numVisibleFrames;

    // If scale has changed since the last redraw then reset sample bins and establish new detail level
    if ( mScaleFactor != painter->worldTransform().m11() )
    {
        mScaleFactor = painter->worldTransform().m11(); // m11() returns the current horizontal scale factor
        resetSampleBins();
    }

    if ( mDetailLevel != VERY_HIGH )
    {
        // Reduce no. of samples to draw by finding the min/max values in each consecutive sample "bin"
        firstVisibleBin = (int) floor( option->exposedRect.left() * mScaleFactor );
        lastVisibleBin = qMin( (int) ceil( option->exposedRect.right() * mScaleFactor ), mNumBins - 1 );

        if ( mFirstCalculatedBin == NOT_SET || mLastCalculatedBin == NOT_SET )
        {
            findMinMaxSamples( firstVisibleBin, lastVisibleBin );
            mFirstCalculatedBin = firstVisibleBin;
            mLastCalculatedBin = lastVisibleBin;
        }

        if ( firstVisibleBin < mFirstCalculatedBin )
        {
            findMinMaxSamples( firstVisibleBin, mFirstCalculatedBin - 1 );
            mFirstCalculatedBin = firstVisibleBin;
        }

        if ( lastVisibleBin > mLastCalculatedBin )
        {
            findMinMaxSamples( mLastCalculatedBin + 1, lastVisibleBin );
            mLastCalculatedBin = lastVisibleBin;
        }

        numVisibleBins = lastVisibleBin - firstVisibleBin + 1;
    }
    else // mDetailLevel == VERY_HIGH
    {
        distanceBetweenFrames = rect().width() / mSampleRange->numFrames;

        firstVisibleFrame = mSampleRange->startFrame + (int) floor( option->exposedRect.left() / distanceBetweenFrames );

        lastVisibleFrame = mSampleRange->startFrame + qMin( (int) ceil( option->exposedRect.right() / distanceBetweenFrames ),
                                               mSampleRange->numFrames - 1 );

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
    painter->setPen( mCentreLinePen );
    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        painter->drawLine( 0.0, 0.0, rect().width(), 0.0 );
        painter->translate( 0.0, 1.0 * numChans );
    }
    painter->restore();

    // Draw waveform
    painter->translate( 0.0, 1.0 );
    painter->setPen( mWavePen );

    const qreal lineWidth = 1.0 / mScaleFactor;
    const qreal leftEdge = option->exposedRect.left();

    float min;
    float max;

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        if ( mDetailLevel == LOW )
        {
            painter->setRenderHint( QPainter::Antialiasing, false );

            for ( int binCount = 0; binCount < numVisibleBins; binCount++ )
            {
                min = (*mMinSampleValues[ chanNum ])[ firstVisibleBin + binCount ];
                max = (*mMaxSampleValues[ chanNum ])[ firstVisibleBin + binCount ];

                painter->drawLine
                (
                    QPointF( leftEdge + (binCount * lineWidth), -min ),
                    QPointF( leftEdge + (binCount * lineWidth), -max )
                );
            }
        }
        else if ( mDetailLevel == HIGH )
        {
            painter->setRenderHint( QPainter::Antialiasing, true );

            QPointF points[ numVisibleBins * 2 ];

            for ( int binCount = 0; binCount < numVisibleBins; binCount++ )
            {
                min = (*mMinSampleValues[ chanNum ])[ firstVisibleBin + binCount ];
                max = (*mMaxSampleValues[ chanNum ])[ firstVisibleBin + binCount ];

                points[ binCount * 2 ]       = QPointF( leftEdge + (binCount * lineWidth), -min );
                points[ (binCount * 2) + 1 ] = QPointF( leftEdge + (binCount * lineWidth), -max );
            }
            painter->drawPolyline( points, numVisibleBins * 2 );
        }
        else // mDetailLevel == VERY_HIGH
        {
            painter->setRenderHint( QPainter::Antialiasing, true );

            QPointF points[ numVisibleFrames ];
            float* sampleData = mSampleBuffer->getSampleData( chanNum, firstVisibleFrame );

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

bool WaveformItem::isLessThanStartFrame( const SharedWaveformItem item1, const SharedWaveformItem item2 )
{
    return item1->getSampleRange()->startFrame < item2->getSampleRange()->startFrame;
}



bool WaveformItem::isLessThanOrderPos( const WaveformItem* const item1, const WaveformItem* const item2 )
{
    return item1->getOrderPos() < item2->getOrderPos();
}



QList<WaveformItem*> WaveformItem::getSortedListSelectedItems( const QGraphicsScene* const scene )
{
    QList<WaveformItem*> selectedItems;

    if ( scene != NULL )
    {
        foreach ( QGraphicsItem* item, scene->selectedItems() )
        {
            if ( item->type() == WaveformItem::Type )
            {
                selectedItems << qgraphicsitem_cast<WaveformItem*>( item );
            }
        }
        qSort( selectedItems.begin(), selectedItems.end(), WaveformItem::isLessThanOrderPos );
    }

    return selectedItems;
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
            QList<WaveformItem*> selectedItems = getSortedListSelectedItems( scene() );

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
        newPos.setY( 0.0 );

        return newPos;
    }

    // If this waveform item is selected then bring it to the front, else send it to the back
    if ( change == ItemSelectedHasChanged )
    {
        if ( isSelected() )
            setZValue( 1 );
        else
            setZValue( 0 );
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
                mOrderPosBeforeMove = mCurrentOrderPos;
            }
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        emit playSampleRange( mSampleRange->startFrame, mSampleRange->numFrames, event->scenePos() );
        event->ignore();
    }
}



void WaveformItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseMoveEvent( event );

    QList<WaveformItem*> selectedItems = getSortedListSelectedItems( scene() );

    // If this item is being dragged to the left...
    if ( event->screenPos().x() < event->lastScreenPos().x() )
    {
        // Get the order position and scene position of the leftmost selected item
        const QPointF leftmostSelectedItemScenePos = selectedItems.first()->scenePos();
        const int leftmostSelectedItemOrderPos = selectedItems.first()->getOrderPos();

        // Get item under left edge of the leftmost selected item
        QGraphicsItem* const otherItem = scene()->items( leftmostSelectedItemScenePos ).last();

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
        QGraphicsItem* const otherItem = scene()->items( QPointF(rightmostSelectedItemRightEdge, 0.0) ).last();

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

    QList<WaveformItem*> selectedItems = getSortedListSelectedItems( scene() );

    if ( mOrderPosBeforeMove != mCurrentOrderPos )
    {
        const int numPlacesMoved = mCurrentOrderPos - mOrderPosBeforeMove;
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

void WaveformItem::init()
{
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges | ItemUsesExtendedStyleOption );

    setBackgroundGradient();
    mWavePen  = QPen( QColor(15, 15, 159, 191) );
    mCentreLinePen = QPen( QColor(127, 127, 127, 191) );

    // Don't draw rect border
    setPen( QPen( QColor(0, 0, 0, 0) ) );

    // Set up min/max sample "bins"
    const int numChans = mSampleBuffer->getNumChannels();
    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        mMinSampleValues.add( new Array<float> );
        mMaxSampleValues.add( new Array<float> );
    }
}



void WaveformItem::setBackgroundGradient()
{
    QLinearGradient gradient( 0.0, 0.0, rect().width(), 0.0 );

    if ( isJoined() )
    {
        gradient.setColorAt( 0,     QColor::fromRgbF(1.0,   1.0,   1.0,   1.0) );
        gradient.setColorAt( 0.125, QColor::fromRgbF(0.975, 0.975, 0.775, 1.0) );
        gradient.setColorAt( 0.875, QColor::fromRgbF(0.975, 0.975, 0.775, 1.0) );
        gradient.setColorAt( 1,     QColor::fromRgbF(0.9,   0.9,   0.6,   1.0) );
    }
    else
    {
        gradient.setColorAt( 0,     QColor::fromRgbF(1.0,   1.0,   1.0,   1.0) );
        gradient.setColorAt( 0.125, QColor::fromRgbF(0.925, 0.925, 0.975, 1.0) );
        gradient.setColorAt( 0.875, QColor::fromRgbF(0.925, 0.925, 0.975, 1.0) );
        gradient.setColorAt( 1,     QColor::fromRgbF(0.8,   0.8,   0.9,   1.0) );
    }

    setBrush( QBrush( gradient ) );
}



void WaveformItem::resetSampleBins()
{
    const int numChans = mSampleBuffer->getNumChannels();

    mFirstCalculatedBin = NOT_SET;
    mLastCalculatedBin = NOT_SET;

    mNumBins = rect().width() * mScaleFactor;
    mBinSize = (qreal) mSampleRange->numFrames / ( rect().width() * mScaleFactor );

    if ( mBinSize <= DETAIL_LEVEL_VERY_HIGH_CUTOFF )
    {
        mDetailLevel = VERY_HIGH;

        if ( mBinSize <= DETAIL_LEVEL_MAX_CUTOFF )
        {
            emit maxDetailLevelReached();
        }
    }
    else if ( mBinSize <= DETAIL_LEVEL_HIGH_CUTOFF )
    {
        mDetailLevel = HIGH;

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            mMinSampleValues[ chanNum ]->resize( mNumBins );
            mMaxSampleValues[ chanNum ]->resize( mNumBins );
        }
    }
    else
    {
        mDetailLevel = LOW;

        for ( int chanNum = 0; chanNum < numChans; chanNum++ )
        {
            mMinSampleValues[ chanNum ]->resize( mNumBins );
            mMaxSampleValues[ chanNum ]->resize( mNumBins );
        }
    }
}



void WaveformItem::findMinMaxSamples( const int startBin, const int endBin )
{
    const int numChans = mSampleBuffer->getNumChannels();
    float min;
    float max;

    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
    {
        for ( int binNum = startBin; binNum <= endBin; binNum++ )
        {
            mSampleBuffer->findMinMax( chanNum,
                                       mSampleRange->startFrame + int( binNum * mBinSize ),
                                       (int) mBinSize,
                                       min,
                                       max);

            mMinSampleValues[ chanNum ]->set( binNum, min );
            mMaxSampleValues[ chanNum ]->set( binNum, max );
        }
    }
}
