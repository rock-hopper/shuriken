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
                            const qreal width, const qreal height,
                            QGraphicsItem* parent ) :
    QObject(),
    QGraphicsRectItem( 0.0, 0.0, width, height, parent ),
    mSampleBuffer( sampleBuffer ),
    mStartFrame( 0 ),
    mNumFrames( sampleBuffer->getNumFrames() ),
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
    mStartFrame( sampleRange->startFrame ),
    mNumFrames( sampleRange->numFrames ),
    mCurrentOrderPos( orderPos ),
    mScaleFactor( NOT_SET ),
    mFirstCalculatedBin( NOT_SET ),
    mLastCalculatedBin( NOT_SET )
{
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
        distanceBetweenFrames = rect().width() / mNumFrames;

        firstVisibleFrame = mStartFrame + (int) floor( option->exposedRect.left() / distanceBetweenFrames );

        lastVisibleFrame = mStartFrame + qMin( (int) ceil( option->exposedRect.right() / distanceBetweenFrames ),
                                               mNumFrames - 1 );

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

bool WaveformItem::isLessThan( const SharedWaveformItem item1, const SharedWaveformItem item2 )
{
    return item1->getStartFrame() < item2->getStartFrame();
}



//==================================================================================================
// Protected:

QVariant WaveformItem::itemChange( GraphicsItemChange change, const QVariant &value )
{
    // Keep waveform item within bounds of scene rect
    if ( change == ItemPositionChange && scene() != NULL )
    {
        QPointF newPos = value.toPointF();
        const QPointF newPosBottomRight = QPointF( newPos.x() + rect().width(), newPos.y() + rect().height() );
        const QRectF sceneRect = scene()->sceneRect();

        if ( ! ( sceneRect.contains( newPos ) && sceneRect.contains( newPosBottomRight ) ) )
        {
            newPos.setX
            (
                    qMin( sceneRect.right() - rect().width(), qMax( newPos.x(), sceneRect.left() ) )
            );
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
    if ( event->button() == Qt::RightButton )
    {
        emit rightMousePressed( mStartFrame, mNumFrames, event->scenePos() );
        event->ignore();
    }
    else
    {
        QGraphicsItem::mousePressEvent( event );
        mOrderPosBeforeMove = mCurrentOrderPos;
    }
}



void WaveformItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseMoveEvent( event );

    // If this item is being dragged to the left
    if ( event->screenPos().x() < event->lastScreenPos().x() )
    {
        // Get item under left edge of this item
        QGraphicsItem* const item = scene()->items( QPointF(scenePos()) ).last();

        if ( item != this && item->type() == type() ) // If the other item is a WaveformItem
        {
            WaveformItem* const otherWaveformItem = qgraphicsitem_cast<WaveformItem*>( item );

            if ( otherWaveformItem->getOrderPos() < mCurrentOrderPos )
            {
                // If the left edge of this item is more than halfway across the other item then swap places
                if ( scenePos().x() < otherWaveformItem->scenePos().x() + otherWaveformItem->rect().center().x() )
                {
                    emit orderPosIsChanging( mCurrentOrderPos, otherWaveformItem->getOrderPos() );
                }
            }
        }
    }

    // If this item is being dragged to the right
    if ( event->screenPos().x() > event->lastScreenPos().x() )
    {
        // Get item under right edge of this slice
        const qreal rightX = scenePos().x() + rect().width() - 1;
        if ( rightX >= 1.0 )
        {
            QGraphicsItem* const item = scene()->items( QPointF(rightX, 0.0) ).last();

            if ( item != this && item->type() == type() ) // If the other item is a WaveformItem
            {
                WaveformItem* const otherWaveformItem = qgraphicsitem_cast<WaveformItem*>( item );

                if ( otherWaveformItem->getOrderPos() > mCurrentOrderPos )
                {
                    // If the right edge of this item is more than halfway across the other item then swap places
                    if ( rightX > otherWaveformItem->scenePos().x() + otherWaveformItem->rect().center().x() )
                    {
                        emit orderPosIsChanging( mCurrentOrderPos, otherWaveformItem->getOrderPos() );
                    }
                }
            }
        }
    }
}



void WaveformItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    QGraphicsItem::mouseReleaseEvent( event );

    if ( mOrderPosBeforeMove != mCurrentOrderPos )
    {
        emit orderPosHasChanged( mOrderPosBeforeMove, mCurrentOrderPos );
    }

    emit finishedMoving( mCurrentOrderPos );
}



//==================================================================================================
// Private:

void WaveformItem::init()
{
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges | ItemUsesExtendedStyleOption );

    setBackgroundGradient();
    mWavePen = QPen( QColor(0, 0, 127, 191) );
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

    gradient.setColorAt( 0,     QColor::fromRgbF(1.0,   1.0,   1.0,   1.0) );
    gradient.setColorAt( 0.125, QColor::fromRgbF(0.925, 0.925, 0.975, 1.0) );
    gradient.setColorAt( 0.875, QColor::fromRgbF(0.925, 0.925, 0.975, 1.0) );
    gradient.setColorAt( 1,     QColor::fromRgbF(0.8,   0.8,   0.9,   1.0) );

    setBrush( QBrush( gradient ) );
}



void WaveformItem::resetSampleBins()
{
    const int numChans = mSampleBuffer->getNumChannels();

    mFirstCalculatedBin = NOT_SET;
    mLastCalculatedBin = NOT_SET;

    mNumBins = rect().width() * mScaleFactor;
    mBinSize = (qreal) mNumFrames / ( rect().width() * mScaleFactor );

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



// Both `startBin` and `endBin` are inclusive
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
                                       mStartFrame + int( binNum * mBinSize ),
                                       (int) mBinSize,
                                       min,
                                       max);

            mMinSampleValues[ chanNum ]->set( binNum, min );
            mMaxSampleValues[ chanNum ]->set( binNum, max );
        }
    }
}
